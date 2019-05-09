#!/bin/sh
#============ pjsub Options ============
#PJM --rsc-list "node=540"
#PJM --rsc-list "elapse=00:30:00"
#PJM --rsc-list "rscgrp=large"
#PJM --mpi "proc=4320"
#PJM --stg-transfiles all
#PJM --mpi "use-rankdir"
#PJM --stgin  "rank=* ./main_topological_efficiency.out %r:./"
#PJM --stgin  "rank=* /data/ra000014/a03115/m3_PDgame/candidates_50 %r:./"
#PJM --stgout "rank=* %r:./out.%04r /data/ra000014/a03115/m3_PDgame/%j/"
#PJM --stgout "rank=* %r:./out.pending.%04r /data/ra000014/a03115/m3_PDgame/%j/"
#PJM --stgout "rank=* %r:./stderr.txt.%r /data/ra000014/a03115/m3_PDgame/%j/"
#PJM -s

. /work/system/Env_base

ulimit -s 8192

mpiexec -ofout-proc stdout.txt -oferr-proc stderr.txt ./main_topological_efficiency.out candidates_50 out

