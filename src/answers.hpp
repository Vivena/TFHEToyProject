#ifndef ANSWERS
#define ANSWERS

void question1(LweSample* reponce1,const LweSample* ciphertext1,const LweSample* ccscode,const LweSample* totalcharges,const int nb_bits_ccsCode,const int nb_bits_totalCharges,const TFheGateBootstrappingCloudKeySet* cloudKey);
void question2(LweSample* reponce2,const LweSample* ciphertext3,const LweSample* totalCharges,const int nb_bits_addition,const TFheGateBootstrappingCloudKeySet* cloudKey);
void question3(LweSample* reponce3,const LweSample* ciphertext2,const LweSample* ciphertext3,const LweSample* totalcost,const int nb_bits_addition,const TFheGateBootstrappingCloudKeySet* cloudKey);
void question4(LweSample* reponce4,const LweSample* ciphertext2,const LweSample* ciphertext3,const int nb_bits_addition,const int nb_bits_ccsCode,const int nb_bits_totalCharges,const int nb_bits_totalCost,long int nb_lines,uint16_t nb_thread,const TFheGateBootstrappingCloudKeySet* cloudKey);

#endif
