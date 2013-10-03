#include "globalcontext.h"

#include <signal.h>
#include <sys/timeb.h>
#include <sstream>

GlobalContext::GlobalContext() {
  init();
}

void GlobalContext::init() {
  SSL_library_init();
  SSL_load_error_strings();
  ssl_ctx = SSL_CTX_new(TLSv1_client_method());
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(pthread_self(), "clusterbomb");
#endif
  pthread_mutex_init(&time_mutex, NULL);
  updateTime();
}

void GlobalContext::linkWorkManager(WorkManager * wm) {
  this->wm = wm;
}

void GlobalContext::linkComponents(DataFileHandler * dfh, IOManager * iom,
    Engine * e, UICommunicator * uic, SiteManager * sm, SiteLogicManager * slm,
    TransferManager * tm, TickPoke * tp, RemoteCommandHandler * rch,
    SkipList * sl) {
  this->dfh = dfh;
  this->iom = iom;
  this->e = e;
  this->uic = uic;
  this->sm = sm;
  this->slm = slm;
  this->tm = tm;
  this->tp = tp;
  this->rch = rch;
  this->sl = sl;
}

SSL_CTX * GlobalContext::getSSLCTX() {
  return ssl_ctx;
}

Engine * GlobalContext::getEngine() {
  return e;
}

DataFileHandler * GlobalContext::getDataFileHandler() {
  return dfh;
}

IOManager * GlobalContext::getIOManager() {
  return iom;
}

WorkManager * GlobalContext::getWorkManager() {
  return wm;
}

UICommunicator * GlobalContext::getUICommunicator() {
  return uic;
}

SiteManager * GlobalContext::getSiteManager() {
  return sm;
}

SiteLogicManager * GlobalContext::getSiteLogicManager() {
  return slm;
}

TransferManager * GlobalContext::getTransferManager() {
  return tm;
}

TickPoke * GlobalContext::getTickPoke() {
  return tp;
}

RemoteCommandHandler * GlobalContext::getRemoteCommandHandler() {
  return rch;
}

SkipList * GlobalContext::getSkipList() {
  return sl;
}

pthread_attr_t * GlobalContext::getPthreadAttr() {
  return &attr;
}

int GlobalContext::ctimeMSec() {
  timeb tb;
  ftime(&tb);
  int count = tb.millitm + tb.time * 1000;
  return count;
}

void GlobalContext::updateTime() {
  time_t rawtime;
  time(&rawtime);
  struct tm * timedata = localtime(&rawtime);
  currentyear = timedata->tm_year + 1900;
  currentmonth = timedata->tm_mon + 1;
  currentday = timedata->tm_mday;
}

int GlobalContext::currentYear() {
  return currentyear;
}

int GlobalContext::currentMonth() {
  return currentmonth;
}

int GlobalContext::currentDay() {
  return currentday;
}

std::string GlobalContext::ctimeLog() {
  pthread_mutex_lock(&time_mutex);
  time_t rawtime;
  time(&rawtime);
  std::string readabletime = asctime(localtime(&rawtime));
  pthread_mutex_unlock(&time_mutex);
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

std::string GlobalContext::int2Str(unsigned int i) {
  return int2Str((int)i);
}

std::string GlobalContext::int2Str(int i) {
  std::stringstream out;
  out << i;
  return out.str();
}

std::string GlobalContext::int2Str(unsigned long long int i) {
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

std::string & GlobalContext::debugString(const char * s) {
    return *(new std::string(s));
}
