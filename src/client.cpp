#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include "keyGestion.hpp"
#include "opsInEncrypt.hpp"
#include "util.hpp"

#define TARGET_CCS_DIAG_CODE 122
#define FLOOR_TOTALCOST 10000


int main(int argc, char const *argv[]) {

  //TODO get the values of plaintext1/2 via argv[]
  int16_t plaintext1 = TARGET_CCS_DIAG_CODE;
  int16_t plaintext2 = FLOOR_TOTALCOST;
  // int cmpt=0;
  int32_t plaintext3 = 1;
  FILE* cloud_data=nullptr;
  TFheGateBootstrappingSecretKeySet* key=nullptr;
  TFheGateBootstrappingCloudKeySet* cloudKey=nullptr;
  keyGen();
  getSecretKey(&key);
  getCloudKey(&cloudKey);
  //if necessary, the params are inside the key
  const TFheGateBootstrappingParameterSet* params = key->params;

  //we encryt the values in order to use them
  LweSample* ciphertext1 = new_gate_bootstrapping_ciphertext_array(16,params);
  for (int i=0; i<16; i++) {
    bootsSymEncrypt(&ciphertext1[i], (plaintext1>>i)&1, key);
  }

  LweSample* ciphertext2 = new_gate_bootstrapping_ciphertext_array(32, params);
  for (int i=0; i<32; i++) {
    bootsSymEncrypt(&ciphertext2[i], (plaintext2>>i)&1, key);
  }

  LweSample* ciphertext3 = new_gate_bootstrapping_ciphertext_array(32, params);
  for (int i=0; i<32; i++) {
    bootsSymEncrypt(&ciphertext3[i], (plaintext3>>i)&1, key);
  }

  printf("sample size %lu\n",sizeof(LweSample) );
  //-----------------------------------test-------------------------------------

  // if (!(cloud_data = fopen("cloud.data","wb"))) {
  //   throw "Could not open the cloud.data file";
  // }
  // int nb_lines = encryptFile(cloud_data,key);
  // fclose(cloud_data);

  if (!(cloud_data = fopen("cloud.data","rb"))) {
    throw "Could not open the cloud.data file";
  }



  LweSample* ccscode=new_gate_bootstrapping_ciphertext_array(16,cloudKey->params);
  LweSample* totalcharges=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
  LweSample* totalcost=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
  LweSample* tmp1=new_gate_bootstrapping_ciphertext(cloudKey->params);
  LweSample* tmp2=new_gate_bootstrapping_ciphertext_array(16,cloudKey->params);
  LweSample* tmp3=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);

  LweSample* reponce1=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
  LweSample* reponce2=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
  LweSample* reponce3=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);

  for (size_t i = 0; i < 32; i++) {
    bootsCONSTANT(&reponce1[i],0, cloudKey);
  }
  for (size_t i = 0; i < 32; i++) {
    bootsCONSTANT(&reponce2[i],0, cloudKey);
  }
  for (size_t i = 0; i < 32; i++) {
    bootsCONSTANT(&reponce3[i],0, cloudKey);
  }

  // printf("traitement:    0%%");
  //for each lines of the data set
  // for (size_t i = 0; i < 10; i++) {
  //   //we get the data we need
  //   for (int i=0; i<16; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &ccscode[i], params);
  //   for (int i=0; i<32; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalcharges[i], params);
  //   for (int i=0; i<32; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalcost[i], params);
  //
  //   //resolution of the first question, if the ccscode is equal to 122, we
  //   //change the solution to solution+totalcharges
  //   isEqual(tmp1,ciphertext1,ccscode,16,cloudKey);
  //   rippleCarryAdder(tmp2,reponce1,totalcharges,32,cloudKey);
  //   for (size_t i = 0; i < 32; i++) {
  //     bootsMUX(&reponce1[i], tmp1, &tmp2[i], &reponce1[i],cloudKey);
  //   }
  //
  //   //resolution of the second question, if the LSB of totalcharges is 0, we
  //   //add 1 to the final answer
  //   rippleCarryAdder(tmp3,reponce2,ciphertext3,32,cloudKey);
  //   for (size_t i = 0; i < 32; i++) {
  //     bootsMUX(&reponce2[i],&totalcharges[0],&reponce2[i],&tmp3[i],cloudKey);
  //   }
  //
  //
  //   //resolution of the 3rd question, if the totalcost is greater than 10000, we
  //   //add 1 to the final answer
  //   isGreater(tmp1,totalcost,ciphertext2,32,cloudKey);
  //   rippleCarryAdder(tmp3,reponce3,ciphertext3,32,cloudKey);
  //   for (size_t i = 0; i < 32; i++) {
  //     bootsMUX(&reponce3[i], tmp1, &tmp3[i], &reponce3[i],cloudKey);
  //   }
  //
  // }

  uint32_t cmpt=0;
  while (true) {
    cmpt++;
    rippleCarryAdder(reponce3,reponce3,ciphertext3,32,cloudKey);
    uint32_t int_answer = 0;
    for(int i=0; i<16; i++) {
      int ai = bootsSymDecrypt(&reponce3[i], key);
      int_answer |= (ai<<i);
    }
    printf("%u\n",cmpt );
    if(cmpt!=int_answer){
      printf("stop pblm!!! %u\n",cmpt );
      break;
    }

  }
  fclose(cloud_data);
  //-----------------------------------clean------------------------------------
  // //clean up all pointers
  delete_gate_bootstrapping_ciphertext_array(16, ccscode);
  delete_gate_bootstrapping_ciphertext_array(32, totalcharges);
  delete_gate_bootstrapping_ciphertext_array(32, totalcost);
  delete_gate_bootstrapping_ciphertext_array(32, tmp3);
  delete_gate_bootstrapping_ciphertext_array(16, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
  delete_gate_bootstrapping_ciphertext_array(32, reponce3);
  delete_gate_bootstrapping_ciphertext_array(32, reponce2);
  delete_gate_bootstrapping_ciphertext_array(32, reponce1);
  delete_gate_bootstrapping_ciphertext_array(32, ciphertext3);
  delete_gate_bootstrapping_ciphertext_array(32, ciphertext2);
  delete_gate_bootstrapping_ciphertext_array(16, ciphertext1);
  delete_gate_bootstrapping_secret_keyset(key);
  fclose(cloud_data);
  return 0;
}
