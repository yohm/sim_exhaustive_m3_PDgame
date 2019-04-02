#!/bin/sh
#============ pjsub Options ============
#PJM --rsc-list "node=768"
#PJM --rsc-list "elapse=04:00:00"
#PJM --rsc-list "rscgrp=large"
#PJM --mpi "proc=6144"
#PJM --stg-transfiles all
#PJM --mpi "use-rankdir"
#PJM --stgin  "rank=* ./main_defensible.out %r:./"
#PJM --stgin  "rank=* ./result_allD_checked/allD_checked_%04r %r:./"
#PJM --stgout "rank=* %r:./defensible_%04r %j/"
#PJM -s

. /work/system/Env_base

ulimit -s 8192

mpiexec ./main_defensible.out allD_checked_%04d defensible_%04d

