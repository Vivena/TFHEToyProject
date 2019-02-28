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

  //we look if the ccscode is equal to 122
  isEqual(tmp1,ciphertext1,ccscode,nb_bits_ccsCode,cloudKey);
  //we pre-compute the sum of the current solution and the totalcharges of that
  //line
  rippleCarryAdder(tmp2,reponce1,totalcharges,nb_bits_totalCharges,cloudKey);
  //we change the solution to the sum we've just computed iff the ccscode was
  //equal to 122
  for (size_t i = 0; i < nb_bits_totalCharges; i++) {
    bootsMUX(&reponce1[i], tmp1, &tmp2[i], &reponce1[i],cloudKey);
  }

  //-----------------------------------clean------------------------------------
  delete_gate_bootstrapping_ciphertext_array(nb_bits_totalCharges, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
}

//Resolution of the second question, if the LSB of totalcharges is 0, we add
//1 to the final answer
void question2(LweSample* reponce2,const LweSample* ciphertext3,const LweSample* totalCharges,const int nb_bits_addition,const TFheGateBootstrappingCloudKeySet* cloudKey){
  LweSample* tmp=new_gate_bootstrapping_ciphertext_array(nb_bits_addition,cloudKey->params);

  //we pre-compute current solution plus 1
  rippleCarryAdder(tmp,reponce2,ciphertext3,nb_bits_addition,cloudKey);
  //we change the solution to the sum we've just computed iff the least
  //significant bit of the totalcharges is equal to 0
  for (size_t i = 0; i < nb_bits_addition; i++) {
    bootsMUX(&reponce2[i],&totalCharges[0],&reponce2[i],&tmp[i],cloudKey);
  }

  //-----------------------------------clean------------------------------------
  delete_gate_bootstrapping_ciphertext_array(nb_bits_addition, tmp);
}

