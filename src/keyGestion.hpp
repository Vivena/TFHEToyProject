#ifndef KEY_USAGE
#define KEY_USAGE

void keyGen();
void getSecretKey(TFheGateBootstrappingSecretKeySet** key);
void getCloudKey(TFheGateBootstrappingCloudKeySet** key);

#endif
