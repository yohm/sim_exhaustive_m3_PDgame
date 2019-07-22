#!/bin/sh
#============ pjsub Options ============
#PJM --rsc-list "node=1200"
#PJM --rsc-list "elapse=06:00:00"
#PJM --rsc-list "rscgrp=large"
#PJM --mpi "proc=9600"
#PJM --stg-transfiles all
#PJM --mpi "use-rankdir"
#PJM --stgin  "rank=* ./main_topological_efficiency.out %r:./"
#PJM --stgin  "rank=0-1199    /data/ra000014/a03115/m3_PDgame/run_2/candidates_s0 %r:./"
#PJM --stgin  "rank=1200-2399 /data/ra000014/a03115/m3_PDgame/run_2/candidates_s1 %r:./"
#PJM --stgin  "rank=2400-3599 /data/ra000014/a03115/m3_PDgame/run_2/candidates_s2 %r:./"
#PJM --stgin  "rank=3600-4799 /data/ra000014/a03115/m3_PDgame/run_2/candidates_s3 %r:./"
#PJM --stgin  "rank=4800-5999 /data/ra000014/a03115/m3_PDgame/run_2/candidates_s4 %r:./"
#PJM --stgin  "rank=6000-7199 /data/ra000014/a03115/m3_PDgame/run_2/candidates_s5 %r:./"
#PJM --stgin  "rank=7200-8399 /data/ra000014/a03115/m3_PDgame/run_2/candidates_s6 %r:./"
#PJM --stgin  "rank=8400-9599 /data/ra000014/a03115/m3_PDgame/run_2/candidates_s7 %r:./"
#PJM --stgout "rank=* %r:./out.%05r /data/ra000014/a03115/m3_PDgame/%j/%05r/"
#PJM --stgout "rank=* %r:./out.passed.%05r /data/ra000014/a03115/m3_PDgame/%j/%05r/"
#PJM --stgout "rank=* %r:./out.pending.%05r /data/ra000014/a03115/m3_PDgame/%j/%05r/"
#PJM --stgout "rank=* %r:./stderr.txt.%r /data/ra000014/a03115/m3_PDgame/%j/%05r/"
#PJM -s

. /work/system/Env_base

ulimit -s 8192

mpiexec -ofout-proc stdout.txt -oferr-proc stderr.txt ./main_topological_efficiency.out candidates_s%01d 8 out.%05d out.passed.%05d out.pending.%05d

