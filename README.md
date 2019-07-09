# Comprehensive enumeration of successful strategies for memory-3 Prisoner's Dilemma game

## Build

```
mkdir build
cd build
cmake ..
make
```


## Step 1

Initial filtering.

```
./cmake-build-release/main_efficient_defensible init_strategy 8 > init_e
./cmake-build-release/main_trace_negative_defensible init_e 8 > init_e_d
./cmake-build-release/main_efficient_defensible init_e_d 16 > init_e_d_e
./cmake-build-release/main_trace_negative_defensible init_e_d_e 8 > init_e_d_e_d
./cmake-build-release/main_efficient_defensible init_e_d_e_d 24 > init_e_d_e_d_e
./cmake-build-release/main_trace_negative_defensible init_e_d_e_d_e 8 > init_e_d_e_d_e_d
./cmake-build-release/main_efficient_defensible init_e_d_e_d_e_d 32 > init_e_d_e_d_e_d_e   # all the filtered strategies have 1-bit tolerance
./cmake-build-release/main_trace_negative_defensible init_e_d_e_d_e_d_e 8 > init_e_d_e_d_e_d_e_d
```

## Step 2

Copy `init_e_d_e_d_e_d_e_d`. Split them into 8 files. And submit a job.

```
cd cpp
./k_build.sh
pjsub job.sh
```

