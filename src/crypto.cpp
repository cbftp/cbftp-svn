#include "crypto.h"

#include <limits.h>
#include <cstring>
#include <cstdlib>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <time.h>

namespace {

const EVP_CIPHER * cipher() {
  return EVP_aes_256_cbc();
}

int blockSize() {
  return EVP_CIPHER_block_size(cipher());
}

}

void Crypto::encrypt(const BinaryData & indata, const BinaryData & key, BinaryData & outdata) {
  EVP_CIPHER_CTX ctx;
  EVP_CIPHER_CTX_init(&ctx);
  const EVP_CIPHER * cipherp = cipher();
  int ivlen = EVP_CIPHER_iv_length(cipherp);
  int inlen = indata.size();
  outdata.resize(indata.size() + blockSize() + blockSize());
  int sizelen;
  int resultlen;
  int finallen;
  srand(time(NULL));
  for (int i = 0; i < ivlen; i++) {
    outdata[i] = (unsigned char)(rand() % UCHAR_MAX);
  }
  EVP_EncryptInit_ex(&ctx, cipherp, NULL, &key[0], &outdata[0]);
  EVP_EncryptUpdate(&ctx, &outdata[ivlen], &sizelen, (const unsigned char *)&inlen, sizeof(inlen));
  EVP_EncryptUpdate(&ctx, &outdata[ivlen+sizelen], &resultlen, &indata[0], indata.size());
  EVP_EncryptFinal_ex(&ctx, &outdata[ivlen+sizelen+resultlen], &finallen);
  outdata.resize(ivlen + sizelen + resultlen + finallen);
  EVP_CIPHER_CTX_cleanup(&ctx);
}

void Crypto::decrypt(const BinaryData & indata, const BinaryData & key, BinaryData & outdata) {
  EVP_CIPHER_CTX ctx;
  EVP_CIPHER_CTX_init(&ctx);
  const EVP_CIPHER * cipherp = cipher();
  int ivlen = EVP_CIPHER_iv_length(cipherp);
  outdata.resize(indata.size() + blockSize());
  int writelen;
  int finalwritelen;
  EVP_DecryptInit_ex(&ctx, cipherp, NULL, &key[0], &indata[0]);
  EVP_DecryptUpdate(&ctx, &outdata[0], &writelen, &indata[ivlen], indata.size() - ivlen);
  EVP_DecryptFinal_ex(&ctx, &outdata[writelen], &finalwritelen);
  int outlen = *((int *)&outdata[0]);
  if (writelen + finalwritelen < outlen || outlen < 0) {
    outlen = writelen + finalwritelen;
  }
  memmove(&outdata[0], &outdata[sizeof(int)], outlen);
  outdata.resize(outlen);
  EVP_CIPHER_CTX_cleanup(&ctx);
}

void Crypto::sha256(const BinaryData & indata, BinaryData & outdata) {
  outdata.resize(SHA256_DIGEST_LENGTH);
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, &indata[0], indata.size());
  SHA256_Final(&outdata[0], &ctx);
}
