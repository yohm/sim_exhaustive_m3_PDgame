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
../cmake-build-release/main_trace_negative_defensible init init_d 12
../cmake-build-release/main_trace_gSS_from_cccccd init_d.0 8 > init_d_e
mpiexec -n 12 ../cmake-build-release/main_trace_negative_defensible init_d_e init_d_e_d 8
cat init_d_e_d.* > init_d_e_d
mpiexec -n 12 ../cmake-build-release/main_trace_gSS_from_cccccd init_d_e_d init_d_e_d_e 16
cat init_d_e_d_e.* > init_d_e_d_e
mpiexec -n 12 ../cmake-build-release/main_trace_negative_defensible init_d_e_d_e init_d_e_d_e_d 6
cat init_d_e_d_e_d.* > init_d_e_d_e_d
mpiexec -n 12 ../cmake-build-release/main_trace_gSS_from_cccccd init_d_e_d_e_d init_d_e_d_e_d_e 22
cat init_d_e_d_e_d_e.* > init_d_e_d_e_d_e
mpiexec -n 12 ../cmake-build-release/main_trace_negative_defensible init_d_e_d_e init_d_e_d_e_d 6
cat init_d_e_d_e_d_e_d.* > init_d_e_d_e_d_e_d
mpiexec -n 12 ../cmake-build-release/main_trace_gSS_from_cccccd init_d_e_d_e_d_e_d init_d_e_d_e_d_e_d_e 32
cat init_d_e_d_e_d_e_d_e.* > init_d_e_d_e_d_e_d_e
mpiexec -n 12 ../cmake-build-release/main_trace_negative_defensible init_d_e_d_e_d_e_d_e init_d_e_d_e_d_e_d_e_d 3
cat init_d_e_d_e_d_e_d_e_d.* > init_d_e_d_e_d_e_d_e_d
wc -l init_d_e_d_e_d_e_d_e_d                     #=> 61,860,400  (3.7GB)
```

## Step 2

Run the exhaustive search.

```
ruby ~/work/sim_exhaustive_m3_PDgame/cpp/split.rb init_d_e_d_e_d_e_d_e_d 8 candidates_s%01d
./k_build.sh    # to build the code at K
# [TODO] edit job.sh appropriately
pjsub job.sh
```

