#include "crypto.h"

#include <climits>
#include <ctime>
#include <openssl/sha.h>
#include <openssl/evp.h>

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
  outdata.resize(ivlen + indata.size() + blockSize());
  int resultlen;
  int finallen;
  srand(time(NULL));
  for (int i = 0; i < ivlen; i++) {
    outdata[i] = (unsigned char)(rand() % UCHAR_MAX);
  }
  EVP_EncryptInit_ex(&ctx, cipherp, NULL, &key[0], &outdata[0]);
  EVP_EncryptUpdate(&ctx, &outdata[ivlen], &resultlen, &indata[0], indata.size());
  EVP_EncryptFinal_ex(&ctx, &outdata[ivlen+resultlen], &finallen);
  outdata.resize(ivlen + resultlen + finallen);
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
  outdata.resize(writelen + finalwritelen);
  EVP_CIPHER_CTX_cleanup(&ctx);
}

void Crypto::sha256(const BinaryData & indata, BinaryData & outdata) {
  outdata.resize(SHA256_DIGEST_LENGTH);
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, &indata[0], indata.size());
  SHA256_Final(&outdata[0], &ctx);
}
