#include "sslmanager.h"

#include <pthread.h>
#include <vector>

#include "lock.h"

static std::vector<Lock> ssllocks;
static SSL_CTX * ssl_ctx;

static void sslLockingCallback(int mode, int n, const char *, int) {
  if (mode & CRYPTO_LOCK) {
    ssllocks[n].lock();
  }
  else {
    ssllocks[n].unlock();
  }
}

static unsigned long sslThreadIdCallback() {
  return (unsigned long) pthread_self();
}

namespace SSLManager {

void init() {
  ssllocks.resize(CRYPTO_num_locks());
  CRYPTO_set_locking_callback(sslLockingCallback);
  CRYPTO_set_id_callback(sslThreadIdCallback);
  SSL_library_init();
  SSL_load_error_strings();
  ssl_ctx = SSL_CTX_new(SSLv23_client_method());
  SSL_CTX_set_cipher_list(ssl_ctx, "DEFAULT:!SEED");
}

SSL_CTX * getSSLCTX() {
  return ssl_ctx;
}

const char * getCipher(SSL * ssl) {
  return SSL_CIPHER_get_name(SSL_get_current_cipher(ssl));
}

}
