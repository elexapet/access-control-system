#!/bin/sh

# Default parameters to server
CAN_IF=can0
CAN_ADDR=1
REDIS_HOST=localhost
REDIS_PORT=6379

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=$(readlink -f "$0")
# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=$(dirname "$SCRIPT")

# Start the server
python3 $SCRIPTPATH/src/acs_server.py $CAN_IF $CAN_ADDR $REDIS_HOST $REDIS_PORT
