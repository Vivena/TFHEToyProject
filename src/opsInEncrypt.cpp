#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>

void compare_bit(LweSample* newDecision,LweSample* aGreater,const LweSample* a,const LweSample* b,const LweSample* decision,const TFheGateBootstrappingCloudKeySet* cloudKey) {
    LweSample* tmp=new_gate_bootstrapping_ciphertext(cloudKey->params);

    //are we at the first bit with a difference
    //if A=B and Decision true -> Decision+1 true
    bootsXNOR(tmp,a,b, cloudKey);
    bootsAND(newDecision, tmp, decision, cloudKey);
    bootsMUX(aGreater, tmp, aGreater, b, cloudKey);

    //-----------------------------------clean----------------------------------
    delete_gate_bootstrapping_ciphertext(tmp);
}

void compare(const LweSample* a,const LweSample* b,LweSample* decision,LweSample* aGreater,const int nb_bits,const TFheGateBootstrappingCloudKeySet* cloudKey){
    for (int i=0; i<nb_bits; i++) {
        compare_bit(decision,aGreater,&a[i],&b[i],decision,cloudKey);
    }
}

void isEqual(LweSample* result,const LweSample* a,const LweSample* b,const int nb_bits,const TFheGateBootstrappingCloudKeySet* cloudKey){
    LweSample* tmps = new_gate_bootstrapping_ciphertext(cloudKey->params);

    bootsCONSTANT(result,1, cloudKey);
    compare(a,b,result,tmps,nb_bits,cloudKey);

    delete_gate_bootstrapping_ciphertext(tmps);
}

void isGreater(LweSample* result,const LweSample* a,const LweSample* b,const int nb_bits,const TFheGateBootstrappingCloudKeySet* cloudKey){
    LweSample* tmps = new_gate_bootstrapping_ciphertext(cloudKey->params);
    bootsCONSTANT(tmps,0, cloudKey);
    compare(b,a,tmps,result,nb_bits,cloudKey);

    //-----------------------------------clean----------------------------------
    delete_gate_bootstrapping_ciphertext(tmps);
}

void halfAdder(LweSample* rez,LweSample* carry,const LweSample* a,const LweSample* b,const TFheGateBootstrappingCloudKeySet* cloudKey){
    bootsXOR(rez,a,b, cloudKey);
    bootsAND(carry,a,b, cloudKey);
}

void fullAdder(LweSample* rez,LweSample* carry,const LweSample* a,const LweSample* b,const LweSample* c,const TFheGateBootstrappingCloudKeySet* cloudKey){
    LweSample* tmps = new_gate_bootstrapping_ciphertext_array(2, cloudKey->params);
    halfAdder(&tmps[0],&tmps[1],a,b,cloudKey);
    halfAdder(rez,&tmps[0],c,&tmps[0],cloudKey);
    bootsOR(carry,&tmps[0],&tmps[1], cloudKey);

    //-----------------------------------clean----------------------------------
    delete_gate_bootstrapping_ciphertext_array(2,tmps);
}

//we will use the ripple carry adder as it has a shallow circuit, the time lost
//by waiting for the carry is therefore compansated by the fact it uses fewer
//gates.
//a possible improvement might be using 2 threads for the halfadder (the time
//spent evalutating the gate might be greater than the time necessary to launch
//a thread).
void rippleCarryAdder(LweSample* rez, const LweSample* a,const LweSample* b,const int nb_bits,const TFheGateBootstrappingCloudKeySet* cloudKey){
    LweSample* carry = new_gate_bootstrapping_ciphertext(cloudKey->params);
    bootsCONSTANT(carry,0, cloudKey);
    for (int i = 0; i < nb_bits; i++) {
        bootsCOPY(&rez[i], &a[i], cloudKey);
    }
    for (int i = 0; i < nb_bits; i++) {
        fullAdder(&rez[i],carry,&rez[i],&b[i],carry,cloudKey);
    }

    //-----------------------------------clean----------------------------------
    delete_gate_bootstrapping_ciphertext(carry);
}
