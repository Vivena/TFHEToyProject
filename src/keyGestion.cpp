#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>

//generate the secret key and the cloud key
void keyGen() {
  FILE* secret_key;
  FILE* cloud_key;
  const int minimum_lambda = 110;
  uint32_t seed[] = { 314, 1592, 657 };

  TFheGateBootstrappingParameterSet* params = new_default_gate_bootstrapping_parameters(minimum_lambda);

  //generate a random key
  tfhe_random_generator_setSeed(seed,3);
  TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(params);

  //export the secret key to file for later use
  if (!(secret_key = fopen("secret.key","wb"))) {
    throw "Could not open tne secret.key file";
  }
  export_tfheGateBootstrappingSecretKeySet_toFile(secret_key, key);
  fclose(secret_key);

  //export the cloud key to a file (for the cloud)
  if (!(cloud_key = fopen("cloud.key","wb"))) {
    throw "Could not open tne cloud.key file";
  }
  export_tfheGateBootstrappingCloudKeySet_toFile(cloud_key, &key->cloud);
  fclose(cloud_key);

  //-----------------------------------clean------------------------------------
  delete_gate_bootstrapping_secret_keyset(key);
  delete_gate_bootstrapping_parameters(params);
}

//get the secret key from file
void getSecretKey(TFheGateBootstrappingSecretKeySet** key){
  FILE* secret_key;
  if (!(secret_key = fopen("secret.key","rb"))) {
    throw "Could not open tne secret.key file";
  }
  *key = new_tfheGateBootstrappingSecretKeySet_fromFile(secret_key);
  fclose(secret_key);
}

//get the cloud key from file
void getCloudKey(TFheGateBootstrappingCloudKeySet** key){
  FILE* cloud_key;
  if (!(cloud_key = fopen("cloud.key","rb"))) {
    throw "Could not open tne secret.key file";
  }
  *key = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
  fclose(cloud_key);
}
