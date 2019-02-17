#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <chrono>
#include <thread>
#include "keyGestion.hpp"
#include "answers.hpp"
#include "util.hpp"
#include "opsInEncrypt.hpp"

#define _GNU_SOURCE
#define TARGET_CCS_DIAG_CODE 122
#define FLOOR_TOTALCOST 10000
#define NB_BITS_CCSCODE 16
#define NB_BITS_TOTALCHARGES 32
#define NB_BITS_TOTALCOST 32

int main(int argc, char const *argv[]) {

  //TODO get the values of plaintext1/2 via argv[]
  int16_t plaintext1 = TARGET_CCS_DIAG_CODE;
  int32_t plaintext2 = FLOOR_TOTALCOST;
  int32_t plaintext3 = 1;
  // int cmpt=0;

  FILE* cloud_data=nullptr;
  TFheGateBootstrappingSecretKeySet* key=nullptr;
  TFheGateBootstrappingCloudKeySet* cloudKey=nullptr;
  keyGen();
  getSecretKey(&key);
  getCloudKey(&cloudKey);
  //if necessary, the params are inside the key
  const TFheGateBootstrappingParameterSet* params = key->params;

  //we encryt the values in order to use them
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

  //-----------------------------------test-------------------------------------

  // if (!(cloud_data = fopen("cloud.data","wb"))) {
  //   throw "Could not open the cloud.data file";
  // }
  // int nb_lines = encryptFile(cloud_data,key);
  // fclose(cloud_data);

  if (!(cloud_data = fopen("cloud.data","rb"))) {
    throw "Could not open the cloud.data file";
  }



  LweSample* ccscode=new_gate_bootstrapping_ciphertext_array(NB_BITS_CCSCODE,cloudKey->params);
  LweSample* totalCharges=new_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES,cloudKey->params);
  LweSample* totalCost=new_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCOST,cloudKey->params);

  LweSample* reponce1=new_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES,cloudKey->params);
  LweSample* reponce2=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
  LweSample* reponce3=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
  LweSample* reponce4=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
  LweSample* reponce5=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
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
  for (size_t i = 0; i < 32; i++) {
    bootsCONSTANT(&reponce5[i],0, cloudKey);
  }


  //for each lines of the data set
  // for (size_t i = 0; i < nb_lines; i++) {
  // //we get the data we need
  //   for (int i=0; i<NB_BITS_CCSCODE; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &ccscode[i],params);
  //   for (int i=0; i<NB_BITS_TOTALCHARGES; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalCharges[i],params);
  //   for (int i=0; i<NB_BITS_TOTALCOST; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalCost[i], params);

  //   // question1(reponce1,ciphertext1,ccscode,totalCharges,NB_BITS_CCSCODE,NB_BITS_TOTALCHARGES,cloudKey);
  //   // question2(reponce2,ciphertext3,totalCharges,32,cloudKey);
  //   // question3(reponce3,ciphertext2,ciphertext3,totalCost,32,cloudKey);

  // }

  // question4(reponce4,ciphertext2,ciphertext3,32,NB_BITS_CCSCODE,NB_BITS_TOTALCHARGES,NB_BITS_TOTALCHARGES,10000,std::thread::hardware_concurrency(),cloudKey);


  setpriority(PRIO_PROCESS, 0, -20);

  int32_t cmp=0,res;
  int64_t elapsed_seconds=0;
  std::chrono::time_point<std::chrono::system_clock> start, end;
  for (size_t i = 0; i < 1<<31; i++) {
    cmp++;
    start = std::chrono::system_clock::now();
    rippleCarryAdder(reponce5,reponce5,ciphertext3,32,cloudKey);
    end = std::chrono::system_clock::now();
    elapsed_seconds+= std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    int32_t int_answer = 0;
      for (int i=0; i<32; i++) {
          int ai = bootsSymDecrypt(&reponce5[i], key);
          int_answer |= (ai<<i);
      }
    if (cmp!=int_answer) {
      res=cmp;
    }
    printf("cmp: %i\n",cmp );
  }
  printf("tmp: %lli\n",elapsed_seconds/10000 );

  fclose(cloud_data);
  //-----------------------------------clean------------------------------------
  // //clean up all pointers
  delete_gate_bootstrapping_ciphertext_array(NB_BITS_CCSCODE, ccscode);
  delete_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES, totalCharges);
  delete_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCOST, totalCost);

  delete_gate_bootstrapping_ciphertext_array(32, reponce4);
  delete_gate_bootstrapping_ciphertext_array(32, reponce3);
  delete_gate_bootstrapping_ciphertext_array(32, reponce2);
  delete_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCHARGES, reponce1);
  delete_gate_bootstrapping_ciphertext_array(32, ciphertext3);
  delete_gate_bootstrapping_ciphertext_array(NB_BITS_TOTALCOST, ciphertext2);
  delete_gate_bootstrapping_ciphertext_array(NB_BITS_CCSCODE, ciphertext1);
  delete_gate_bootstrapping_secret_keyset(key);
  fclose(cloud_data);
  return 0;
}
