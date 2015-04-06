#include <iostream>
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

#define DATAFILE "data"
#define DATAPATH ".clusterbomb"

GlobalContext * global;

std::string setupDataDir(LocalStorage * ls) {
  std::string datadirpath = std::string(getenv("HOME")) + "/" + DATAPATH;
  std::string datapath = datadirpath + "/" + DATAFILE;
  char * specialdatapath = getenv("CLUSTERBOMB_DATA_PATH");
  if (specialdatapath != NULL) {
    datapath = std::string(specialdatapath);
  }
  else if (ls->directoryExistsReadable(datadirpath)) {
    if (!ls->directoryExistsWritable(datadirpath)) {
      perror(std::string("Error: no write access to " + datadirpath).c_str());
      exit(1);
    }
  }
  else {
    if (!ls->createDirectory(datadirpath, true)) {
      perror(std::string("Error: could not create " + datadirpath).c_str());
      exit(1);
    }
  }
  return datapath;
}

class Main {
public:
  Main() {
    LocalStorage * ls = new LocalStorage();
    std::string datapath = setupDataDir(ls);
    DataFileHandler * dfh = new DataFileHandler(datapath);
    global = new GlobalContext();
    EventLog * el = new EventLog();
    global->linkEventLog(el);
    WorkManager * wm = new WorkManager();
    global->linkWorkManager(wm);
    TickPoke * tp = new TickPoke();
    global->linkTickPoke(tp);
    IOManager * iom = new IOManager();
    Engine * e = new Engine();
    Ui * ui = new Ui();
    SiteManager * sm = new SiteManager();
    SiteLogicManager * slm = new SiteLogicManager();
    TransferManager * tm = new TransferManager();
    RemoteCommandHandler * rch = new RemoteCommandHandler();
    SkipList * sl = new SkipList();
    ProxyManager * pm = new ProxyManager();
    ExternalFileViewing * efv = new ExternalFileViewing();
    TimeReference * tr = new TimeReference();
    global->linkComponents(dfh, iom, e, ui, sm, slm, tm, rch, sl, pm, ls, efv, tr);
    if (!ui->init()) exit(1);
    iom->init();
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
};

void sighandler_ignore(int sig) {
}

void signal_ignore() {
  signal(SIGABRT, &sighandler_ignore);
  signal(SIGTERM, &sighandler_ignore);
  signal(SIGINT, &sighandler_ignore);
  signal(SIGWINCH, &sighandler_ignore);
}

void sighandler(int sig) {
  signal_ignore();
  global->getTickPoke()->breakLoop();
}

void sighandler_winch(int sig) {
  signal(SIGWINCH, &sighandler_ignore);
  global->getUIBase()->terminalSizeChanged();
  signal(SIGWINCH, &sighandler_winch);
}

void signal_catch() {
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  signal(SIGWINCH, &sighandler_winch);
}

int main(int argc, char * argv[]) {
  signal_catch();
  Main();
}
