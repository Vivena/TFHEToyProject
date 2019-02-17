#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

#define SPLIT_CHAR ','

using namespace std;

//encrypt the values needed and return the number of lines
int encryptFile(FILE * cloud_data, TFheGateBootstrappingSecretKeySet* key){
  //we read the file line by line
  int cmpt =0;
  uint16_t tmp16;
  uint32_t tmp32;
  size_t start,end;
  string line;
  char split_char = SPLIT_CHAR;
  vector<string> splited;

  const TFheGateBootstrappingParameterSet* params = key->params;

  LweSample* ciphertext16 = new_gate_bootstrapping_ciphertext_array(16, params);
  LweSample* ciphertext32 = new_gate_bootstrapping_ciphertext_array(32, params);


  ifstream myfile("TestData.csv");
  if(myfile){
    //we throw away the first line (but it might be interesting to parse it in
    //order to have a more flexible parser)
    getline(myfile,line);
    //for each lines, we extract the needed information
    printf("encyption:    0%%");
    while(getline(myfile,line)){
      //we split the string
      start=0;
      end=line.find_first_of(split_char);
      splited.clear();
      while (end <= std::string::npos) {
        splited.emplace_back(line.substr(start, end-start));
        if (end == std::string::npos) break;
        start=end+1;
        end=line.find_first_of(split_char, start);
      }
      //Then we encrypt en write the needed informations
      //For the CS Diagnosis Code
      tmp16=strtol(splited[1].c_str(),nullptr,0);
      for (int i=0; i<16; i++) {
        bootsSymEncrypt(&ciphertext16[i], (tmp16>>i)&1, key);
      }
      for (int i=0; i<16; i++){
        export_gate_bootstrapping_ciphertext_toFile(cloud_data, &ciphertext16[i], params);
      }
      //For the Total Charges
      tmp32=strtol(splited[4].c_str(),nullptr,10);
      for (int i=0; i<32; i++) {
        bootsSymEncrypt(&ciphertext32[i], (tmp32>>i)&1, key);
      }
      for (int i=0; i<32; i++){
        export_gate_bootstrapping_ciphertext_toFile(cloud_data, &ciphertext32[i], params);
      }
      //For the Total Costs
      tmp32=strtol(splited[5].c_str(),nullptr,10);
      for (int i=0; i<32; i++) {
        bootsSymEncrypt(&ciphertext32[i], (tmp32>>i)&1, key);
      }
      for (int i=0; i<32; i++){
        export_gate_bootstrapping_ciphertext_toFile(cloud_data, &ciphertext32[i], params);
      }

      //Write completion percentage (we use as assumption that there is 10000
      //lines in the CSV)
      cmpt++;
      if (cmpt/100<10) {
        printf("\b\b%i%%",(cmpt/100));
      }
      else if (cmpt/100<100) {
        printf("\b\b\b%i%%",(cmpt/100));
      }
      else
        printf("\b\b\b\b%i%%",(cmpt/100));
      fflush(stdout);
    }
    printf("\n");
  }
  else{
    throw "Could not open the TestData.csv file.";
  }
  return cmpt;
}

void printLWESample(const LweSample* answer,const uint16_t nb_bits,const TFheGateBootstrappingSecretKeySet* key){
  int32_t int_answer = 0;
    for (int i=0; i<nb_bits; i++) {
        int ai = bootsSymDecrypt(&answer[i], key);
        int_answer |= (ai<<i);
    }
    printf("%i\n",int_answer);
}
