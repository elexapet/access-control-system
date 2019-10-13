import sys
import os
import socket
import struct
import errno
import select
import logging
import ctypes
import mmap

class can_filter(ctypes.Structure):
    """
    Native structure of a CAN filter for SocketCAN
    """
    _pack_ = 1
    _fields_ = [
                ('id', ctypes.c_uint32),
                ('mask', ctypes.c_uint32),
                ]


class can_raw_sock(object):
    """
    Thin layer over CAN socket.
    """
    __CAN_MTU = 16
    __FORMAT = "<IB3x8s"
    CAN_ERR_TX_TIMEOUT = 0x00000001
    CAN_ERR_BUSOFF = 0x00000040
    CAN_ERR_RESTARTED = 0x00000100

    def __init__(self):
        try:
            # Default behavior:
            # - The filters are set to exactly one filter receiving everything
            # - The socket only receives valid data frames (=> no error message frames)
            # - The loopback of sent CAN frames is enabled
            # - The socket does not receive its own sent frames (in loopback mode)
            # - No CAN FD
            self.__cansock = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)

            # Receive following error frames
            err_mask = self.CAN_ERR_TX_TIMEOUT | self.CAN_ERR_BUSOFF | self.CAN_ERR_RESTARTED
            self.__cansock.setsockopt(socket.SOL_CAN_RAW, socket.CAN_RAW_ERR_FILTER, err_mask)

        except OSError as e:
            logging.critical("{}\n".format(os.strerror(e.errno)))
            sys.exit(e.errno)

    # matches when <recieved_id> & mask == id & mask
    def set_recv_filter(self, can_filters):
        # convert list of structures to c-style array of structures
        FilterArray = can_filter * len(can_filters)
        mm_file = mmap.mmap(-1, ctypes.sizeof(FilterArray), access=mmap.ACCESS_WRITE)
        rfilters = FilterArray.from_buffer(mm_file)
        for i in range(len(rfilters)):
            rfilters[i] = can_filters[i]
        # all filters must be set by one call
        self.__cansock.setsockopt(socket.SOL_CAN_RAW, socket.CAN_RAW_FILTER, rfilters)

    def bind(self, on_interface:str):
        try:
            self.__cansock.bind((on_interface,))
            self.__interface = on_interface
        except OSError as e:
            logging.critical("{}\n".format(os.strerror(e.errno)))
            sys.exit(e.errno)

    # raises OSError
    def send(self, can_id:int, dlc, data, flags=0):
        can_id = can_id | flags
        data = data.ljust(8, b'\x00')

        can_pkt = struct.pack(self.__FORMAT, can_id, dlc, data)
        return self.__cansock.send(can_pkt)

    # raises OSError
    def recv(self):
        can_pkt = self.__cansock.recv(self.__CAN_MTU)

        if len(can_pkt) == self.__CAN_MTU:
            can_id, length, data = struct.unpack(self.__FORMAT, can_pkt)
        else:
            logging.warning("Incomplete CAN frame from '{}' interface \n".format(self.__interface))
            return (0, 0, [0])

        # remote frames are not supported
        if can_id & socket.CAN_RTR_FLAG:
            return (0, 0, [0])

        # process error frames
        if can_id & socket.CAN_ERR_FLAG:
            self._got_error_frame(can_id)
            return (0, 0, [0])

        return (can_id, length, data[:length])

    def try_select_recv_for(self, timeout_secs):
        rtr, rtw, ie = select.select([self.__cansock], [], [], timeout_secs)
        return True if len(rtr) > 0 else False

    def close(self):
        self.__cansock.close()
        self.__cansock = None

    def _got_error_frame(self, can_id):
        if can_id & self.CAN_ERR_TX_TIMEOUT:
            logging.warning("CAN TX timeout (by netdevice driver)")
        elif can_id & self.CAN_ERR_BUSOFF:
            logging.error("CAN bus off")
        elif can_id & self.CAN_ERR_RESTARTED:
            logging.warning("CAN controller restarted")


