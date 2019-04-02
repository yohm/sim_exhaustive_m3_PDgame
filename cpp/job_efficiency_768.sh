#!/bin/sh
#============ pjsub Options ============
#PJM --rsc-list "node=768"
#PJM --rsc-list "elapse=01:00:00"
#PJM --rsc-list "rscgrp=large"
#PJM --mpi "proc=6144"
#PJM --stg-transfiles all
#PJM --mpi "use-rankdir"
#PJM --stgin  "rank=* ./main_efficiency.out %r:./"
#PJM --stgin  "rank=* ./defensible/defensible_%04r %r:./"
#PJM --stgout "rank=* %r:./defensible_efficiency_%04r %j/"
#PJM -s

. /work/system/Env_base

ulimit -s 8192

mpiexec ./main_efficiency.out 0.001 defensible_%04d defensible_efficiency_%04d

