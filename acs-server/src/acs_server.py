"""
This script runs the ACS server.
"""
import errno
import signal
import sys
import os
import logging

from helpers import parse_args, format_data
from acs_database import acs_database
from acs_can_proto import acs_can_proto, can_raw_sock


class acs_server(object):
    __running = True

    def __init__(self, can_if, addr, r_host, r_port):
        self.can_if = can_if
        self.addr = addr
        try:
            self.can_sock = can_raw_sock()
            self.can_sock.bind(can_if)
            self.proto = acs_can_proto(addr, self.add_new_user, self.resp_to_auth_req)
        except Exception as e:
            logging.critical("Unable to start the server: %s", e)
            sys.exit(1)

        try:
            self.db = acs_database(r_host, r_port)
        except Exception as e:
            logging.critical("Unable to connect to Redis server: %s", e)
            sys.exit(1)

        signal.signal(signal.SIGINT, self.sigterm)
        signal.signal(signal.SIGTERM, self.sigterm)
        signal.signal(signal.SIGHUP, self.sigterm)

    def sigterm(self, signum, frame):
        if self.__running:
            self.__running = False
            logging.warning("Shutdown requested...")
        else:
            logging.warning("Forcing shutdown...")
            sys.exit(0)

    def add_new_user(self, panel_addr, user_id):
        logging.debug("add_new_user: panel = {} , user id = {}".format(panel_addr, user_id))
        group = self.db.create_group_for_panel(panel_addr)
        self.db.add_user(user_id, group)
        return self.proto.msg_new_user(panel_addr, user_id)

    def resp_to_auth_req(self, panel_addr, user_id):
        logging.debug("resp_to_auth_req: panel = {} , user id = {}".format(panel_addr, user_id))
        if self.db.is_user_authorized(user_id, panel_addr):
            return self.proto.msg_auth_ok
        else:
            return self.proto.msg_auth_fail(panel_addr, user_id)

    def switch_panel_to_learn(self, panel_addr):
        can_id, data = self.proto.msg_panel_learn(panel_addr)
        self.can_sock.send(can_id, data)
        logging.info("<-- %s: 0x%03x#0x%s" % (self.can_if, can_id, format_data(data)))

    def run(self):
        logging.warning("ACS server has started")
        logging.warning("Listening on {} with address {}".format(self.can_if, self.addr))

        while self.__running:
            try:
                if self.can_sock.try_select_recv_for(5):

                    can_id, len, data = self.can_sock.recv()

                    if can_id == 0:
                        continue

                    logging.debug("--> %s: 0x%03x#0x%s" % (self.can_if, can_id, format_data(data)))

                    can_id, data = self.proto.process_msg(can_id, len, data)

                    if can_id == 0:
                        logging.info("\tmessage ignored (or is invalid)")
                    else:
                        self.can_sock.send(can_id, data)
                        logging.debug("<-- %s: 0x%03x#0x%s" % (self.can_if, can_id, format_data(data)))

            except OSError as eos:
                logging.error("{}\n".format(os.strerror(eos.errno)))
            except Exception as e:
                logging.exception("Exception occurred: %s", e)

        self.can_sock.close()

        logging.warning("ACS server has shutdown")

def setup_logging(logname, verbose):
    if verbose:
        level = logging.DEBUG
    else:
        level = logging.WARNING

    if logname:
        logging.basicConfig(filename=logname, level=level,
                            format='%(asctime)s [%(levelname)s] %(message)s', datefmt='%d/%m/%Y %H:%M:%S')
    else:
        logging.basicConfig(level=level,
                            format='%(asctime)s [%(levelname)s] %(message)s', datefmt='%d/%m/%Y %H:%M:%S')

def main():
    pargs = parse_args()
    logname = pargs.log_dir
    if pargs.log_dir:
        logname = "{}/{}_{}.log".format(pargs.log_dir, pargs.interface, pargs.id)
    setup_logging(logname, pargs.verbose)
    acs_server(pargs.interface, pargs.id, pargs.redis_hostname, pargs.redis_port).run()

if __name__ == "__main__":
    main()