class acs_can_proto(object):
    """
    Implementation of ACS protocol for CAN (Uses extended arbitration ID).
    """

    # ACS protocol function codes
    FC_RESERVED = 0
    # s->m
    FC_USER_AUTH_REQ = 1
    # m->s
    FC_USER_AUTH_RESP_FAIL = 2
    # m->s
    FC_USER_AUTH_RESP_OK = 3
    # m->s
    FC_DOOR_CTRL = 4
    # s->m
    FC_DOOR_STATUS = 5
    # m->s
    FC_ALIVE = 6

    # priorities
    PRIO_RESERVED = 0
    PRIO_USER_AUTH_REQ = 2
    PRIO_USER_AUTH_RESP_FAIL = 2
    PRIO_USER_AUTH_RESP_OK = 2
    PRIO_DOOR_CTRL = 3
    PRIO_DOOR_STATUS = 4
    PRIO_ALIVE = 1

    MASTER_ALIVE_PERIOD = 5  # seconds
    MASTER_ALIVE_TIMEOUT = 12

    # Data for FC_DOOR_CTRL
    DATA_DOOR_CTRL_REMOTE_UNLCK = b'\x01'
    DATA_DOOR_CTRL_CLR_CACHE = b'\x02'

    # Data for FC_DOOR_STATUS
    DATA_DOOR_STATUS_CLOSED = b'\x01'
    DATA_DOOR_STATUS_OPEN = b'\x02'

    # Sizes of partitions in message header (CAN_ID)
    ACS_PRIO_BITS = 3
    ACS_FC_BITS = 6
    ACS_ADDR_BITS = 10

    # Address space
    ACS_BROADCAST_ADDR = (1 << ACS_ADDR_BITS) - 1
    ACS_RESERVED_ADDR = 0
    ACS_MSTR_FIRST_ADDR = 1
    ACS_MSTR_LAST_ADDR = 3
    ACS_PNL_FIRST_ADDR = 4
    ACS_PNL_LAST_ADDR = ACS_BROADCAST_ADDR - 1

    # msg head offsets
    ACS_SRC_ADDR_OFFSET = 0
    ACS_DST_ADDR_OFFSET = ACS_SRC_ADDR_OFFSET + ACS_ADDR_BITS
    ACS_FC_OFFSET = ACS_DST_ADDR_OFFSET + ACS_ADDR_BITS
    ACS_PRIO_OFFSET = ACS_FC_OFFSET + ACS_FC_BITS

    # msg head masks
    ACS_SRC_ADDR_MASK = ((1 << ACS_ADDR_BITS) - 1) << ACS_SRC_ADDR_OFFSET
    ACS_DST_ADDR_MASK = ((1 << ACS_ADDR_BITS) - 1) << ACS_DST_ADDR_OFFSET
    ACS_FC_MASK = ((1 << ACS_FC_BITS) - 1) << ACS_FC_OFFSET
    ACS_PRIO_MASK = ((1 << ACS_PRIO_BITS) - 1) << ACS_PRIO_OFFSET

    ACC_MAX_PRIO = 0
    ACS_LOW_PRIO = (1 << ACS_PRIO_BITS) - 1

    NO_MESSAGE = (0, 0, [0])

    CAN_ID_MASK = 0xFFFFFFFF

    def __init__(self, master_addr:int, cb_user_auth_req, cb_door_status_update):
        # create socket
        self.can_sock = can_raw_sock()

        # register message callbacks
        self.cb_user_auth_req = cb_user_auth_req
        self.cb_door_status_update = cb_door_status_update

        if self.ACS_MSTR_LAST_ADDR >= master_addr >= self.ACS_MSTR_FIRST_ADDR:
            self.addr = master_addr
        else:
            raise Exception("Invalid master address '{}'\n".format(master_addr))

        # get only messages for us or broadcast
        f_us = can_filter(socket.CAN_EFF_FLAG | (self.addr << self.ACS_DST_ADDR_OFFSET),
                          socket.CAN_EFF_FLAG | self.ACS_DST_ADDR_MASK)
        f_bcast = can_filter(socket.CAN_EFF_FLAG | (self.ACS_BROADCAST_ADDR << self.ACS_DST_ADDR_OFFSET),
                             socket.CAN_EFF_FLAG | self.ACS_DST_ADDR_MASK)
        self.can_sock.set_recv_filter([f_us, f_bcast])

    # Bind to a CAN interface.
    def bind(self, can_if):
        self.can_sock.bind(can_if)

    # Create CAN arbitration ID.
    def __msg(self, prio, fc, dst):
        # improvement: replace with struct pack
        can_id = (prio << self.ACS_PRIO_OFFSET) & self.ACS_PRIO_MASK
        can_id |= (fc << self.ACS_FC_OFFSET) & self.ACS_FC_MASK
        can_id |= (dst << self.ACS_DST_ADDR_OFFSET) & self.ACS_DST_ADDR_MASK
        can_id |= (self.addr << self.ACS_SRC_ADDR_OFFSET) & self.ACS_SRC_ADDR_MASK
        can_id |= socket.CAN_EFF_FLAG
        can_id &= self.CAN_ID_MASK
        return can_id

    def msg_auth_fail(self, reader_addr, user_id:int):
        return (self.__msg(self.PRIO_USER_AUTH_RESP_FAIL, self.FC_USER_AUTH_RESP_FAIL, reader_addr),
                4, user_id.to_bytes(4, "little", signed=True))

    def msg_auth_ok(self, reader_addr, user_id:int):
        return (self.__msg(self.PRIO_USER_AUTH_RESP_OK, self.FC_USER_AUTH_RESP_OK, reader_addr),
                4, user_id.to_bytes(4, "little", signed=True))

    def msg_reader_unlock_once(self, reader_addr):
        return (self.__msg(self.PRIO_DOOR_CTRL, self.FC_DOOR_CTRL, reader_addr),
                1, self.DATA_DOOR_CTRL_REMOTE_UNLCK)

    def msg_reader_clear_cache(self, reader_addr):
        return (self.__msg(self.PRIO_DOOR_CTRL, self.FC_DOOR_CTRL, reader_addr),
                1, self.DATA_DOOR_CTRL_CLR_CACHE)

    def msg_master_alive(self):
        return (self.__msg(self.PRIO_ALIVE, self.FC_ALIVE, self.ACS_BROADCAST_ADDR),
                0, b'\x00')

    # Parse arbitration ID
    def __parse_msg_head(self, msg_head):
        prio = (msg_head & self.ACS_PRIO_MASK) >> self.ACS_PRIO_OFFSET
        fc = (msg_head & self.ACS_FC_MASK) >> self.ACS_FC_OFFSET
        dst = (msg_head & self.ACS_DST_ADDR_MASK) >> self.ACS_DST_ADDR_OFFSET
        src = (msg_head & self.ACS_SRC_ADDR_MASK) >> self.ACS_SRC_ADDR_OFFSET
        return (prio, fc, dst, src)

    # Process received CAN message
    def process_msg(self, msg_head:int, msg_len:int, msg_data):
        prio, fc, dst, src = self.__parse_msg_head(msg_head)

        if (msg_head & socket.CAN_EFF_FLAG and
            src <= self.ACS_PNL_LAST_ADDR and
            src >= self.ACS_PNL_FIRST_ADDR and
            prio > self.PRIO_RESERVED):
            if fc == self.FC_USER_AUTH_REQ:
                if self.cb_user_auth_req is not None:
                    user_id = int.from_bytes(msg_data[:4], "little", signed=True)
                    if self.cb_user_auth_req(src, user_id):
                        return self.msg_auth_ok(src, user_id)
                    else:
                        return self.msg_auth_fail(src, user_id)
            elif fc == self.FC_DOOR_STATUS:
                if self.cb_door_status_update is not None:
                    self.cb_door_status_update(src, msg_data[:1] == self.DATA_DOOR_STATUS_OPEN)
                    return self.NO_MESSAGE
            else:
                return self.NO_MESSAGE

        return self.NO_MESSAGE
