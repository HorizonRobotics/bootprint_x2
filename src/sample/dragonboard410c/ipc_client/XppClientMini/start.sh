#!/bin/bash

WORK_DIR=`dirname $0`

if [ "$WORK_DIR" = "." ];then
    WORK_DIR=$PWD
fi
export LD_LIBRARY_PATH=./

./xppclientmini.out lib96boardstream.so
