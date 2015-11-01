#pragma once

#include <openssl/evp.h>
#include <string>

class Crypto {
private:
  static const EVP_CIPHER * cipher();
public:
  static int blocksize();
  static void encrypt(unsigned char *, int, unsigned char *, unsigned char *, int *);
  static void decrypt(unsigned char *, int, unsigned char *, unsigned char *, int *);
  static void sha256(const std::string &, unsigned char *);
};
