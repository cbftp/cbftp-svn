#include "ftpthread.h"

FTPThread::FTPThread(int id, Site * site, FTPThreadCom * ftpthreadcom) {
  this->id = id;
  this->site = site;
  this->ftpthreadcom = ftpthreadcom;
  ready = false;
  controlssl = false;
  refreshloop = false;
  currentpath = "/";
  tv.tv_sec = 0;
  tv.tv_usec = 500000;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_UNSPEC;
  sock.ai_socktype = SOCK_STREAM;
  int status = getaddrinfo(site->getAddress().data(), site->getPort().data(), &sock, &res);
  FD_ZERO(&readfd);
  sem_init(&commandsem, 0, 0);
  sem_init(&transfersem, 0, 0);
  sem_init(&tick, 0, 0);
  pthread_mutex_init(&commandq_mutex, NULL);
  list_refresh = global->getListRefreshSem();
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_create(&tickthread, global->getPthreadAttr(), runTick, (void *) this);
}

void FTPThread::loginAsync() {
  putCommand(new CommandQueueElement(0));
}

bool FTPThread::loginT() {
  int status;
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  int loggedin = connect(sockfd, res->ai_addr, res->ai_addrlen);
  FD_SET(sockfd, &readfd);
  if (loggedin < 0) {
    ftpthreadcom->loginConnectFailed(id);
    return false;
  }
  if (read() != 220) {
    ftpthreadcom->loginUnknownResponse(id);
    return false;
  }
  write("AUTH TLS");
  if ((status = read()) == 234) {
    ssl = SSL_new(global->getSSLCTX());
    SSL_set_fd(ssl, sockfd);
    SSL_connect(ssl);
    const char * cipher = SSL_CIPHER_get_name(SSL_get_current_cipher(ssl));
    std::cout << "[Cipher: " << cipher << "]" << std::endl;
    controlssl = true;
  }
  else {
    ftpthreadcom->loginTLSFailed(id, status);
    return false;
  }
  doUSERPASST(false);
  return true;
}

void FTPThread::reconnectAsync() {
  putCommand(new CommandQueueElement(8), false);
}

void FTPThread::reconnectT() {
  controlssl = false;
  loginT();
}

void FTPThread::doUSERPASSAsync() {
  putCommand(new CommandQueueElement(4), false);
}

void FTPThread::doUSERPASST(bool killer) {
  int status;
  char * reply;
  write((std::string("USER ") + (killer ? "!" : "") + site->getUser()).data());
  if ((status = readall(&reply, true)) != 331) {
    if (!killer) ftpthreadcom->loginUserDenied(id, status, reply);
    else ftpthreadcom->loginKillFailed(id, status, reply);
    return;
  }
  delete reply;
  std::string pass = site->getPass();
  char * passc = new char[pass.length()+1];
  for (int i = 0; i < pass.length(); i++) passc[i] = '*';
  passc[pass.length()] = '\0';
  std::cout << "PASS " << passc << std::endl;
  delete[] passc;
  write((std::string("PASS ") + site->getPass()).data(), false);
  if ((status = readall(&reply, true)) != 230) {
    if (!killer) ftpthreadcom->loginPasswordDenied(id, status, reply);
    else ftpthreadcom->loginKillFailed(id, status, reply);
    return;
  }
  delete reply;
  ftpthreadcom->loginSuccessful(id);
}

void FTPThread::loginKillAsync() {
  putCommand(new CommandQueueElement(6));
}

void FTPThread::loginKillT() {
  doUSERPASST(true);
}

bool FTPThread::isReady() {
  return ready;
}

void FTPThread::setBusy() {
  ready = false;
}

void FTPThread::setReady() {
  putCommand(new CommandQueueElement(5));
}

int FTPThread::write(const char * command) {
  return FTPThread::write(command, true);
}

int FTPThread::write(const char * command, bool echo) {
  char out[strlen(command)+2];
  sprintf(out, "%s\n", command);
  int len = strlen(out);
  int b_sent = -1;
  if (controlssl) b_sent = SSL_write(ssl, out, len);
  else b_sent = send(sockfd, out, len, 0);
  if (b_sent == 0) {
    ftpthreadcom->connectionClosedUnexpectedly(id);
    return 0;
  }
  else if (b_sent >= len) {
    if (echo) std::cout << "[" << site->getName() << " " << id << "] " << out << std::flush;
    return 0;
  } else {
    write(out + b_sent);
  }
}

