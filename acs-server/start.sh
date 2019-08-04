#!/bin/sh
CAN_IF=can0
CAN_ADDR=0
REDIS_HOST=localhost
REDIS_PORT=6379

python3 ./src/acs_server.py -v $CAN_IF $CAN_ADDR $REDIS_HOST $REDIS_PORT