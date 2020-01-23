import redis
import logging

class acs_database(object):
    """
        ACS server database model based on Redis key-value store.

        The redis store can be used as stand-alone database or only as a cache
        that is updated from some main database.

        Containts users that belong to groups.
        If user is part of a group it means that he is authorized to use doors in the group.

        User ID database
            - value is group name (can be part of only one group)
            - key must be an unique number (raw number of RFID card or chip)
        Group database
            - value is set of of door/reader addresses that member of the group can use
            - key is a string (unique group name) and must not start with "__"
        Door database
            - value is list containing mode and status
            - key must be door address (unique)
    """

    __ALL_GRP = b"__all"  # Special group representing all doors.
    __EMPTY_GRP = b"__void"  # Special empty group.
    __LEARN_GRP = b"__learn"  # Special group for switching between on and learn.
    __NONAME_GRP_PREFIX = "__nng_"
    __RESERVED_ADDR = 0
    __DEFAULT_HOST = "localhost"
    __DEFAULT_PORT = 6379
    __DOOR_MODE_IDX = 0
    __DOOR_STATUS_IDX = 1

    # user auth types
    USER_AUTH_FAIL = 0
    USER_AUTH_OK = 1
    USER_NOT_EXIST = 2
    USER_AUTH_LEARN = 3

    # Doors can be disabled to prevent access. Or switched to learn mode where authorization request
    # is interpreted that door is unlocked and user is added to database and given access to source door.
    DOOR_MODE_DISABLE = b"off"
    DOOR_MODE_ENABLED = b"on"
    DOOR_MODE_LEARN = b"learn"
    DOOR_STATUS_OPEN = b"open"
    DOOR_STATUS_CLOSED = b"closed"

    def __init__(self, host=__DEFAULT_HOST, port=__DEFAULT_PORT):
        # create database connection
        self.__rclient_user = redis.Redis(host, port, db=0, password=None, encoding='utf-8',
            socket_timeout=10, socket_keepalive=True, retry_on_timeout=True)
        self.__rclient_group = redis.Redis(host, port, db=1, password=None, encoding='utf-8',
            socket_timeout=10, socket_keepalive=True, retry_on_timeout=True)
        self.__rclient_door = redis.Redis(host, port, db=2, password=None, encoding='utf-8',
            socket_timeout=10, socket_keepalive=True, retry_on_timeout=True)

        # create basic groups (if not present)
        self.add_doors_to_group(self.__ALL_GRP, self.__RESERVED_ADDR)
        self.add_doors_to_group(self.__EMPTY_GRP, self.__RESERVED_ADDR)
        self.add_doors_to_group(self.__LEARN_GRP, self.__RESERVED_ADDR)

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

    # Return portition of groups (depending on the cursor value).
    def get_groups(self, cursor:int=0):
        cursor, users = self.__rclient_user.scan(cursor, ".*", 15)
        # If returned cursor is 0 then end was reached.
        return (cursor, users)

    # Return created group's name, None if failed.
    def create_group_for_door(self, door_addr:int) -> str:
        group = "{}{}".format(self.__NONAME_GRP_PREFIX, door_addr)
        if self.add_doors_to_group(group, door_addr):
            return group
        else:
            return None

    # Return the number of doors that were added,
    # not including all the doors already present.
    def add_doors_to_group(self, group:str, *doors) -> int:
        for door_addr in doors:
            if self.__rclient_door.llen(door_addr) == 0:
                self.__rclient_door.rpush(door_addr, self.DOOR_MODE_ENABLED, self.DOOR_STATUS_CLOSED)
        return self.__rclient_group.sadd(group, *doors)

    # Return the number of doors that were removed from the set,
    # not including non existing doors.
    def remove_doors_from_group(self, group:str, *doors) -> int:
        for door_addr in doors:
            self.__rclient_door.delete(door_addr)
        return self.__rclient_group.srem(group, doors)

    # Return all doors in a group. Note that this can be a demanding operation.
    def get_doors_in_group(self, group:str):
        return self.__rclient_group.smembers(group)

    # Return true if door is in database
    def is_door_registered(self, door_addr:int) -> bool:
        self.__rclient_door.llen(door_addr) == 2

    # Return user authorization for given door address.
    def user_authorization(self, user_id:int, door_addr:int) -> bool:
        group = self.get_user_group(user_id)
        if group is None:
            return self.USER_NOT_EXIST
        elif user_id == self.__RESERVED_ADDR or group == self.__EMPTY_GRP:
            return self.USER_AUTH_FAIL
        elif group == self.__ALL_GRP:
            return self.USER_AUTH_OK
        elif group == self.__LEARN_GRP:
            return self.USER_AUTH_LEARN
        elif self.__rclient_group.sismember(group, door_addr):
            return self.USER_AUTH_OK
        else:
            return self.USER_AUTH_FAIL

    # Log that user accessed a door/door.
    def log_user_access(self, user_id, door_addr):
        logging.info("User \"{}\" accessed \"{}\"".format(user_id, door_addr))

    # Mode is one of DOOR_MODE_...
    def set_door_mode(self, door_addr, mode):
        self.__rclient_door.lset(door_addr, self.__DOOR_MODE_IDX, mode)

    # Return one of DOOR_MODE_...
    def get_door_mode(self, door_addr):
        door_mode = self.__rclient_door.lindex(door_addr, self.__DOOR_MODE_IDX)
        if door_mode is None:
            logging.warning("Door {} does not exist! Check DB consistency.".format(door_addr))
        return door_mode

    # Door open/close status is tracked.
    def set_door_is_open(self, door_addr, is_open:bool):
        status = self.DOOR_STATUS_CLOSED
        if is_open:
            status = self.DOOR_STATUS_OPEN
        self.__rclient_door.lset(door_addr, self.__DOOR_STATUS_IDX, status)

    # Door open/close status is tracked.
    def is_door_open(self, door_addr) -> bool:
        if self.__rclient_door.lindex(door_addr, self.__DOOR_STATUS_IDX) == self.DOOR_STATUS_OPEN:
            return True
        else:
            return False
