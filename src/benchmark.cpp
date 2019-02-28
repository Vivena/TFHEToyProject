#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <chrono>
#include "opsInEncrypt.hpp"

//(as rippleCarryAdder take a few seconds each times on my computer, 1000
//mesures are precise enougth)
#define NUM_ITER 10000


//benchmark compute the average time it take for rippleCarryAdder to add 1 to a
//LweSample.
void benchmark(const LweSample* ciphertext3,const TFheGateBootstrappingCloudKeySet* cloudKey,const TFheGateBootstrappingSecretKeySet* key){
  int32_t cmp=0,res=0,int_answer;
  int64_t elapsed_seconds=0;
  std::chrono::time_point<std::chrono::system_clock> start, end;
  LweSample* tmp=new_gate_bootstrapping_ciphertext_array(32,cloudKey->params);
  for (size_t i = 0; i < 32; i++) {
    bootsCONSTANT(&tmp[i],0, cloudKey);
  }

  //we set a higher priority for this process in order to get less interferences
  //TODO: we should also set the CPU affinity, but it is OS specific
  pid_t pid = getpid();
  setpriority(PRIO_PROCESS, pid, -20);

  printf("benchmarking:    0%%");
  fflush(stdout);
  //we run rippleCarryAdder 1000 times and compute the average time in
  //milliseconds in order to be more precise
  for (size_t i = 0; i < NUM_ITER; i++) {
    cmp++;
    //we start the chrono
    start = std::chrono::system_clock::now();
    //run rippleCarryAdder
    rippleCarryAdder(tmp,tmp,ciphertext3,32,cloudKey);
    //end the chrono
    end = std::chrono::system_clock::now();

    elapsed_seconds+= std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    int_answer = 0;
      for (int i=0; i<32; i++) {
          int ai = bootsSymDecrypt(&tmp[i], key);
          int_answer |= (ai<<i);
      }
    if (cmp!=int_answer) {
      res=cmp;
    }
    //display completion percentage
    if (cmp*100/NUM_ITER<10) {
      printf("\b\b%i%%",(cmp*100/NUM_ITER));
    }
    else if (cmp*100/NUM_ITER<100) {
      printf("\b\b\b%i%%",(cmp*100/NUM_ITER));
    }
    else
      printf("\b\b\b\bDONE\n");
    fflush(stdout);
  }
  printf("     %i operations done in %lli milliseconds for an average",cmp,elapsed_seconds);
  printf("of %lli milliseconds per operations. \n", elapsed_seconds/cmp);
  if(res!=0)printf("There was an error in the computation starting the %ith operation.\n",res);
  else printf("     There was no error in the computation.\n" );
  delete_gate_bootstrapping_ciphertext_array(32, tmp);
}
