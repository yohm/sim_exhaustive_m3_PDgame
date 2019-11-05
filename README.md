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

Run the exhaustive search.

```
ruby ~/work/sim_exhaustive_m3_PDgame/cpp/split.rb init_d_e_d_e_d_e_d_e_d 8 candidates_s%01d
./k_build.sh    # to build the code at K
# [TODO] edit job.sh appropriately
pjsub job.sh
```

