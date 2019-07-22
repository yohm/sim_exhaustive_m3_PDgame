#!/bin/sh
#============ pjsub Options ============
#PJM --rsc-list "node=150"
#PJM --rsc-list "elapse=05:00:00"
#PJM --rsc-list "rscgrp=small"
#PJM --mpi "proc=1200"
#PJM --stg-transfiles all
#PJM --mpi "use-rankdir"
#PJM --stgin  "rank=* ./main_distinguishability.out %r:./"
#PJM --stgin  "rank=* /data/ra000014/a03115/m3_PDgame/8266388/merged/passed.%05r %r:./"
#PJM --stgout "rank=* %r:./dis.%05r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM --stgout "rank=* %r:./dis.passed.%05r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM --stgout "rank=* %r:./stderr.txt.%r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM -s

. /work/system/Env_base

ulimit -s 8192

mpiexec -ofout-proc stdout.txt -oferr-proc stderr.txt ./main_distinguishability.out passed.%05d 1200 dis.%05d dis.passed.%05d

