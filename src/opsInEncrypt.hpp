#ifndef OPSINENC
#define OPSINENC

void isEqual(LweSample* result,const LweSample* a,const LweSample* b,const int nb_bits,const TFheGateBootstrappingCloudKeySet* cloudKey);
void isGreater(LweSample* result,const LweSample* a,const LweSample* b,const int nb_bits,const TFheGateBootstrappingCloudKeySet* cloudKey);
void rippleCarryAdder(LweSample* result,const LweSample* a,const LweSample* b,const int nb_bits,const TFheGateBootstrappingCloudKeySet* cloudKey);

#endif
