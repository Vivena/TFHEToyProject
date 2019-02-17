#ifndef UTIL
#define UTIL

int encryptFile(FILE* cloud_data,TFheGateBootstrappingSecretKeySet* key);
void printLWESample(const LweSample* answer,const uint16_t nb_bits,const TFheGateBootstrappingSecretKeySet* key);

#endif
