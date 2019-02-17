#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include "opsInEncrypt.hpp"
#include "util.hpp"

using namespace std;
mutex mtx;

//Resolution of the first question, if the ccscode is equal to 122, we change
//the solution to solution+totalcharges
void question1(LweSample* reponce1,const LweSample* ciphertext1,const LweSample* ccscode,const LweSample* totalcharges,const int nb_bits_ccsCode,const int nb_bits_totalCharges,const TFheGateBootstrappingCloudKeySet* cloudKey){
  LweSample* tmp1=new_gate_bootstrapping_ciphertext(cloudKey->params);
  LweSample* tmp2=new_gate_bootstrapping_ciphertext_array(nb_bits_totalCharges,cloudKey->params);

  isEqual(tmp1,ciphertext1,ccscode,nb_bits_ccsCode,cloudKey);
  rippleCarryAdder(tmp2,reponce1,totalcharges,nb_bits_totalCharges,cloudKey);
  for (size_t i = 0; i < nb_bits_totalCharges; i++) {
    bootsMUX(&reponce1[i], tmp1, &tmp2[i], &reponce1[i],cloudKey);
  }

  delete_gate_bootstrapping_ciphertext_array(nb_bits_totalCharges, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
}

//Resolution of the second question, if the LSB of totalcharges is 0, we add
//1 to the final answer
void question2(LweSample* reponce2,const LweSample* ciphertext3,const LweSample* totalCharges,const int nb_bits_addition,const TFheGateBootstrappingCloudKeySet* cloudKey){
  LweSample* tmp=new_gate_bootstrapping_ciphertext_array(nb_bits_addition,cloudKey->params);

  rippleCarryAdder(tmp,reponce2,ciphertext3,nb_bits_addition,cloudKey);
  for (size_t i = 0; i < nb_bits_addition; i++) {
    bootsMUX(&reponce2[i],&totalCharges[0],&reponce2[i],&tmp[i],cloudKey);
  }

  delete_gate_bootstrapping_ciphertext_array(nb_bits_addition, tmp);
}

//resolution of the 3rd question, if the totalCost is greater than 10000, we
//add 1 to the final answer
void question3(LweSample* reponce3,const LweSample* ciphertext2,const LweSample* ciphertext3,const LweSample* totalCost,const int nb_bits_addition,const TFheGateBootstrappingCloudKeySet* cloudKey){
  LweSample* tmp1=new_gate_bootstrapping_ciphertext(cloudKey->params);
  LweSample* tmp2=new_gate_bootstrapping_ciphertext_array(nb_bits_addition,cloudKey->params);

  isGreater(tmp1,totalCost,ciphertext2,32,cloudKey);
  rippleCarryAdder(tmp2,reponce3,ciphertext3,32,cloudKey);
  for (size_t i = 0; i < 32; i++) {
    bootsMUX(&reponce3[i], tmp1, &tmp2[i], &reponce3[i],cloudKey);
  }

  delete_gate_bootstrapping_ciphertext_array(nb_bits_addition, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
}

void question3mutex(LweSample* reponce4,const LweSample* ciphertext2,const LweSample* ciphertext3,const int nb_bits_addition,const long int offsetGetCost,const long int lengthLine,const int nb_bits_totalCost,long int nb_lines,uint16_t nb_thread,const TFheGateBootstrappingCloudKeySet* cloudKey,const uint16_t num_thread){
  long int offset,linesToRead;
  FILE* cloud_data;

  LweSample* tmp1=new_gate_bootstrapping_ciphertext(cloudKey->params);
  LweSample* tmp2=new_gate_bootstrapping_ciphertext_array(nb_bits_addition,cloudKey->params);
  LweSample* totalCost=new_gate_bootstrapping_ciphertext_array(nb_bits_totalCost,cloudKey->params);

  if (!(cloud_data = fopen("cloud.data","rb"))) {
    throw "Could not open the cloud.data file";
  }

  //we go to the offset for this particular thread
    //offset = number of the thread * number of line in each subset * number of
    //bits in each lines
  offset= num_thread*(nb_lines/nb_thread)*(lengthLine);
  linesToRead=nb_lines/nb_thread;
    //if the rest of the division of the number of lines by the number of threads
    //is not equal to 0, we add 1 line by thread (starting from thread 1 and
    //upward), util thoses extra lines are all distributed
  if (nb_lines%nb_thread!=0) {
    if (num_thread>(nb_lines%nb_thread)) {
      offset+=(nb_lines%nb_thread-1)*(lengthLine);
    }else{
      offset+=num_thread*(lengthLine);
      linesToRead++;
    }
  }
  fseek ( cloud_data , offset , SEEK_SET );
  printf("thread %hu: %li lines to read starting line %li\n", num_thread,linesToRead, offset/(lengthLine) );

  for (size_t i = 0; i < linesToRead; i++) {
    fseek ( cloud_data , offsetGetCost , SEEK_CUR	);
    for (int i=0; i<nb_bits_totalCost; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalCost[i], cloudKey->params);

    isGreater(tmp1,totalCost,ciphertext2,32,cloudKey);
    mtx.lock();
    rippleCarryAdder(tmp2,reponce4,ciphertext3,32,cloudKey);
    for (size_t i = 0; i < 32; i++) {
      bootsMUX(&reponce4[i], tmp1, &tmp2[i], &reponce4[i],cloudKey);
    }
    mtx.unlock();
  }

  fclose(cloud_data);
  delete_gate_bootstrapping_ciphertext_array(nb_bits_totalCost,totalCost);
  delete_gate_bootstrapping_ciphertext_array(nb_bits_addition, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
}

void question4(LweSample* reponce4,const LweSample* ciphertext2,const LweSample* ciphertext3,const int nb_bits_addition,const int nb_bits_ccsCode,const int nb_bits_totalCharges,const int nb_bits_totalCost,long int nb_lines,uint16_t nb_thread,const TFheGateBootstrappingCloudKeySet* cloudKey){
  long int x, offsetGetCost,lengthLine;
  FILE* cloud_data;
  thread* tarray = new thread[nb_thread - 1];
  LweSample* tmp1=new_gate_bootstrapping_ciphertext(cloudKey->params);
  LweSample* tmp2=new_gate_bootstrapping_ciphertext_array(nb_bits_addition,cloudKey->params);
  LweSample* totalCost=new_gate_bootstrapping_ciphertext_array(nb_bits_totalCost,cloudKey->params);

  //we get the length of ccscode+totalcharges as well as the length of a line
  if (!(cloud_data = fopen("cloud.data","rb"))) {
    throw "Could not open the cloud.data file";
  }
  x=ftell(cloud_data);
  //for the ccscode+totalcharges
  for (size_t i = 0; i < nb_bits_ccsCode+nb_bits_totalCharges; i++) {
    import_gate_bootstrapping_ciphertext_fromFile(cloud_data, tmp1, cloudKey->params);
  }
  offsetGetCost=ftell(cloud_data)-x;
  //for the line
  for (size_t i = 0; i <nb_bits_totalCost; i++) {
    import_gate_bootstrapping_ciphertext_fromFile(cloud_data, tmp1, cloudKey->params);
  }
  lengthLine=ftell(cloud_data)-x;
  fseek(cloud_data,0,SEEK_SET);

  //we launch the N-1 threads
  for (size_t i = 0; i < nb_thread - 1; i++) {
    tarray[i] = thread(question3mutex,reponce4,ciphertext2,ciphertext3,nb_bits_addition,offsetGetCost,lengthLine,nb_bits_totalCost,nb_lines,nb_thread,cloudKey,i);
  }

  //the main thread will work on the last set of lines

  long int offset= ( (nb_thread - 1)*((nb_lines/nb_thread)+(nb_lines%nb_thread)))*(lengthLine);
  long int linesToRead=nb_lines/nb_thread;
  printf("main thread: %li lines to read starting line %li \n", linesToRead, offset/lengthLine);
  fseek ( cloud_data , offset , SEEK_SET );

  for (size_t i = 0; i < linesToRead; i++) {
    fseek ( cloud_data , offsetGetCost , SEEK_CUR	);
    for (int i=0; i<nb_bits_totalCost; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalCost[i], cloudKey->params);

    isGreater(tmp1,totalCost,ciphertext2,32,cloudKey);
    mtx.lock();
    rippleCarryAdder(tmp2,reponce4,ciphertext3,32,cloudKey);
    for (size_t i = 0; i < 32; i++) {
      bootsMUX(&reponce4[i], tmp1, &tmp2[i], &reponce4[i],cloudKey);
    }
    mtx.unlock();
  }

  for (size_t i = 0; i < nb_thread - 1; i++) {
    tarray[i].join();
  }

  delete [] tarray;
  delete_gate_bootstrapping_ciphertext_array(nb_bits_totalCost,totalCost);
  delete_gate_bootstrapping_ciphertext_array(nb_bits_addition, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
}
