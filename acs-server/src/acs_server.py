import errno
import signal
import sys
import os
import logging
import time
# For remote debugging add firewall exception e.g. iptables -A INPUT -p tcp -m state --state NEW -m tcp --dport 5678 -j ACCEPT
# import ptvsd

from helpers import parse_args, format_data
from acs_database import acs_database
from acs_can_proto import acs_can_proto, can_raw_sock


class acs_server(object):
    """
    This is access control server (ACS) main class.

    The server acts as a master to RFID readers connected by CAN bus.
    Interfaces to database of users trough "acs_database" and uses protocol implemented by "acs_can_proto".
    """

    __running = True

    def __init__(self, can_if, addr, r_host, r_port, debug):
        self.can_if = can_if
        self.addr = addr
        try:
            self.proto = acs_can_proto(addr, cb_user_auth_req=self.resp_to_auth_req,
                                       cb_door_status_update=self.door_status_update)
            self.proto.bind(can_if)
        except Exception as e:
            logging.exception("Unable to start the server: %s", e)
            sys.exit(1)

        try:
            self.db = acs_database(r_host, r_port)
        except Exception as e:
            logging.exception("Unable to connect to Redis server: %s", e)
            sys.exit(1)

        signal.signal(signal.SIGINT, self.sigterm)
        signal.signal(signal.SIGTERM, self.sigterm)
        signal.signal(signal.SIGHUP, self.sigterm)

        self.debug = debug

    # OS signals handler
    def sigterm(self, signum, frame):
        if self.__running:
            self.__running = False
            logging.warning("Shutdown requested...")
        else:
            logging.warning("Forcing shutdown...")
            sys.exit(0)

    # server command to unlock door
    def remote_unlock_door(self, reader_addr):
        if self.debug:
            logging.debug("remote_unlock_door: reader={}".format(reader_addr))
        can_id, dlc, data = self.proto.msg_reader_unlock_once(reader_addr)
        self.proto.can_sock.send(can_id, dlc, data)

    # add a new user to database
    def add_new_user(self, reader_addr, user_id):
        if self.debug:
            logging.debug("add_new_user: reader={}, user={}".format(reader_addr, user_id))
        # do not add existing users
        if self.db.get_user_group(user_id) is not None:
            return
        # create special group
        group = self.db.create_group_for_panel(reader_addr)
        if group is not None:
            if self.db.add_user(user_id, group):
                return
            else:
                self.db.remove_group(group)
                return
        return

    # callback to authorization request
    # return True if authorized to open door False otherwise
    def resp_to_auth_req(self, reader_addr, user_id):
        if self.debug:
            logging.debug("resp_to_auth_req: reader={}, user={}".format(reader_addr, user_id))
        mode = self.db.get_door_mode(reader_addr)
        if mode is None:
            if self.debug:
                logging.debug("resp_to_auth_req: no door mode!")
            mode = self.db.DOOR_MODE_ENABLED  # fallback to default
        if mode == self.db.DOOR_MODE_ENABLED:
            if self.db.is_user_authorized(user_id, reader_addr):
                self.db.log_user_access(user_id, reader_addr)
                return True
            else:
                return False
        elif mode == self.db.DOOR_MODE_LEARN:
            self.add_new_user(reader_addr, user_id)
            return True
        else:
            return False

    # callback for door status update
    def door_status_update(self, reader_addr, is_open):
        if self.debug:
            logging.debug("door_status_update: reader={} open={}".format(reader_addr, is_open))
        self.db.set_door_is_open(reader_addr, is_open)

    # main processing loop
    def run(self):
        logging.info("ACS server has started")
        logging.info("Listening on {} with address {}".format(self.can_if, self.addr))

        last_alive = time.monotonic()

        while self.__running:
            try:
                # alive msg
                this_alive = time.monotonic()
                if (this_alive - last_alive) >= self.proto.MASTER_ALIVE_PERIOD:
                    last_alive = this_alive
                    can_id, dlc, data = self.proto.msg_master_alive()
                    self.proto.can_sock.send(can_id, dlc, data)

                # try recv
                if self.proto.can_sock.try_select_recv_for(5):
                    can_id, dlc, data = self.proto.can_sock.recv()

                    if can_id == 0:
                        continue
                    if self.debug:
                        logging.debug("%s:recv: 0x%03x#0x%s" % (self.can_if, can_id, format_data(data)))

                    # process
                    can_id, dlc, data = self.proto.process_msg(can_id, dlc, data)

                    if can_id != 0:
                        # response
                        self.proto.can_sock.send(can_id, dlc, data)
                        if self.debug:
                            logging.debug("%s:send: 0x%03x#0x%s" % (self.can_if, can_id, format_data(data)))
                    elif self.debug:
                        logging.debug("msg no response")

            except OSError as eos:
                logging.error("{}\n".format(os.strerror(eos.errno)))
            except Exception as e:
                logging.exception("Exception occurred: %s", e)

        self.proto.can_sock.close()

        logging.info("ACS server has shutdown")

def setup_logging(logname, verbose):
    if verbose:
        level = logging.DEBUG
    else:
        level = logging.INFO

    if logname:
        logging.basicConfig(filename=logname, level=level,
                            format='%(asctime)s [%(levelname)s] %(message)s', datefmt='%d/%m/%Y %H:%M:%S')
    else:
        logging.basicConfig(level=level,
                            format='%(asctime)s [%(levelname)s] %(message)s', datefmt='%d/%m/%Y %H:%M:%S')

def main():
    # ptvsd.enable_attach()
    pargs = parse_args()
    logname = pargs.log_dir
    if pargs.log_dir:
        logname = "{}/{}_{}.log".format(pargs.log_dir, pargs.interface, pargs.id)
    setup_logging(logname, pargs.verbose)
    acs_server(pargs.interface, pargs.id, pargs.redis_hostname, pargs.redis_port, pargs.verbose).run()

if __name__ == "__main__":
    main()