int FTPThread::read() {
  char buf[MAXDATASIZE+1];
  int b_recv = -1;
  if (controlssl) b_recv = SSL_read(ssl, buf, MAXDATASIZE);
  else b_recv = recv(sockfd, buf, MAXDATASIZE, 0);
  if (b_recv == 0) {
    ftpthreadcom->connectionClosedUnexpectedly(id);
    return 0;
  }
  buf[b_recv] = '\0';
  std::cout << "[" << site->getName() << " " << id << "] " << buf << std::flush;
  if (b_recv && buf[b_recv-1] == '\n') {
    char * loc = buf + b_recv - 5;
    while (loc >= buf) {
      if (*loc >= 48 && *loc <= 57 && *(loc+1) >= 48 && *(loc+1) <= 57 && *(loc+2) >= 48 && *(loc+2) <= 57) {
        if ((*(loc+3) == ' ' || *(loc+3) == '\n') && (loc == buf || *(loc-1) == '\n')) {
          char num[4];
          num[3] = '\0';
          strncpy(num, loc, 3);
          int ret = atoi(num);
          if (ret == 550) {
            // workaround for a glftpd bug causing an extra row '550 Unable to load your own user file!.' on retr/stor
            // distinguished to a separate error code, handled specially in retr/stor
            if (*(loc+4) == 'U' && *(loc+5) == 'n' && *(loc+28) == 'u' && *(loc+33) == 'f') return 559;
          }
          return ret;
        }
      }
      --loc;
    }
    return read();
  }
  else return read();
}

int FTPThread::readall(char ** reply, bool print) {
  char * tmp = (char *) malloc(sizeof(char));
  *tmp = '\0';
  return readallsub(reply, tmp, print);
}

int FTPThread::readallsub(char ** reply, char * tmp, bool print) {
  char buf[MAXDATASIZE+1];
  int b_recv = -1;
  if (controlssl) b_recv = SSL_read(ssl, buf, MAXDATASIZE);
  else b_recv = recv(sockfd, buf, MAXDATASIZE, 0);
  if (b_recv == 0) {
    ftpthreadcom->connectionClosedUnexpectedly(id);
    return 0;
  }
  buf[b_recv] = '\0';
  if (print) std::cout << "[" << site->getName() << " " << id << "] " << buf << std::flush;
  int tmplen = strlen(tmp);
  char * build = (char *) malloc(tmplen + b_recv + 1);
  strcpy(build, tmp);
  strcat (build, buf);
  delete tmp;
  if (b_recv && build[tmplen + b_recv - 1] == '\n') {
    char * loc = build + tmplen + b_recv - 5;
    while (loc >= build) {
      if (*loc >= 48 && *loc <= 57 && *(loc+1) >= 48 && *(loc+1) <= 57 && *(loc+2) >= 48 && *(loc+2) <= 57) {
        if ((*(loc+3) == ' ' || *(loc+3) == '\n') && (loc == build || *(loc-1) == '\n')) {
          *reply = build;
          char num[4];
          num[3] = '\0';
          strncpy(num, loc, 3);
          int ret = atoi(num);
          if (ret == 550) {
            // workaround for a glftpd bug causing an extra row '550 Unable to load your own user file!.' on retr/stor
            // distinguished to a separate error code, handled specially in retr/stor
            if (*(loc+4) == 'U' && *(loc+5) == 'n' && *(loc+28) == 'u' && *(loc+33) == 'f') return 559;
          }
          return ret;
        }
      }
      --loc;
    }
    return readallsub(reply, build, print);
  } else return readallsub(reply, build, print);
}

bool FTPThread::pendingread() {
  if (controlssl && SSL_pending(ssl) > 0) return true;
  else return (select(sockfd+1, &readfd, NULL, NULL, &tv) > 0);
}

void FTPThread::refreshLoopAsync(SiteRace * race) {
  putCommand(new CommandQueueElement(2, (void *) race));
}

void FTPThread::refreshLoopT() {
  refreshloop = true;
  if (currentpath != currentsiterace->getPath()) doCWDT(currentsiterace->getPath());
  int val;
  while(true) {
    updateFileList(currentsiterace->getFileList());
    sleepTickT();
    sem_getvalue(&commandsem, &val);
    if (val > 0) return;
  }
}

int FTPThread::updateFileList(FileList * filelist) {
  char * reply;
  write("STAT -l");
  if (readall(&reply, false) == 213) {
    char * loc = reply, * start, * tmp;
    int linelen;
    while(*++loc != '\n');
    while(*++loc != '\n');
    int files = 0;
    int touch = rand();
    while (*++loc != '2') {
      start = loc;
      while(*++loc != '\n');
      *loc = '\0';
      files++;
      filelist->updateFile(start, touch);
    }
    if (filelist->getSize() > files) {
      filelist->cleanSweep(touch);
    }
    delete reply;
    std::cout << "[" << site->getName() << " " << id << "] File list retrieved." << std::endl;
    if (!filelist->isFilled()) filelist->setFilled();
    int tmpi;
    sem_getvalue(list_refresh, &tmpi);
    if (tmpi == 0) sem_post(list_refresh);
  }
}

