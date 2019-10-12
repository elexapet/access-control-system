import redis

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

    def __init__(self, host=__DEFAULT_HOST, port=__DEFAULT_PORT):
        self.__rclient = redis.Redis(host, port, db=0, password=None,
            socket_timeout=10, socket_keepalive=True, retry_on_timeout=True)

        # create basic groups (if not present)
        self.add_panels_to_group(self.__ALL_GRP, self.__RESERVED_ID)
        self.add_panels_to_group(self.__EMPTY_GRP, self.__RESERVED_ID)

    # Return user's group name or None if user does not exist.
    def get_user_group(self, user_id:int) -> str:
        group = self.__rclient.get(user_id)
        return group

    # Return True if user was added.
    def add_user(self, user_id:int, group:str=__ALL_GRP, expire_secs:int=0) -> bool:
        if expire_secs > 0:
            status = self.__rclient.set(user_id, group, ex=expire_secs)
        else:
            status = self.__rclient.set(user_id, group)
        return status

    # Return: True if user/group was removed
    def remove_user_or_group(self, user_or_group) -> bool:
        status = self.__rclient.delete(user_or_group)
        return True if (status == 1) else False

    # Return portition of users (depending on the cursor value).
    def get_users(self, cursor:int=0):
        cursor, users = self.__rclient.scan(cursor, "[0-9]+", 15)
        # if cursor is 0 end was reached
        return (cursor, users)

    # return created group's name
    def create_group_for_panel(self, panel_id:int) -> str:
        group = "{}{}".format(self.__NONAME_GRP_PREFIX, panel_id)
        if self.add_panels_to_group(group, panel_id):
            return group
        else:
            return None

    # return: the number of elements that were added to the set,
    #   not including all the elements already present into the set.
    def add_panels_to_group(self, group:str, panels:list) -> int:
        return self.__rclient.sadd(group, panels)

    # return: the number of members that were removed from the set,
    #   not including non existing members.
    def remove_panels_from_group(self, group:str, panels:list) -> int:
        return self.__rclient.srem(group, panels)

    # can be demanding (do not use often)
    def get_panels_in_group(self, group:str):
        return self.__rclient.smembers(group)

    def is_user_authorized(self, user_id:int, panel:int) -> bool:
        group = self.get_user_group(user_id)
        if group is None or user_id == self.__RESERVED_ID or group == self.__EMPTY_GRP:
            return False
        elif group == self.__ALL_GRP:
            return True
        else:
            return self.__rclient.sismember(group, panel)