#include "globalcontext.h"

GlobalContext::GlobalContext() {
  init();
}

void GlobalContext::init() {
  SSL_library_init();
  SSL_load_error_strings();
  ssl_ctx = SSL_CTX_new(TLSv1_client_method());
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  sem_init(&list_refresh, 0, 0);
}

void GlobalContext::linkManagers(UserInterface * ui, SiteManager * sm, SiteThreadManager * stm, TransferManager * tm) {
  this->ui = ui;
  this->sm = sm;
  this->stm = stm;
  this->tm = tm;
}

void GlobalContext::linkEngine(Engine * e) {
  this->e = e;
}

SSL_CTX * GlobalContext::getSSLCTX() {
  return ssl_ctx;
}

UserInterface * GlobalContext::getUI() {
  return ui;
}

SiteManager * GlobalContext::getSiteManager() {
  return sm;
}

SiteThreadManager * GlobalContext::getSiteThreadManager() {
  return stm;
}

TransferManager * GlobalContext::getTransferManager() {
  return tm;
}

pthread_attr_t * GlobalContext::getPthreadAttr() {
  return &attr;
}

sem_t * GlobalContext::getListRefreshSem() {
  return &list_refresh;
}

int GlobalContext::ctimeMSec() {
  timeb tb;
  ftime(&tb);
  int count = tb.millitm + tb.time * 1000;
  return count;
}

