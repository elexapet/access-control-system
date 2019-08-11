import redis

# Database of groups and user IDs (based on Redis kv store)
# connects to Redis server
#
# group = must start with letter
# user_id = unique number
#
# user can be part of only one group
#
class acs_database(object):
    __ALL_GRP = "_all"
    __EMPTY_GRP = "_void"
    __NONAME_GRP_PREFIX = "_nng_"
    __RESERVED_ID = 0
    __DEFAULT_HOST = "localhost"
    __DEFAULT_PORT = 6379

    def __init__(self, host=__DEFAULT_HOST, port=__DEFAULT_PORT):
        self.__rclient = redis.Redis(host, port, db=0, password=None,
            socket_timeout=10, socket_keepalive=True, retry_on_timeout=True)

        # create basic groups (if not present)
        self.add_panels_to_group(self.__ALL_GRP, self.__RESERVED_ID)
        self.add_panels_to_group(self.__EMPTY_GRP, self.__RESERVED_ID)

    # return: group's name or None if user_id do not exist
    def get_user_group(self, user_id:int) -> str:
        group = self.__rclient.get(user_id)
        return group

    # Return: True if user was added
    def add_user(self, user_id:int, group:str=__ALL_GRP, expire_secs:int=0) -> bool:
        if not self.__rclient.exists(group):
            group = self.__EMPTY_GRP

        if expire_secs > 0:
            status = self.__rclient.set(user_id, group, ex=expire_secs)
        else:
            status = self.__rclient.set(user_id, group)
        return True if (status == 'OK') else False

    # Return: True if user/group was removed
    def remove_user_or_group(self, user_or_group) -> bool:
        status = self.__rclient.delete(user_or_group)
        return True if (status == 1) else False

    # return portition of users (depending on the cursor value)
    def get_users(self, cursor:int=0):
        cursor, users = self.__rclient.scan(cursor, "[0-9]+", 15)
        # if cursor is 0 end was reached
        return (cursor, users)

    # return created group's name
    def create_group_for_panel(self, panel_id:int) -> str:
        group = "{}{}".format(self.__NONAME_GRP_PREFIX, panel_id)
        self.__rclient.sadd(group, panel_id)
        return group

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
        if group is None or user_id == self.__RESERVED_ID:
            return False
        elif group == self.__ALL_GRP:
            return True
        else:
            return self.__rclient.sismember(group, panel)