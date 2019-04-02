# exhaustive enumeration of successful strategies for three-person public goods game

Source code for the following study.  
[Y. Murase and S.K. Baek, "Seven rules to avoid the tragedy of the commons", J.Theor.Biol. (2018)](https://www.sciencedirect.com/science/article/pii/S0022519318301954)

## Step 1

The first step is defensibility against allD.

```
./ruby/bin/run_defensibility_check.sh
```

The files containing the strategies passing the AllD-defensibility check are created in "result_allD_checked".

## Step 2

Then, we filter out these strategies by the defensibility condition.

To compile the program on K computer, run

```
cd cpp
./k_build.sh
```

The script to submit the job is also included in the repository.

```
pjsub job_defensible_768.sh
```

Edit the script to fix the path of input files.

Then, submit the job to check efficiency condition.

```
pjsub job_efficiency_768.sh
```

This will filter out the strategies based on the efficiency condition.

## Step 3

To filter out the strategy based on the distinguishability condition, run the following.
MPI execution is not necessary since the number of strategy is not so large.

```
./main_distinguishable
```

Then, we filter out strategies using the efficiency condition.

## Step 4

To convert m=2 strategies into m=3 strategies and make them successful, run the following script.

```
cd ruby
ruby ./bin/convert_m2_into_m3.rb filtered_strategies.txt > m3_strategies
```

Then, run the defensibility check

```
cd cpp
./main_m3_defensible m3_strategies m3_defensible
```

To calculate the efficiency,

```
ruby ruby/bin/split.rb m3_defensible 8 m3_defensible%02d
mpiexec -n 8 main_m3_efficiency 0.001 m3_defensible%02d m3_defensible_efficiency_%02d
cat m3_defensible_efficiency_* > m3_defensible_efficiency
```

## (optional) Step 5

To calculate the second largest eigenvalues for the successful strategies,

```
cat m3_defensible | xargs -n1 ./calc_second_eig.sh > second_eigs.txt
cat -n second_eigs.txt | sort --key=2 -n > second_eigs_sorted.txt
```

You'll find the second eigenvalues sorted by the size.
