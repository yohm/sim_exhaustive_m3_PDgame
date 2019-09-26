#!/bin/bash -eux

SCRIPT_DIR=$(cd $(dirname $0); pwd)
abspath=`greadlink -f "$1"`
IN_DIR=$(dirname "$abspath")

cd "$IN_DIR"
"$SCRIPT_DIR/cmake-build-release/main_print_ITG" "$abspath" cccccd > itg_cccccd.txt
"$SCRIPT_DIR/cmake-build-release/main_print_ITG" "$abspath" dddddc > itg_dddddc.txt

