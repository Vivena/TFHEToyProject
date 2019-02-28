#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include "keyGestion.hpp"
#include "answers.hpp"
#include "util.hpp"
#include "benchmark.hpp"
#include "opsInEncrypt.hpp"

#define _GNU_SOURCE
#define TARGET_CCS_DIAG_CODE 122
#define FLOOR_TOTALCOST 10000
#define NB_BITS_CCSCODE 16
#define NB_BITS_TOTALCHARGES 32
#define NB_BITS_TOTALCOST 32
#define NB_LINES_DEFAULT 10000

using namespace std;

int main(int argc, char** argv) {
  //a simple arg parser
  extern char * optarg;
  int nb_lines = NB_LINES_DEFAULT;
  int opt,cmpt=0;
  uint8_t flags=0;
  extern char * optarg;

   while ((opt = getopt(argc, argv, "eb1234n:"))!= -1) {
        switch (opt){
            case 'e':
                flags += 1;
                break;
            case 'b':
                flags += 1<<1;
                break;
            case '4':
                flags += 1<<2;
                break;
            case '1':
                flags += 1<<3;
                break;
            case '2':
                flags += 1<<4;
                break;
            case '3':
                flags += 1<<5;
                break;
            case 'n':
                nb_lines=atoi(optarg);
                break;
            case '?':  // unknown option...
                cerr << "Unknown option: '" << char(opt) << "'!" << endl;
                break;
        }
    }

  int16_t plaintext1 = TARGET_CCS_DIAG_CODE;
  int32_t plaintext2 = FLOOR_TOTALCOST;
  int32_t plaintext3 = 1;

  FILE* cloud_data=nullptr;
  TFheGateBootstrappingSecretKeySet* key=nullptr;
  TFheGateBootstrappingCloudKeySet* cloudKey=nullptr;
  keyGen();
  getSecretKey(&key);
  getCloudKey(&cloudKey);
  //if necessary, the params are inside the key
  const TFheGateBootstrappingParameterSet* params = key->params;

  //we encryt the constant values in order to use them later
  LweSample* ciphertext1 = new_gate_bootstrapping_ciphertext_array(NB_BITS_CCSCODE,params);
  for (int i=0; i<NB_BITS_CCSCODE; i++) {
    bootsSymEncrypt(&ciphertext1[i], (plaintext1>>i)&1, key);
  }
  LweSample* ciphertext2 = new_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCOST, params);
  for (int i=0; i<NB_BITS_TOTALCOST; i++) {
    bootsSymEncrypt(&ciphertext2[i], (plaintext2>>i)&1, key);
  }
  LweSample* ciphertext3 = new_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES, params);
  for (int i=0; i<32; i++) {
    bootsSymEncrypt(&ciphertext3[i], (plaintext3>>i)&1, key);
  }

  //---------------------------------computation--------------------------------

  //if we have the e option, we encrypt the data set
  if(flags&1){
    if (!(cloud_data = fopen("cloud.data","wb"))) {
      cerr << "Could not open the cloud.data file"<< endl;
    }
    nb_lines = encryptFile(cloud_data,key);
    fclose(cloud_data);
  }

  if ((flags>>1)>0) {

    if (!(cloud_data = fopen("cloud.data","rb"))) {
      cerr <<"Could not open the cloud.data file"<< endl;
    }

    LweSample* ccscode=new_gate_bootstrapping_ciphertext_array(NB_BITS_CCSCODE,cloudKey->params);
    LweSample* totalCharges=new_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES,cloudKey->params);
    LweSample* totalCost=new_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCOST,cloudKey->params);

    LweSample* reponce1=new_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES,cloudKey->params);
    LweSample* reponce2=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
    LweSample* reponce3=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
    LweSample* reponce4=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);

    for (size_t i = 0; i < NB_BITS_TOTALCHARGES; i++) {
      bootsCONSTANT(&reponce1[i],0, cloudKey);
    }
    for (size_t i = 0; i < 32; i++) {
      bootsCONSTANT(&reponce2[i],0, cloudKey);
    }
    for (size_t i = 0; i < 32; i++) {
      bootsCONSTANT(&reponce3[i],0, cloudKey);
    }
    for (size_t i = 0; i < 32; i++) {
      bootsCONSTANT(&reponce4[i],0, cloudKey);
    }

    //if we want to solve question 1,2 or 3, we do it in the same loop
    if ((flags>>3)>0) {
      //for each lines of the data set
      printf("question 1-3:    0%%");
      for (size_t i = 0; i < nb_lines; i++) {

        cmpt++;
        if (cmpt*100/nb_lines<10) {
          printf("\b\b%i%%",(cmpt*100/nb_lines));
        }
        else if (cmpt*100/nb_lines<100) {
          printf("\b\b\b%i%%",(cmpt*100/nb_lines));
        }
        else
          printf("\b\b\b\bDONE");
        fflush(stdout);
        //we get the data we need
        for (int i=0; i<NB_BITS_CCSCODE; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &ccscode[i],params);
        for (int i=0; i<NB_BITS_TOTALCHARGES; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalCharges[i],params);
        for (int i=0; i<NB_BITS_TOTALCOST; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalCost[i], params);

        if(flags&(1<<3))question1(reponce1,ciphertext1,ccscode,totalCharges,NB_BITS_CCSCODE,NB_BITS_TOTALCHARGES,cloudKey);
        if(flags&(1<<4))question2(reponce2,ciphertext3,totalCharges,32,cloudKey);
        if(flags&(1<<5))question3(reponce3,ciphertext2,ciphertext3,totalCost,32,cloudKey);

      }
    }

    //if we want to answer question 4
    if(flags&(1<<2))question4(reponce4,ciphertext2,ciphertext3,32,NB_BITS_CCSCODE,NB_BITS_TOTALCHARGES,NB_BITS_TOTALCHARGES,nb_lines,std::thread::hardware_concurrency(),cloudKey);

    //print the answers
    if (flags&(1<<3)) {
      printf("The sum of all TotalCharges values where CCS Diagnosis Code = 122 is: ");
      printLWESample(reponce1,NB_BITS_TOTALCHARGES,key);
    } else if (flags&(1<<4)) {
      printf("The number of even values in TotalCharges is: ");
      printLWESample(reponce2,32,key);
    }else if (flags&(1<<5)) {
      printf("The number of TotalCost>10000 is: ");
      printLWESample(reponce3,32,key);
    }else if (flags&(1<<2)){
      printf("(Parallelized) The number of TotalCost>10000 is: ");
      printLWESample(reponce4,32,key);
    }

    //if we want to benchmark the adder function
    if(flags&(1<<1))benchmark(ciphertext3,cloudKey,key);




    fclose(cloud_data);
    delete_gate_bootstrapping_ciphertext_array(NB_BITS_CCSCODE, ccscode);
    delete_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES, totalCharges);
    delete_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCOST, totalCost);
    delete_gate_bootstrapping_ciphertext_array(32, reponce4);
    delete_gate_bootstrapping_ciphertext_array(32, reponce3);
    delete_gate_bootstrapping_ciphertext_array(32, reponce2);
    delete_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES, reponce1);
  }
  //-----------------------------------clean------------------------------------

  //clean up all pointers
  delete_gate_bootstrapping_ciphertext_array(32, ciphertext3);
  delete_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCOST, ciphertext2);
  delete_gate_bootstrapping_ciphertext_array(NB_BITS_CCSCODE, ciphertext1);
  delete_gate_bootstrapping_secret_keyset(key);
  return 0;
}
