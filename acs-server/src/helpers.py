import argparse
import sys
import errno

def format_data(data) -> str:
    # return "".join([hex(byte)[2:] for byte in data])
    return "".join('%02x'%byte for byte in data)

def parse_args():
    parser = argparse.ArgumentParser(description='Access control system server')

    parser.add_argument('interface', type=str, default='can0', help='CAN interface name (can0, vcan1, ...)')
    parser.add_argument('id', type=int, default='0', help='ACS server(master) address (0 or 1)')
    parser.add_argument('redis_hostname', type=str, default='localhost', help='Redis server hostname')
    parser.add_argument('redis_port', type=int, default='6379', help='Redis server port')
    parser.add_argument("-v", "--verbose", help="increase output verbosity", action="store_true")
    parser.add_argument("-l", "--log_dir", type=str, help="path to dir for log (after init it will not output to console)")

    args = parser.parse_args()

    return args