std::string FTPThread::getCurrentPath() {
  return currentpath;
}
std::string FTPThread::doPWD() {
  write("PWD");
  char * reply;
  if (readall(&reply, true) == 257) {
    std::string line(reply);
    delete reply;
    int loc = 0;
    while(line[++loc] != '"');
    int start = loc + 1;
    while(line[++loc] != '"');
    return line.substr(start, loc - start);
  }
  return "";
}

void FTPThread::sleepTickAsync() {
  putCommand(new CommandQueueElement(7));
}

void FTPThread::sleepTickT() {
  sem_wait(&tick);
}

bool FTPThread::doPASV(std::string ** ret) {
  sem_t donesem;
  putCommand(new CommandQueueElement(10, &donesem, (void *) ret));
  sem_wait(&donesem);
  sem_destroy(&donesem);
  if ((**ret).substr(0, 3).compare("227") != 0) return false;
  int start = (**ret).find('(') + 1;
  int end = (**ret).find(')');
  std::string * tmp = new std::string((**ret).substr(start, end-start));
  delete *ret;
  *ret = tmp;
  return true;
}

void FTPThread::doPASVT(std::string ** ret, sem_t * donesem) {
  write("PASV");
  char * reply;
  readall(&reply, true);
  *ret = new std::string(reply);
  delete reply;
  sem_post(donesem);
}

bool FTPThread::doPORT(std::string addr) {
  sem_t donesem;
  putCommand(new CommandQueueElement(11, &donesem, (void *) &addr));
  sem_wait(&donesem);
  sem_destroy(&donesem);
  return true;
}

void FTPThread::doPORTT(std::string addr, sem_t * donesem) {
  write(("PORT " + addr).c_str());
  if (read() == 200) {
    // can this command even fail? :)
  }
  sem_post(donesem);
}

void FTPThread::doCWDAsync(std::string path) {
  putCommand(new CommandQueueElement(1, (void *) new std::string(path)));
}

bool FTPThread::doCWDT(std::string path) {
  write (("CWD " + path).c_str());
  if (read() == 250) {
    currentpath = path;
    return true;
  }
  return false;
}

bool FTPThread::doMKDirT(std::string dir) {
  write(("MKD " + dir).c_str());
  if (read() == 257) return true;
  else return false;
}

void FTPThread::doCWDorMKDirAsync(std::string section, std::string release) {
  putCommand(new CommandQueueElement(3, (void *) new std::string(section), (void *) new std::string(release)));
}

void FTPThread::doCWDorMKDirT(std::string section, std::string release) {
  if (doCWDT(section + "/" + release)) return;
  if (!doCWDT(section)) return;
  doMKDirT(release);
  doCWDT(release);
}

bool FTPThread::doPRETRETR(std::string file) {
  sem_t donesem;
  int status;
  putCommand(new CommandQueueElement(22, &donesem, (void *) new std::string(file), (void *) &status));
  sem_wait(&donesem);
  if (status != 200) return false;
  return true;
}

void FTPThread::doPRETRETRT(std::string file, int * status, sem_t * donesem) {
  write (("PRET RETR " + file).c_str());
  *status = read();
  sem_post(donesem);
}

bool FTPThread::doRETR(std::string file) {
  sem_t donesem;
  int status;
  putCommand(new CommandQueueElement(20, &donesem, (void *) new std::string(file), (void *) &status));
  sem_wait(&donesem);
  if (status != 150) return false;
  return true;
}

bool FTPThread::doRETRAsyncT(std::string file, int * status, sem_t * donesem) {
  write (("RETR " + file).c_str());
  int stat = *status = read();
  if (stat == 559) stat = *status = read(); // workaround for a glftpd bug causing an extra reply
  sem_post(donesem);
  if (stat != 150) return false;
  stat = read();
  // status 226 on transfer complete
  sem_post(&transfersem);
  return true;
}

bool FTPThread::doPRETSTOR(std::string file) {
  sem_t donesem;
  int status;
  putCommand(new CommandQueueElement(23, &donesem, (void *) new std::string(file), (void *) &status));
  sem_wait(&donesem);
  if (status != 200) return false;
  return true;
}

void FTPThread::doPRETSTORT(std::string file, int * status, sem_t * donesem) {
  write (("PRET STOR " + file).c_str());
  *status = read();
  sem_post(donesem);
}

bool FTPThread::doSTOR(std::string file) {
  sem_t donesem;
  int status;
  putCommand(new CommandQueueElement(21, &donesem, (void *) new std::string(file), (void *) &status));
  sem_wait(&donesem);
  if (status != 150) return false;
  return true;
}

