# TFHEToyProject

TFHEToyProject is a toy project that uses the TFHE library.

# Getting Started

## Prerequisites

This project need the TFHE library in order to run.
Please find the informations on how to install this library [here](https://tfhe.github.io/tfhe/installation.html).

Please set the ```CMAKE_BUILD_TYPE``` to ```optim```

## Installing

To compile the source code, go to the root folder of this project.

```
make
```

# Running

To run the program: ```./bin/test```followed by the options you want.
For example:

```
./bin/test -e -1 #encrypt the .csv file and answer the question 1 of the test
```
You can also separate it in multiple call to the program

```
./bin/test -e #encrypt the .csv file
./bin/test -1 -n 10000 #number of lines in .cvs = 10000 and answer the question 1
```
> Note that you will first need to encrypt the .csv file before any other calls.

> By default the number of lines in .cvs is 10000, it is also changed to the
number of lines read during encryption if the -e option is used

## List of options

option | result
--- | ---
-e | encrypt the .cvs file
-b | benchmark the adder function
-n | fix the number of lines in present in the .csv if -e is not used (by default 10000)
-1 | answer question 1: Add all TotalCharges values where CCS Diagnosis Code = 122
-2 | answer question 2: Count number of even values in TotalCharges
-3 | answer question 3: Count number of TotalCost>10000.
-4 | answer question 4: Parallelize Question 3 using multi threading. 
