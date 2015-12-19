#pragma once

#include <openssl/ssl.h>

namespace SSLManager {
  void init();
  SSL_CTX * getSSLCTX();
  const char * getCipher(SSL *);
}
