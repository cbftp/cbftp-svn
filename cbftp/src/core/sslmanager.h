#pragma once

#include <openssl/ssl.h>
#include <string>

#include "types.h"

class SSLManager {
public:
  static void init();
  static SSL_CTX * getSSLCTX();
  static const char * getCipher(SSL *);
  static void checkCertificateReady();
  static X509 * createCertificate(EVP_PKEY *);
  static bool hasPrivateKey();
  static bool hasCertificate();
  static BinaryData privateKey();
  static BinaryData certificate();
  static void setPrivateKey(const BinaryData &);
  static void setCertificate(const BinaryData &);
  static void registerKeyAndCertificate(EVP_PKEY *, X509 *);
  static std::string version();
private:
  static void registerKeyAndCertificate();
  static EVP_PKEY * createKey();
};
