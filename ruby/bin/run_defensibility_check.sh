#!/bin/bash -eux

SCRIPT_DIR=$(cd $(dirname $0);pwd)

mkdir -p temp
ruby ${SCRIPT_DIR}/filter_out_allD.rb > temp/allD_pattern
cat temp/allD_pattern | xargs -P 12 -n 1 -t -I{} sh -c "ruby ${SCRIPT_DIR}/enumerate_strategy.rb \"\$1\" > \"temp/\$1_out\"" -- {}

OUT_DIR=result_allD_checked
mkdir -p ${OUT_DIR}
cat -- temp/-*d_out > ${OUT_DIR}/allD_checked
rm -rf temp
ruby ${SCRIPT_DIR}/split.rb ${OUT_DIR}/allD_checked 6144 ${OUT_DIR}/allD_checked_%04d

