import redis
import logging
from acs_can_proto import acs_can_proto

class acs_database(object):
    """
        ACS server database model based on Redis key-value store.

        The redis store can be used as stand-alone database or only as a cache
        that is updated from some main database.

        Containts users that belong to groups.
        If user is part of a group it means that he is authorized to use doors in the group.

        User ID (key)
            - value is group name (can be part of only one group)
            - the user id must be an unique number (raw number of RFID card or chip)
        Group (key)
            - value is set of of door/reader addresses
            - the group must be an unique name
            - is a string (group name) and must not start with "__"

    """

    __ALL_GRP = b"__all"  # Special group representing all doors.
    __EMPTY_GRP = b"__void"  # Special empty group.
    __NONAME_GRP_PREFIX = "__nng_"
    __RESERVED_ADDR = 0
    __DEFAULT_HOST = "localhost"
    __DEFAULT_PORT = 6379
    __DOOR_MODE_IDX = 0
    __DOOR_STATUS_IDX = 1

    # Doors can be disabled to prevent access. Or switched to learn mode where authorization request
    # is interpreted that door is unlocked and user is added to database and given access to source door.
    DOOR_MODE_DISABLE = 0
    DOOR_MODE_ENABLED = 1
    DOOR_MODE_LEARN = 2

    def __init__(self, host=__DEFAULT_HOST, port=__DEFAULT_PORT):
        # create database connection
        self.__rclient_user = redis.Redis(host, port, db=0, password=None, encoding='utf-8',
            socket_timeout=10, socket_keepalive=True, retry_on_timeout=True)
        self.__rclient_group = redis.Redis(host, port, db=1, password=None, encoding='utf-8',
            socket_timeout=10, socket_keepalive=True, retry_on_timeout=True)
        self.__rclient_door = redis.Redis(host, port, db=2, password=None, encoding='utf-8',
            socket_timeout=10, socket_keepalive=True, retry_on_timeout=True)

        # create basic groups (if not present)
        self.add_panels_to_group(self.__ALL_GRP, self.__RESERVED_ADDR)
        self.add_panels_to_group(self.__EMPTY_GRP, self.__RESERVED_ADDR)

    # Return user's group name or None if user does not exist.
    def get_user_group(self, user_id:int) -> str:
        group = self.__rclient_user.get(user_id)
        return group

    # Return True if user was added.
    def add_user(self, user_id:int, group:str=__ALL_GRP, expire_secs:int=0) -> bool:
        if expire_secs > 0:
            status = self.__rclient_user.set(user_id, group, ex=expire_secs)
        else:
            status = self.__rclient_user.set(user_id, group)
        return status

    # Return True if user was removed.
    def remove_user(self, user_id) -> bool:
        status = self.__rclient_user.delete(user_id)
        return True if (status == 1) else False

    # Return True if group was removed.
    def remove_group(self, group) -> bool:
        status = self.__rclient_group.delete(group)
        return True if (status == 1) else False

    # Return portition of users (depending on the cursor value).
    def get_users(self, cursor:int=0):
        cursor, users = self.__rclient_user.scan(cursor, "[0-9]+", 15)
        # If returned cursor is 0 then end was reached.
        return (cursor, users)

    # Return created group's name, None if failed.
    def create_group_for_panel(self, reader_addr:int) -> str:
        group = "{}{}".format(self.__NONAME_GRP_PREFIX, reader_addr)
        if self.add_panels_to_group(group, reader_addr):
            return group
        else:
            return None

    # Return the number of readers that were added,
    # not including all the readers already present.
    def add_panels_to_group(self, group:str, readers:list) -> int:
        for reader_addr in readers:
            if self.__rclient_door.llen(reader_addr) == 0:
                self.__rclient_door.rpush(reader_addr, self.DOOR_MODE_ENABLED, acs_can_proto.DATA_DOOR_STATUS_OPEN)
        return self.__rclient_group.sadd(group, readers)

    # Return the number of readers that were removed from the set,
    # not including non existing readers.
    def remove_panels_from_group(self, group:str, readers:list) -> int:
        for reader_addr in readers:
            self.__rclient_door.delete(reader_addr)
        return self.__rclient_group.srem(group, readers)

    # Return all readers in a group. Note that this can be a demanding operation.
    def get_panels_in_group(self, group:str):
        return self.__rclient_group.smembers(group)

    # Return True if user is authorized for given reader (door) number.
    def is_user_authorized(self, user_id:int, reader_addr:int) -> bool:
        group = self.get_user_group(user_id)
        if group is None or user_id == self.__RESERVED_ADDR or group == self.__EMPTY_GRP:
            return False
        elif group == self.__ALL_GRP:
            return True
        else:
            return self.__rclient_group.sismember(group, reader_addr)

    # Log that user accessed a door/reader.
    def log_user_access(self, user_id, reader_addr):
        logging.info("User \"{}\" accessed door \"{}\"".format(user_id, reader_addr))

    def set_door_mode(self, reader_addr, mode):
        self.__rclient_door.lset(reader_addr, self.__DOOR_MODE_IDX, mode)

    def get_door_mode(self, reader_addr):
        return self.__rclient_door.lindex(reader_addr, self.__DOOR_MODE_IDX)

    # Door open/close status is tracked.
    def set_door_status(self, reader_addr, status):
        self.__rclient_door.lset(reader_addr, self.__DOOR_STATUS_IDX, status)

    # Door open/close status is tracked. Return data type for FC_DOOR_STATUS
    def get_door_status(self, reader_addr):
        return self.__rclient_door.lindex(reader_addr, self.__DOOR_STATUS_IDX)
