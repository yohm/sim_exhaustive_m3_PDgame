#!/bin/bash
set -eux

SCRIPT_DIR=$(cd $(dirname $0); pwd)
EXE=$SCRIPT_DIR/cmake-build-release/main_evolutionary_game
$EXE "$@" > _output.json

