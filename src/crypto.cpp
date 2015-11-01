#include "crypto.h"

#include <limits.h>
#include <cstring>
#include <openssl/sha.h>
#include <time.h>

const EVP_CIPHER * Crypto::cipher() {
  return EVP_aes_256_cbc();
}

int Crypto::blocksize() {
  return EVP_CIPHER_block_size(cipher());
}

void Crypto::encrypt(unsigned char * indata, int inlen, unsigned char * key, unsigned char * outdata, int * outlen) {
  EVP_CIPHER_CTX ctx;
  EVP_CIPHER_CTX_init(&ctx);
  const EVP_CIPHER * cipherp = cipher();
  int ivlen = EVP_CIPHER_iv_length(cipherp);
  int sizelen;
  int resultlen;
  int finallen;
  srand(time(NULL));
  for (int i = 0; i < ivlen; i++) {
    outdata[i] = (unsigned char)(rand() % UCHAR_MAX);
  }
  EVP_EncryptInit_ex(&ctx, cipherp, NULL, key, outdata);
  EVP_EncryptUpdate(&ctx, &outdata[ivlen], &sizelen, (const unsigned char *)&inlen, sizeof(inlen));
  EVP_EncryptUpdate(&ctx, &outdata[ivlen+sizelen], &resultlen, indata, inlen);
  EVP_EncryptFinal_ex(&ctx, &outdata[ivlen+sizelen+resultlen], &finallen);
  *outlen = ivlen + sizelen + resultlen + finallen;
  EVP_CIPHER_CTX_cleanup(&ctx);
}

void Crypto::decrypt(unsigned char * indata, int inlen, unsigned char * key, unsigned char * outdata, int * outlen) {
  EVP_CIPHER_CTX ctx;
  EVP_CIPHER_CTX_init(&ctx);
  const EVP_CIPHER * cipherp = cipher();
  int ivlen = EVP_CIPHER_iv_length(cipherp);
  int writelen;
  int finalwritelen;
  EVP_DecryptInit_ex(&ctx, cipherp, NULL, key, indata);
  EVP_DecryptUpdate(&ctx, outdata, &writelen, indata + ivlen, inlen - ivlen);
  EVP_DecryptFinal_ex(&ctx, &outdata[writelen], &finalwritelen);
  *outlen = *((int *)outdata);
  if (writelen + finalwritelen < *outlen || *outlen < 0) {
    *outlen = writelen + finalwritelen;
  }
  memmove(outdata, outdata + sizeof(int), *outlen);
  EVP_CIPHER_CTX_cleanup(&ctx);
}

void Crypto::sha256(const std::string & indata, unsigned char * outdata) {
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, indata.c_str(), indata.length());
  SHA256_Final(outdata, &ctx);
}
