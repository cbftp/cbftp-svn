#include "remotecommandhandler.h"

RemoteCommandHandler::RemoteCommandHandler() {
  enabled = false;
  password = DEFAULTPASS;
  port = DEFAULTPORT;
  sem_init(&commandsem, 0, 0);
  pthread_mutex_init(&commandq_mutex, NULL);
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread, "RemoteCommand");
}

bool RemoteCommandHandler::isEnabled() {
  return enabled;
}

int RemoteCommandHandler::getUDPPort() {
  return port;
}

std::string RemoteCommandHandler::getPassword() {
  return password;
}

void RemoteCommandHandler::setPassword(std::string newpass) {
  password = newpass;
}

void RemoteCommandHandler::setPort(int newport) {
  bool reopen = true;
  if (port == newport || !isEnabled()) {
    reopen = false;
  }
  port = newport;
  if (reopen) {
    setEnabled(false);
    setEnabled(true);
  }
}

void * RemoteCommandHandler::run(void * arg) {
  ((RemoteCommandHandler *) arg)->runInstance();
  return NULL;
}

void RemoteCommandHandler::runInstance() {
  while(1) {
    sem_wait(&commandsem);
    pthread_mutex_lock(&commandq_mutex);
    int command = commandqueue.front();
    commandqueue.pop_front();
    pthread_mutex_unlock(&commandq_mutex);
    switch (command) {
      case 0: // connect
        connect();
        awaitIncoming();
        break;
      case 1: // disconnect
        disconnect();
        break;
    }
  }
}

void RemoteCommandHandler::awaitIncoming() {
  struct pollfd fd;
  fd.fd = sockfd;
  fd.events = POLLIN;
  char buf[MAXDATASIZE+1];
  int len;
  while(1) {
    if (poll(&fd, 1, 500) > 0) {
      len = recvfrom(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr *) 0, (socklen_t *) 0);
      buf[len] = '\0';
      handleMessage(std::string(buf));
    }
    else if (commandqueue.size() > 0) {
      return;
    }
  }
}

void RemoteCommandHandler::connect() {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_UNSPEC;
  sock.ai_socktype = SOCK_DGRAM;
  sock.ai_protocol = IPPROTO_UDP;
  getaddrinfo("0.0.0.0", global->int2Str(getUDPPort()).data(), &sock, &res);
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  bind(sockfd, res->ai_addr, res->ai_addrlen);
}

void RemoteCommandHandler::handleMessage(std::string message) {
  size_t one = message.find(" ");
  size_t two = message.find(" ", one + 1);
  size_t three = message.find(" ", two + 1);
  if (one == std::string::npos || two == std::string::npos || three == std::string::npos) {
    return;
  }
  std::string pass = message.substr(0, one);
  if (pass != password) {
    return;
  }
  std::string section = message.substr(one + 1, two - (one + 1));
  std::string release = message.substr(two + 1, three - (two + 1));
  std::string sitestring = message.substr(three + 1);
  std::list<std::string> sites;
  while (true) {
    size_t commapos = sitestring.find(",");
    if (commapos != std::string::npos) {
      sites.push_back(sitestring.substr(0, commapos));
      sitestring = sitestring.substr(commapos + 1);
    }
    else {
      sites.push_back(sitestring);
      break;
    }
  }
  global->getEngine()->newRace(release, section, sites);
}

void RemoteCommandHandler::disconnect() {
  close(sockfd);
}

void RemoteCommandHandler::setEnabled(bool enabled) {
  if ((isEnabled() && enabled) || (!isEnabled() && !enabled)) {
    return;
  }
  pthread_mutex_lock(&commandq_mutex);
  if (enabled) {
    commandqueue.push_back(0);
  }
  else {
    commandqueue.push_back(1);
  }
  pthread_mutex_unlock(&commandq_mutex);
  sem_post(&commandsem);
  this->enabled = enabled;
}

void RemoteCommandHandler::readConfiguration() {
  std::vector<std::string> lines;
  global->getDataFileHandler()->getDataFor("RemoteCommandHandler", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  bool enable = false;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("enabled")) {
      if (!value.compare("true")) {
        enable = true;
      }
    }
    else if (!setting.compare("port")) {
      setPort(global->str2Int(value));
    }
    else if (!setting.compare("password")) {
      setPassword(value);
    }
  }
  if (enable) {
    setEnabled(true);
  }
}

void RemoteCommandHandler::writeState() {
  DataFileHandler * filehandler = global->getDataFileHandler();
  if (enabled) filehandler->addOutputLine("RemoteCommandHandler", "enabled=true");
  filehandler->addOutputLine("RemoteCommandHandler", "port=" + global->int2Str(port));
  filehandler->addOutputLine("RemoteCommandHandler", "password=" + password);
}
