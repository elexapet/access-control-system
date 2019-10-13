#!/bin/sh
cat _example_redis_dataset.txt | redis-cli -h localhost -p 6379 --pipe