#include "main.h"

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "datafilehandler.h"
#include "sitelogicmanager.h"
#include "transfermanager.h"
#include "sitemanager.h"
#include "globalcontext.h"
#include "engine.h"
#include "ui/ui.h"
#include "tickpoke.h"
#include "remotecommandhandler.h"
#include "iomanager.h"
#include "workmanager.h"
#include "skiplist.h"
#include "eventlog.h"
#include "proxymanager.h"
#include "localstorage.h"
#include "externalfileviewing.h"
#include "timereference.h"

Main::Main() {
  std::string datadirpath = std::string(getenv("HOME")) + "/" + DATAPATH;
  std::string datapath = datadirpath + "/" + DATAFILE;
  char * specialdatapath = getenv("CLUSTERBOMB_DATA_PATH");
  if (specialdatapath != NULL) {
    datapath = std::string(specialdatapath);
  }
  else if (access(datadirpath.c_str(), F_OK) == 0) {
    if (access(datadirpath.c_str(), R_OK | W_OK) < 0) {
      perror(std::string("Error: could not access " + datadirpath).c_str());
      exit(1);
    }
    struct stat status;
    stat(datadirpath.c_str(), &status);
    if (!(status.st_mode & S_IFDIR)) {
      std::cout << "Error: could not create " << datadirpath << ": File exists" << std::endl;
      exit(1);
    }
  }
  else {
    if (mkdir(datadirpath.c_str(), 0700) < 0) {
      perror(std::string("Error: could not create " + datadirpath).c_str());
      exit(1);
    }
  }
  forever = true;
  global = new GlobalContext();
  EventLog * el = new EventLog();
  global->linkEventLog(el);
  WorkManager * wm = new WorkManager();
  global->linkWorkManager(wm);
  TickPoke * tp = new TickPoke();
  global->linkTickPoke(tp);
  DataFileHandler * dfh = new DataFileHandler(datapath);
  IOManager * iom = new IOManager();
  Engine * e = new Engine();
  Ui * ui = new Ui();
  SiteManager * sm = new SiteManager();
  SiteLogicManager * slm = new SiteLogicManager();
  TransferManager * tm = new TransferManager();
  RemoteCommandHandler * rch = new RemoteCommandHandler();
  SkipList * sl = new SkipList();
  ProxyManager * pm = new ProxyManager();
  LocalStorage * ls = new LocalStorage();
  ExternalFileViewing * efv = new ExternalFileViewing();
  TimeReference * tr = new TimeReference();
  global->linkComponents(dfh, iom, e, ui, sm, slm, tm, rch, sl, pm, ls, efv, tr);
  if (!ui->init()) exit(1);
  tp->tickerLoop();
  global->getExternalFileViewing()->killAll();
  ui->kill();
  if (global->getDataFileHandler()->isInitialized()) {
    std::cout << "Saving data to file..." << std::endl;
    global->getSiteManager()->writeState();
    global->getRemoteCommandHandler()->writeState();
    ui->writeState();
    global->getSkipList()->writeState();
    global->getProxyManager()->writeState();
    global->getIOManager()->writeState();
    global->getExternalFileViewing()->writeState();
    global->getLocalStorage()->writeState();
    global->getDataFileHandler()->writeFile();
    std::cout << "Done, exiting..." << std::endl << std::flush;
  }
}

int main(int argc, char * argv[]) {
  global->signal_catch();
  new Main();
}

void sighandler(int sig) {
  global->signal_ignore();
  global->getTickPoke()->breakLoop();
}

void sighandler_winch(int sig) {
  signal(SIGWINCH, &sighandler_ignore);
  global->getUIBase()->terminalSizeChanged();
  signal(SIGWINCH, &sighandler_winch);
}

void sighandler_ignore(int sig) {
}

