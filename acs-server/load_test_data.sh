#!/bin/sh
cat _test_dataset.txt | redis-cli -h localhost -p 6379 --pipe