bool FTPThread::doSTORAsyncT(std::string file, int * status, bool * timeout, sem_t * donesem) {
  write (("STOR " + file).c_str());
  int stat = *status = read();
  if (stat == 559) stat = *status = read(); // workaround for a glftpd bug causing an extra reply
  sem_post(donesem);
  if (stat != 150) return false;
  stat = read();
  // status 226 on transfer complete
  sem_post(&transfersem);
  return true;
}

void FTPThread::abortTransfer() {
  write("ABOR");
}

void FTPThread::awaitTransferComplete() {
  sem_wait(&transfersem);
}

void FTPThread::doQUITAsync() {
  putCommand(new CommandQueueElement(100));
}

void FTPThread::doQUITT() {
  write("QUIT");
  read();
  disconnectT();
}

void FTPThread::disconnectAsync() {
  putCommand(new CommandQueueElement(101));
}

void FTPThread::disconnectT() {
  if (controlssl) SSL_shutdown(ssl);
  close(sockfd);
}

std::list<CommandQueueElement *> * FTPThread::getCommandQueue() {
  return &commandqueue;
}

void * FTPThread::run(void * arg) {
  ((FTPThread *) arg)->runInstance();
}

void * FTPThread::runTick(void * arg) {
  ((FTPThread *) arg)->runTickInstance();
}

void FTPThread::runInstance() {
  bool ret;
  while(1) {
    sem_wait(&commandsem);
    pthread_mutex_lock(&commandq_mutex);
    CommandQueueElement * command = commandqueue.front();
    pthread_mutex_unlock(&commandq_mutex);
    ready = false;
    switch(command->getOpcode()) {
      case 0:
        loginT();
        break;
      case 1:
        doCWDT(*(std::string *)command->getArg1());
        delete (std::string *)command->getArg1();
        ready = true;
        break;
      case 2:
        ready = true;
        refreshloop = true;
        currentsiterace = (SiteRace *)command->getArg1();
        break;
      case 3:
        doCWDorMKDirT(*(std::string *)command->getArg1(), *(std::string *)command->getArg2());
        delete (std::string *)command->getArg1();
        delete (std::string *)command->getArg2();
        ready = true;
        break;
      case 4:
        doUSERPASST(false);
        break;
      case 5:
        ready = true;
        break;
      case 6:
        loginKillT();
        break;
      case 7:
        sleepTickT();
        break;
      case 8:
        reconnectT();
        break;
      case 10:
        doPASVT((std::string **)command->getArg1(), command->getDoneSem());
        break;
      case 11:
        doPORTT(*(std::string *)command->getArg1(), command->getDoneSem());
        break;
      case 20:
        ret = doRETRAsyncT(*(std::string *)command->getArg1(), (int *)command->getArg2(), command->getDoneSem());
        delete (std::string *)command->getArg1();
        if (ret) ready = true;
        break;
      case 21:
        ret = doSTORAsyncT(*(std::string *)command->getArg1(), (int *)command->getArg2(), (bool *)command->getArg3(), command->getDoneSem());
        delete (std::string *)command->getArg1();
        if (ret) ready = true;
        break;
      case 22:
        doPRETRETRT(*(std::string *)command->getArg1(), (int *)command->getArg2(), command->getDoneSem());
        delete (std::string *)command->getArg1();
        break;
      case 23:
        doPRETSTORT(*(std::string *)command->getArg1(), (int *)command->getArg2(), command->getDoneSem());
        delete (std::string *)command->getArg1();
        break;
      case 100:
        doQUITT();
        break;
      case 101:
        disconnectT();
        break;
    }
    pthread_mutex_lock(&commandq_mutex);
    commandqueue.pop_front();
    pthread_mutex_unlock(&commandq_mutex);
    delete command;
    int val;
    sem_getvalue(&commandsem, &val);
    if (ready && refreshloop && val == 0) {
      refreshLoopT();
    }
  }
}

void FTPThread::runTickInstance() {
  srand(time(NULL));
  usleep (rand() % SLEEPDELAY);
  int val;
  while(1) {
    usleep(SLEEPDELAY * 1000);
    postTick();
  }
}

void FTPThread::postTick() {
  int val;
  sem_getvalue(&tick, &val);
  if (val == 0) sem_post(&tick);
}

void FTPThread::putCommand(CommandQueueElement * cqe) {
  putCommand(cqe, true);
}

void FTPThread::putCommand(CommandQueueElement * cqe, bool interruptsleep) {
  pthread_mutex_lock(&commandq_mutex);
  commandqueue.push_back(cqe);
  pthread_mutex_unlock(&commandq_mutex);
  sem_post(&commandsem);
  if (!interruptsleep) postTick();
}
