#!/bin/sh
#============ pjsub Options ============
#PJM --rsc-list "node=5400"
#PJM --rsc-list "elapse=03:00:00"
#PJM --rsc-list "rscgrp=large"
#PJM --mpi "proc=43200"
#PJM --stg-transfiles all
#PJM --mpi "use-rankdir"
#PJM --stgin  "rank=* ./main_topological_efficiency.out %r:./"
#PJM --stgin  "rank=0-5399 /data/ra000014/a03115/m3_PDgame/candidates_s0 %r:./"
#PJM --stgin  "rank=5400-10799 /data/ra000014/a03115/m3_PDgame/candidates_s1 %r:./"
#PJM --stgin  "rank=10800-16199 /data/ra000014/a03115/m3_PDgame/candidates_s2 %r:./"
#PJM --stgin  "rank=16200-21599 /data/ra000014/a03115/m3_PDgame/candidates_s3 %r:./"
#PJM --stgin  "rank=21600-26999 /data/ra000014/a03115/m3_PDgame/candidates_s4 %r:./"
#PJM --stgin  "rank=27000-32399 /data/ra000014/a03115/m3_PDgame/candidates_s5 %r:./"
#PJM --stgin  "rank=32400-37799 /data/ra000014/a03115/m3_PDgame/candidates_s6 %r:./"
#PJM --stgin  "rank=37800-43199 /data/ra000014/a03115/m3_PDgame/candidates_s7 %r:./"
#PJM --stgout "rank=* %r:./out.%05r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM --stgout "rank=* %r:./out.pending.%05r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM --stgout "rank=* %r:./stderr.txt.%r /data/ra000014/a03115/m3_PDgame/%j/%r/"
#PJM -s

. /work/system/Env_base

ulimit -s 8192

mpiexec -ofout-proc stdout.txt -oferr-proc stderr.txt ./main_topological_efficiency.out candidates_s%01d 8 out.%05d out.pending.%05d