//resolution of the 3rd question, if the totalCost is greater than 10000, we
//add 1 to the final answer
void question3(LweSample* reponce3,const LweSample* ciphertext2,const LweSample* ciphertext3,const LweSample* totalCost,const int nb_bits_addition,const TFheGateBootstrappingCloudKeySet* cloudKey){
  LweSample* tmp1=new_gate_bootstrapping_ciphertext(cloudKey->params);
  LweSample* tmp2=new_gate_bootstrapping_ciphertext_array(nb_bits_addition,cloudKey->params);

  //we look if the ccscode is greater than 10000
  isGreater(tmp1,totalCost,ciphertext2,32,cloudKey);
  //we pre-compute current solution plus 1
  rippleCarryAdder(tmp2,reponce3,ciphertext3,32,cloudKey);
  //we change the solution to the sum we've just computed iff the totalCost was
  //greater than 10000
  for (size_t i = 0; i < 32; i++) {
    bootsMUX(&reponce3[i], tmp1, &tmp2[i], &reponce3[i],cloudKey);
  }

  //-----------------------------------clean------------------------------------
  delete_gate_bootstrapping_ciphertext_array(nb_bits_addition, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
}

//a version of question3 that uses mutex in order to be usable in a thread while
//still updating the value of reponce4
void question3mutex(LweSample* reponce4,const LweSample* ciphertext2,const LweSample* ciphertext3,const int nb_bits_addition,const long int offsetGetCost,const long int lengthLine,const int nb_bits_totalCost,long int nb_lines,uint16_t nb_thread,const TFheGateBootstrappingCloudKeySet* cloudKey,const uint16_t num_thread,int* cmpt){
  long int offset,linesToRead;
  FILE* cloud_data;

  LweSample* tmp1=new_gate_bootstrapping_ciphertext(cloudKey->params);
  LweSample* tmp2=new_gate_bootstrapping_ciphertext_array(nb_bits_addition,cloudKey->params);
  LweSample* totalCost=new_gate_bootstrapping_ciphertext_array(nb_bits_totalCost,cloudKey->params);

  //each thread will open the cloud.data file in order to have his own position
  //indicator
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
    //note that I have prefered using multiple fseek in order to read less as
    //the size of the data we skip each iteration is quite large, and the use of
    //a buffer don't seem practicle to me as we have to use the
    //import_gate_bootstrapping_ciphertext_fromFile function.
    fseek ( cloud_data , offsetGetCost , SEEK_CUR	);
    for (int i=0; i<nb_bits_totalCost; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalCost[i], cloudKey->params);

    //the same way we did in question3, we look if totalcost is greater than
    //10000
    isGreater(tmp1,totalCost,ciphertext2,32,cloudKey);
    //we have to take the lock here as reponce4 can be updated by other threads
    mtx.lock();
    rippleCarryAdder(tmp2,reponce4,ciphertext3,32,cloudKey);
    for (size_t i = 0; i < 32; i++) {
      bootsMUX(&reponce4[i], tmp1, &tmp2[i], &reponce4[i],cloudKey);
    }
    *cmpt=*cmpt+1;
    if (*cmpt*100/nb_lines<10) {
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\bquestion 4:  %li%%",(*cmpt*100/nb_lines));
    }
    else if (*cmpt*100/nb_lines<100) {
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\bquestion 4:  %li%%",(*cmpt*100/nb_lines));
    }
    else
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bquestion 4:DONE");
    fflush(stdout);
    mtx.unlock();
  }

  //-----------------------------------clean------------------------------------
  fclose(cloud_data);
  delete_gate_bootstrapping_ciphertext_array(nb_bits_totalCost,totalCost);
  delete_gate_bootstrapping_ciphertext_array(nb_bits_addition, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
}

//resolution of the 4th question, if the totalCost is greater than 10000, we
//add 1 to the final answer while using threads
void question4(LweSample* reponce4,const LweSample* ciphertext2,const LweSample* ciphertext3,const int nb_bits_addition,const int nb_bits_ccsCode,const int nb_bits_totalCharges,const int nb_bits_totalCost,long int nb_lines,uint16_t nb_thread,const TFheGateBootstrappingCloudKeySet* cloudKey){
  long int x, offsetGetCost,lengthLine;
  int* cmpt;
  FILE* cloud_data;
  thread* tarray = new thread[nb_thread - 1];
  LweSample* tmp1=new_gate_bootstrapping_ciphertext(cloudKey->params);
  LweSample* tmp2=new_gate_bootstrapping_ciphertext_array(nb_bits_addition,cloudKey->params);
  LweSample* totalCost=new_gate_bootstrapping_ciphertext_array(nb_bits_totalCost,cloudKey->params);
  cmpt=(int *)malloc(sizeof(int));
  *cmpt=0;

  //we get the length of ccscode+totalcharges as well as the length of a line
  if (!(cloud_data = fopen("cloud.data","rb"))) {
    throw "Could not open the cloud.data file";
  }
  x=ftell(cloud_data);
  //for the ccscode+totalcharges
  for (size_t i = 0; i < nb_bits_ccsCode+nb_bits_totalCharges; i++) {
    import_gate_bootstrapping_ciphertext_fromFile(cloud_data, tmp1, cloudKey->params);
  }
  //offsetGetCost is the offset we will use for each lines to get the totalcost
  offsetGetCost=ftell(cloud_data)-x;
  //for the line
  for (size_t i = 0; i <nb_bits_totalCost; i++) {
    import_gate_bootstrapping_ciphertext_fromFile(cloud_data, tmp1, cloudKey->params);
  }
  //lengthLine is the offset we will use to skip lines at the start of each
  //threads in order to get to the subset allocated to the thread
  lengthLine=ftell(cloud_data)-x;
  fseek(cloud_data,0,SEEK_SET);

  //we launch the N-1 threads
  for (size_t i = 0; i < nb_thread - 1; i++) {
    tarray[i] = thread(question3mutex,reponce4,ciphertext2,ciphertext3,nb_bits_addition,offsetGetCost,lengthLine,nb_bits_totalCost,nb_lines,nb_thread,cloudKey,i,cmpt);
  }

  //the main thread will work on the last set of lines, it is basicaly the same
  //code as in question3mutex
  long int offset= ( (nb_thread - 1)*((nb_lines/nb_thread)+(nb_lines%nb_thread)))*(lengthLine);
  long int linesToRead=nb_lines/nb_thread;
  printf("main thread: %li lines to read starting line %li \n", linesToRead, offset/lengthLine);
  //we get to the begining of our subset
  fseek ( cloud_data , offset , SEEK_SET );

  //we do our computation for each lines of our subset
  for (size_t i = 0; i < linesToRead; i++) {

    fseek ( cloud_data , offsetGetCost , SEEK_CUR	);
    for (int i=0; i<nb_bits_totalCost; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &totalCost[i], cloudKey->params);
    isGreater(tmp1,totalCost,ciphertext2,32,cloudKey);

    mtx.lock();
    rippleCarryAdder(tmp2,reponce4,ciphertext3,32,cloudKey);
    for (size_t i = 0; i < 32; i++) {
      bootsMUX(&reponce4[i], tmp1, &tmp2[i], &reponce4[i],cloudKey);
    }
    *cmpt=*cmpt+1;
    if (*cmpt*100/nb_lines<10) {
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\bquestion 4:  %li%%",(*cmpt*100/nb_lines));
    }
    else if (*cmpt*100/nb_lines<100) {
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\bquestion 4:  %li%%",(*cmpt*100/nb_lines));
    }
    else
      printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bquestion 4:  DONE");
    fflush(stdout);
    mtx.unlock();
  }

  for (size_t i = 0; i < nb_thread - 1; i++) {
    tarray[i].join();
  }

  //-----------------------------------clean------------------------------------
  delete [] tarray;
  delete_gate_bootstrapping_ciphertext_array(nb_bits_totalCost,totalCost);
  delete_gate_bootstrapping_ciphertext_array(nb_bits_addition, tmp2);
  delete_gate_bootstrapping_ciphertext(tmp1);
}
