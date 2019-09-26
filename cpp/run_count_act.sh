#!/bin/bash -eux

SCRIPT_DIR=$(cd $(dirname $0); pwd)
abspath=`greadlink -f "$1"`
IN_DIR=$(dirname "$abspath")

cd "$IN_DIR"
ruby "$SCRIPT_DIR/count_act.rb" "$abspath" > count_act.txt

