# Comprehensive enumeration of successful strategies for memory-3 Prisoner's Dilemma game

C++ and ruby codes are located in 'cpp' and 'ruby' directories, respectively.
Ruby code is prepared for quick and interactive inspection and debugging while C++ code is for the actual enumeration process.

## Prerequisites

- [cmake](https://cmake.org/)
- [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
    - Eigen is a C++ template library for linear algebra: matrices, vectors, numerical solvers, and related algorithms.
    - If you're using macOS, run `brew install eigen`.
- An MPI library such as OpenMPI or mpich.

## Build

Use cmake to build the code.
If something went wrong, path to the libraries might be wrong. Edit CMakeLists.txt to make it conform to your environment.

```
cd cpp
mkdir build
cd build
cmake ..
make
```

## Step 1

Initial filtering.

```
#!/bin/bash -eux

PAR="12 --use-hwthread-cpus"
../cmake-build-release/main_trace_gS_from_negatives init init_d 12
cat init_d.* > init_d
mpiexec -n $PAR ../cmake-build-release/main_trace_gSS_from_cccccd init_d init_de 8
cat init_de.* > init_de
mpiexec -n $PAR ../cmake-build-release/main_trace_gS_from_negatives init_de init_ded 8
cat init_ded.* > init_ded
mpiexec -n $PAR ../cmake-build-release/main_trace_gSS_from_cccccd init_ded init_dede 16
cat init_dede.* > init_dede
mpiexec -n $PAR ../cmake-build-release/main_trace_gS_from_negatives init_dede init_deded 6
cat init_deded.* > init_deded
mpiexec -n $PAR ../cmake-build-release/main_trace_gSS_from_cccccd init_deded init_dedede 22
cat init_dedede.* > init_dedede
mpiexec -n $PAR ../cmake-build-release/main_trace_gS_from_negatives init_dedede init_dededed 6
cat init_dededed.* > init_dededed
mpiexec -n $PAR ../cmake-build-release/main_trace_gSS_from_cccccd init_dededed init_dededede 32
cat init_dededede.* > init_dededede
mpiexec -n $PAR ../cmake-build-release/main_trace_gS_from_negatives init_dededede init_dedededed 3
cat init_dedededed.* > init_dedededed
wc -l init_dedededed                                       #=> 61,860,400  (3.7GB)
```

## Step 2

Get the list of strategies that meet defensibility and efficiency conditions.
A supercomputer is required.

```
ruby <repository root>/ruby/split.rb init_dedededed 8 candidates_s%01d     # split the list into 8 files
./k_build.sh    # to build the code on the K computer
# [TODO] edit job.sh appropriately
pjsub job.sh
```

## Step 3

Get the list of strategies that satisfy distinguishability condition.
A supercomputer is required.

```
ruby <repository root>/ruby/merge_and_split.rb '*/out.passed.[0-9]*' 1200 'merged/passed.%05d'
# the input files located in '*/out.passed....' files are merged and then split into 1200 files in 'merged/' directory.
./k_build_dis.sh    # to build the code on the K computer
pjsub job_dis.sh
```

## Step 4 (Optional)

Against the obtained list of successful strategies, strategies are randomly sampled and check whether the defensibility and efficiency conditions are really meet.
This is to assure the results obtained in the previous steps.
To test the efficiency condition, linear algebraic calculation is conducted.

A supercomputer is required for this task.

```
./k_build_check.sh               # to build the code on the K computer
pjsub job_check_one_by_one.sh    # submit a job
```


## executable programs

Here is the list of executable programs and their usage.

- main_trace_gSS_from_cccccd
    - Filter strategy sets by the defensibility condition. Used for an initial screening.
    - Defensibility of a strategy set is judged by a graph `g(S,*)`.
        - When the defensibility for a strategy set is not determined, its child strategy sets are recursively tested.
        - If a child strategy set turned out to be not defensible, it is eliminated from the output. Thus, only the strategy sets whose defensibility is assured or undetermined are shown as the output.
        - Child strategy sets are created by fixing an action that is traced by a negative path.
- main_trace_gS_from_negatives
    - Filter strategy sets by the efficiency and defensibility conditions. Used for an initial screening.
    - Efficiency of a strategy set is judged using a graph `g(S,S)`.
        - For a strategy to be efficient, it must recover mutual cooperation from one-bit error. Thus, there must be a path from node `(ccc,ccd)` to `(ccc,ccc)` in `g(S,S)`.
        - If the path from `(ccc,ccd)` does not reach mutual cooperation, the strategy set is removed from the output.
        - When the destination of the path is not determined, its child strategy sets are recursively constructed and tested.
        - Whenever a new child strategy set is constructed, its defensibility is simultaneously checked. If it does not meet the defensibility, it is removed from the output.
- main_filter_efficient_defensible
    - Filter strategy sets that are both efficient and defensible.
    - Used for production runs after initial screening process.
    - Defensibility is judged by `g(S,*)` and efficiency is judged by `g(S,S)`.
- main_filter_distinguishable
    - Filter strategy sets that are distinguishable.
    - Used for the strategy sets that have passed the defensibility and efficiency tests.
- main_check_one_by_one
    - From the list of strategy sets, construct strategies and check if defensibility and efficiency conditions are met.
    - Used for debugging.
- main_print_ITG
    - Print the path in `g(S,S)` starting from a specified node.
    - Used for investigating how the strategies are recovered from the noise.
- count_num_strategies
    - Count the number of strategies in a list of strategy set.
    - Used for debugging.

