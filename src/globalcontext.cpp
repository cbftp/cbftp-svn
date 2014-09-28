#include "globalcontext.h"

#include <signal.h>
#include <sys/timeb.h>
#include <sstream>

static pthread_mutex_t * ssllocks;

static void sslLockingCallback(int mode, int n, const char *, int) {
  if (mode & CRYPTO_LOCK) {
    pthread_mutex_lock(&ssllocks[n]);
  }
  else {
    pthread_mutex_unlock(&ssllocks[n]);
  }
}

static unsigned long sslThreadIdCallback() {
  return pthread_self();
}

GlobalContext::GlobalContext() {
  init();
}

void GlobalContext::init() {
  SSL_library_init();
  SSL_load_error_strings();
  ssl_ctx = SSL_CTX_new(SSLv23_client_method());
  ssllocks = (pthread_mutex_t *) malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
  for (int i = 0; i < CRYPTO_num_locks(); i++) {
    pthread_mutex_init(&ssllocks[i], NULL);
  }
  CRYPTO_set_locking_callback(sslLockingCallback);
  CRYPTO_set_id_callback(sslThreadIdCallback);
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(pthread_self(), "clusterbomb");
#endif
  updateTime();
}

void GlobalContext::linkEventLog(EventLog * el) {
  this->el = el;
}

void GlobalContext::linkWorkManager(WorkManager * wm) {
  this->wm = wm;
}

void GlobalContext::linkTickPoke(TickPoke * tp) {
  this->tp = tp;
}

void GlobalContext::linkComponents(DataFileHandler * dfh, IOManager * iom,
    Engine * e, UIBase * uib, SiteManager * sm, SiteLogicManager * slm,
    TransferManager * tm, RemoteCommandHandler * rch,
    SkipList * sl, ProxyManager * pm, LocalStorage * ls,
    ExternalFileViewing * efv) {
  this->dfh = dfh;
  this->iom = iom;
  this->e = e;
  this->uib = uib;
  this->sm = sm;
  this->slm = slm;
  this->tm = tm;
  this->rch = rch;
  this->sl = sl;
  this->pm = pm;
  this->ls = ls;
  this->efv = efv;
}

SSL_CTX * GlobalContext::getSSLCTX() const {
  return ssl_ctx;
}

Engine * GlobalContext::getEngine() const {
  return e;
}

DataFileHandler * GlobalContext::getDataFileHandler() const {
  return dfh;
}

IOManager * GlobalContext::getIOManager() const {
  return iom;
}

WorkManager * GlobalContext::getWorkManager() const {
  return wm;
}

UIBase * GlobalContext::getUIBase() const {
  return uib;
}

SiteManager * GlobalContext::getSiteManager() const {
  return sm;
}

SiteLogicManager * GlobalContext::getSiteLogicManager() const {
  return slm;
}

TransferManager * GlobalContext::getTransferManager() const {
  return tm;
}

TickPoke * GlobalContext::getTickPoke() const {
  return tp;
}

RemoteCommandHandler * GlobalContext::getRemoteCommandHandler() const {
  return rch;
}

SkipList * GlobalContext::getSkipList() const {
  return sl;
}

EventLog * GlobalContext::getEventLog() const {
  return el;
}

ProxyManager * GlobalContext::getProxyManager() const {
  return pm;
}

LocalStorage * GlobalContext::getLocalStorage() const {
  return ls;
}

ExternalFileViewing * GlobalContext::getExternalFileViewing() const {
  return efv;
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

int GlobalContext::currentYear() const {
  return currentyear;
}

int GlobalContext::currentMonth() const {
  return currentmonth;
}

int GlobalContext::currentDay() const {
  return currentday;
}

std::string GlobalContext::ctimeLog() {
  time_t rawtime;
  time(&rawtime);
  std::string readabletime = asctime(localtime(&rawtime));
  return readabletime.substr(11, 8);
}

std::string GlobalContext::parseSize(unsigned long long int size) {
  int iprefix;
  for (iprefix = 0; iprefix < 6 && size / powers[iprefix] >= 1000; iprefix++);
  unsigned long long int currentpower = powers[iprefix];
  std::string result;
  int whole = size / currentpower;
  if (iprefix == 0) {
    result = int2Str(whole) + " B";
  }
  else {
    unsigned long long int decim = ((size % currentpower) * sizegranularity) / currentpower + 5;
    if (decim >= sizegranularity) {
      whole++;
      decim = 0;
    }
    std::string decimstr = int2Str(decim);
    while (decimstr.length() <= SIZEDECIMALS) {
      decimstr = "0" + decimstr;
    }
    result = int2Str(whole) + "." + decimstr.substr(0, SIZEDECIMALS) + " ";
    switch (iprefix) {
      case 1:
        result.append("kB");
        break;
      case 2:
        result.append("MB");
        break;
      case 3:
        result.append("GB");
        break;
      case 4:
        result.append("TB");
        break;
      case 5:
        result.append("PB");
        break;
      case 6:
        result.append("EB");
        break;
    }
  }
  return result;
}

std::string GlobalContext::getSVNRevision() const {
  return svnrev;
}

std::string GlobalContext::getCompileTime() const {
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

int GlobalContext::getSizeGranularity() {
  int gran = 1;
  for (int i = 0; i <= SIZEDECIMALS; i++) {
    gran *= 10;
  }
  return gran;
}

std::vector<unsigned long long int> GlobalContext::getPowers() {
  std::vector<unsigned long long int> vec;
  vec.reserve(7);
  unsigned long long int pow = 1;
  for (int i = 0; i < 7; i++) {
    vec.push_back(pow);
    pow *= 1024;
  }
  return vec;
}

unsigned int GlobalContext::sizegranularity = GlobalContext::getSizeGranularity();
std::vector<unsigned long long int> GlobalContext::powers = GlobalContext::getPowers();
