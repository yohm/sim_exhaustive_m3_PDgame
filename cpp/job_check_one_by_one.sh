#!/bin/sh
#============ pjsub Options ============
#PJM --rsc-list "node=150"
#PJM --rsc-list "elapse=05:00:00"
#PJM --rsc-list "rscgrp=small"
#PJM --mpi "proc=1200"
#PJM --stg-transfiles all
#PJM --mpi "use-rankdir"
#PJM --stgin  "rank=* ./main_check_one_by_one.out %r:./"
#PJM --stgin  "rank=* /home/ra000014/a03115/work/sim_exhaustive_m3_PDgame/cpp/data/run_2/merged/passed.%05r %r:./"
#PJM --stgout "rank=* %r:./dis.%05r /data/ra000014/a03115/m3_PDgame/%j/%05r/"
#PJM --stgout "rank=* %r:./dis.passed.%05r /data/ra000014/a03115/m3_PDgame/%j/%05r/"
#PJM --stgout "rank=* %r:./stderr.txt.%r /data/ra000014/a03115/m3_PDgame/%j/%05r/"
#PJM -s

. /work/system/Env_base

ulimit -s 8192

mpiexec -ofout-proc stdout.txt -oferr-proc stderr.txt ./main_distinguishability.out passed.%05d 1200 out.%05d 1

