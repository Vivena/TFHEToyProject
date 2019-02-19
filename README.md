# TFHEToyProject

TFHEToyProject is a toy project that uses the TFHE library.

# Getting Started

## Prerequisites

This project needs the TFHE library in order to run.
Please find the information on how to install this library [here](https://tfhe.github.io/tfhe/installation.html).

Please set the ```CMAKE_BUILD_TYPE``` to ```optim```

## Installing

To compile the source code, go to the root folder of this project.

```
make
```

# Running

To run the program: ```./bin/test``` followed by the options you want.
For example:

```
./bin/test -e -1 #encrypt the .csv file and answer the question 1 of the test
```
You can also separate it in multiple calls to the program

```
./bin/test -e #encrypt the .csv file
./bin/test -1 -n 10000 #number of lines in .cvs = 10000 and answer the question 1
```
> Note that you will first need to encrypt the .csv file before any other calls.

> By default the number of lines in .cvs is 10000, it is also changed to the
number of lines read during encryption if the -e option is used

## List of options

Option | result
--- | ---
-e | encrypt the .cvs file
-b | benchmark the adder function
-n | fix the number of lines in present in the .csv if -e is not used (by default 10000)
-1 | answer question 1: Add all TotalCharges values where CCS Diagnosis Code = 122
-2 | answer question 2: Count number of even values in TotalCharges
-3 | answer question 3: Count number of TotalCost>10000.
-4 | answer question 4: Parallelize Question 3 using multi threading.

# Library assessment

## Is there a limit on the number of TotalCharges values, which can be added correctly using the library?

There might be a limit on the number of TotalCharges values we can add correctly.<br/>
However, I was unable to reach it after more than 10000 iterations.


## Running time for addition.

Benchmark:
 10000 additions done in 24719413 milliseconds, for an average of 2471 milliseconds per addition.
<br/>
We can also use the fact that the creators of the library tell us that it can
evaluate 76 gates per second per core.<br/>
As we don't parallelize the evaluations of gates, and as there is a total of 5
gates in the ripple carry adder, that leave us with 5*(number of bits) gates to evaluate
for a theoretical running time of 5*(number of bits)/76 seconds.<br/>
As we do the addition on 32 bits, it give us a theoretical running time of approximately
2105 milliseconds. We might be able to reduce this running time to about 1263 milliseconds (unrealistic best case scenario but good enough approximation) by parallelizing the two gates of the half adder, as doing so decrease the depth of the circuit by two gates. <br/>
The difference in running time can be explained in part with the fact that the ripple carry adder contains the carry as well as the copy of one of the added value inside the output value at the start.

## How much does the ciphertext increase in length as compared to the plaintext value?

Each lines in the set data are composed, in plaintext, of one uint16_t, and two uint32_t.
This makes the line 80 bits.<br/>
In the encrypted data set, a line takes 161280 bits. This mean that a line takes
2016 time more memory space than the plaintext.<br/>
The size also doesn't seem to change when we change the parameters for the key's bootstrapping,
but further test is needed to be categoric.
