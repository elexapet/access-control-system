#!/bin/sh

# Create single executable package from python sources

# Build parameters
APP_BIN=acs-server
WORK_DIR=dist
INPUT_DIR=src
INSTALL_DIR=/usr/local/bin

rm -rf $WORK_DIR
mkdir $WORK_DIR
cp -r $INPUT_DIR/* $WORK_DIR

# echo "Collecting requirements"

# python3 -m pip install -r requirements.txt --upgrade --target $WORK_DIR
# if [ $? -ne 0 ] ; then
#     echo "Error occured"
#     exit 1
# fi

echo "Packing sources..."
python3 -m zipapp -p "/usr/bin/env python3" -m "acs_server:main" -o $APP_BIN $WORK_DIR
if [ $? -ne 0 ] ; then
    echo "Error occured"
    exit 1
fi

echo "Done"

sudo cp $APP_BIN $INSTALL_DIR/$APP_BIN
if [ $? -eq 0 ] ; then
    echo "Installed to $INSTALL_DIR/$APP_BIN"
fi

