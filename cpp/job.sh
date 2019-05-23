#!/bin/sh
#============ pjsub Options ============
#PJM --rsc-list "node=1200"
#PJM --rsc-list "elapse=03:00:00"
#PJM --rsc-list "rscgrp=large"
#PJM --mpi "proc=9600"
#PJM --stg-transfiles all
#PJM --mpi "use-rankdir"
#PJM --stgin  "rank=* ./main_topological_efficiency.out %r:./"
#PJM --stgin  "rank=0-149 /data/ra000014/a03115/m3_PDgame/candidates_s0 %r:./"
#PJM --stgin  "rank=150-299 /data/ra000014/a03115/m3_PDgame/candidates_s1 %r:./"
#PJM --stgin  "rank=300-449 /data/ra000014/a03115/m3_PDgame/candidates_s2 %r:./"
#PJM --stgin  "rank=450-599 /data/ra000014/a03115/m3_PDgame/candidates_s3 %r:./"
#PJM --stgin  "rank=600-749 /data/ra000014/a03115/m3_PDgame/candidates_s4 %r:./"
#PJM --stgin  "rank=750-899 /data/ra000014/a03115/m3_PDgame/candidates_s5 %r:./"
#PJM --stgin  "rank=900-1049 /data/ra000014/a03115/m3_PDgame/candidates_s6 %r:./"
#PJM --stgin  "rank=1050-1199 /data/ra000014/a03115/m3_PDgame/candidates_s7 %r:./"
#PJM --stgout "rank=* %r:./out.%05r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM --stgout "rank=* %r:./out.pending.%05r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM --stgout "rank=* %r:./stderr.txt.%r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM -s

. /work/system/Env_base

ulimit -s 8192

mpiexec -ofout-proc stdout.txt -oferr-proc stderr.txt ./main_topological_efficiency.out candidates_s%01d 8 out.%05d out.pending.%05d

