#include "sslmanager.h"

#include <pthread.h>
#include <vector>
#include <openssl/ec.h>

#include "lock.h"
#include "util.h"

#define ELLIPTIC_CURVE NID_secp521r1

#ifndef COUNTRY
#define COUNTRY "AQ"
#endif

#ifndef CORP
#define CORP "localhost"
#endif

#ifndef FQDN
#define FQDN "localhost"
#endif

namespace {


static SSL_CTX * ssl_ctx;
static bool initialized = false;
static X509 * x509 = NULL;
static EVP_PKEY * pkey = NULL;

#if OPENSSL_VERSION_NUMBER < 0x10100000
static std::vector<Lock> ssllocks;
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
#endif

}

void SSLManager::init() {
  coreutil::assert(!initialized);
  initialized = true;
#if OPENSSL_VERSION_NUMBER < 0x10100000
  ssllocks.resize(CRYPTO_num_locks());
  CRYPTO_set_locking_callback(sslLockingCallback);
  CRYPTO_set_id_callback(sslThreadIdCallback);
  SSL_library_init();
  SSL_load_error_strings();
  ssl_ctx = SSL_CTX_new(SSLv23_method());
#else
  ssl_ctx = SSL_CTX_new(TLS_method());
#endif
  SSL_CTX_set_cipher_list(ssl_ctx, "DEFAULT:!SEED");
#if OPENSSL_VERSION_NUMBER < 0x10200000
    EC_KEY* eckey = EC_KEY_new_by_curve_name(ELLIPTIC_CURVE);
    SSL_CTX_set_tmp_ecdh(ssl_ctx, eckey);
    EC_KEY_free(eckey);
#else
    SSL_CTX_set_ecdh_auto(ctx, 1);
#endif
}

void SSLManager::checkCertificateReady() {
  coreutil::assert(initialized);
  if (x509 == NULL || pkey == NULL) {
    pkey = createKey();
    x509 = createCertificate(pkey);
    registerKeyAndCertificate();
  }
}

EVP_PKEY * SSLManager::createKey() {
  coreutil::assert(initialized);
  EC_KEY * eckey = EC_KEY_new_by_curve_name(ELLIPTIC_CURVE);
  EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);
  EC_KEY_generate_key(eckey);
  EVP_PKEY * pkey = EVP_PKEY_new();
  EVP_PKEY_assign_EC_KEY(pkey, eckey);
  return pkey;
}

X509 * SSLManager::createCertificate(EVP_PKEY * pkey) {
  coreutil::assert(initialized);
  X509 * x509 = X509_new();
  ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);
  X509_NAME * name = X509_get_subject_name(x509);
  X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)COUNTRY, -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)CORP, -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)FQDN, -1, -1, 0);
  X509_set_issuer_name(x509, name);
  X509_set_pubkey(x509, pkey);
  X509_sign(x509, pkey, EVP_sha256());
  return x509;
}

SSL_CTX * SSLManager::getSSLCTX() {
  coreutil::assert(initialized);
  return ssl_ctx;
}

bool SSLManager::hasPrivateKey() {
  return pkey;
}

bool SSLManager::hasCertificate() {
  return x509;
}

BinaryData SSLManager::privateKey() {
  coreutil::assert(initialized);
  if (pkey == NULL) {
    return BinaryData();
  }
  EC_KEY * eckey = EVP_PKEY_get1_EC_KEY(pkey);
  BinaryData data;
  int bufsize = i2d_ECPrivateKey(eckey, NULL);
  data.resize(bufsize);
  unsigned char * buf = &data[0];
  i2d_ECPrivateKey(eckey, &buf);
  EC_KEY_free(eckey);
  return data;
}

BinaryData SSLManager::certificate() {
  coreutil::assert(initialized);
  if (x509 == NULL) {
    return BinaryData();
  }
  BinaryData data;
  int bufsize = i2d_X509(x509, NULL);
  data.resize(bufsize);
  unsigned char * buf = &data[0];
  i2d_X509(x509, &buf);
  return data;
}

void SSLManager::setPrivateKey(const BinaryData & key) {
  coreutil::assert(initialized);
  const unsigned char * bufstart = &key[0];
  EVP_PKEY* pkey = d2i_AutoPrivateKey(NULL, &bufstart, key.size());
  if (!pkey) {
      BIO* bio = BIO_new(BIO_s_mem());
      BIO_write(bio, &key[0], key.size());
      pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
      BIO_free(bio);
  }
  if (::pkey != NULL) {
    EVP_PKEY_free(::pkey);
  }
  ::pkey = pkey;
  registerKeyAndCertificate();
}

void SSLManager::setCertificate(const BinaryData & certificate) {
  coreutil::assert(initialized);
  const unsigned char * bufstart = &certificate[0];
  d2i_X509(&x509, &bufstart, certificate.size());
  if (!x509) {
      BIO* bio = BIO_new(BIO_s_mem());
      BIO_write(bio, &certificate[0], certificate.size());
      x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
      BIO_free(bio);
  }
  registerKeyAndCertificate();
}

void SSLManager::registerKeyAndCertificate() {
  coreutil::assert(initialized);
  if (x509 != NULL && pkey != NULL) {
    SSL_CTX_use_certificate(ssl_ctx, x509);
    SSL_CTX_use_PrivateKey(ssl_ctx, pkey);
  }
}

const char * SSLManager::getCipher(SSL * ssl) {
  return SSL_CIPHER_get_name(SSL_get_current_cipher(ssl));
}
