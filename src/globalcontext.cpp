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
  time_t rawtime;
  time(&rawtime);
  struct tm * timedata = localtime(&rawtime);
  currentyear = timedata->tm_year + 1900;
  sem_init(&list_refresh, 0, 0);
}

void GlobalContext::linkComponents(DataFileHandler * dfh, Engine * e, UICommunicator * uic, SiteManager * sm, SiteThreadManager * stm, TransferManager * tm, TickPoke * tp) {
  this->dfh = dfh;
  this->e = e;
  this->uic = uic;
  this->sm = sm;
  this->stm = stm;
  this->tm = tm;
  this->tp = tp;
}

SSL_CTX * GlobalContext::getSSLCTX() {
  return ssl_ctx;
}

DataFileHandler * GlobalContext::getDataFileHandler() {
  return dfh;
}

UICommunicator * GlobalContext::getUICommunicator() {
  return uic;
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

TickPoke * GlobalContext::getTickPoke() {
  return tp;
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

int GlobalContext::currentYear() {
  return currentyear;
}

std::string GlobalContext::ctimeLog() {
  time_t rawtime;
  time(&rawtime);
  std::string readabletime = asctime(localtime(&rawtime));
  return readabletime.substr(11, 8);
}

std::string GlobalContext::getSVNRevision() {
  return svnrev;
}

std::string GlobalContext::getCompileTime() {
  return compiletime;
}

int GlobalContext::str2Int(std::string str) {
  int num;
  std::istringstream ss(str);
  ss >> num;
  return num;
}

std::string GlobalContext::int2Str(int i) {
  std::stringstream out;
  out << i;
  return out.str();
}

void GlobalContext::signal_catch() {
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  signal(SIGWINCH, &sighandler_winch);
}

void GlobalContext::signal_ignore() {
  signal(SIGABRT, &sighandler_ignore);
  signal(SIGTERM, &sighandler_ignore);
  signal(SIGINT, &sighandler_ignore);
  signal(SIGWINCH, &sighandler_ignore);
}
