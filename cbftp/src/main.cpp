#include <unistd.h>
#include <csignal>
#include <cstdio>

#include "core/workmanager.h"
#include "core/iomanager.h"
#include "core/tickpoke.h"
#include "core/threading.h"
#include "settingsloadersaver.h"
#include "sitelogicmanager.h"
#include "transfermanager.h"
#include "sitemanager.h"
#include "globalcontext.h"
#include "engine.h"
#include "remotecommandhandler.h"
#include "skiplist.h"
#include "eventlog.h"
#include "proxymanager.h"
#include "localstorage.h"
#include "externalfileviewing.h"
#include "timereference.h"
#include "uibase.h"

namespace {

class Main {
public:
  Main() {
    TimeReference::updateTime();

    WorkManager * wm = new WorkManager();
    TickPoke * tp = new TickPoke(wm);
    IOManager * iom = new IOManager(wm, tp);

    Pointer<EventLog> el = makePointer<EventLog>();
    iom->setLogger(el);

    global->linkCore(wm, tp, iom, el);

    SettingsLoaderSaver * sls = new SettingsLoaderSaver();
    LocalStorage * ls = new LocalStorage();
    Engine * e = new Engine();
    SiteManager * sm = new SiteManager();
    SiteLogicManager * slm = new SiteLogicManager();
    TransferManager * tm = new TransferManager();
    RemoteCommandHandler * rch = new RemoteCommandHandler();
    SkipList * sl = new SkipList();
    ProxyManager * pm = new ProxyManager();
    ExternalFileViewing * efv = new ExternalFileViewing();
    TimeReference * tr = new TimeReference();

    UIBase * uibase = UIBase::instance();

    global->linkComponents(sls, e, uibase, sm, slm, tm, rch, sl, pm, ls, efv, tr);

    Threading::setCurrentThreadName("cbftp");

    if (!uibase->init()) exit(1);
    wm->init();
    iom->init();
    tp->tickerLoop();
    global->getExternalFileViewing()->killAll();
    uibase->kill();
    sls->saveSettings();
  }
};

void sighandler(int sig) {
  global->getTickPoke()->breakLoop();
}

}

int main(int argc, char * argv[]) {
  struct sigaction sa;
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);
  sa.sa_handler = sighandler;
  sigaction(SIGABRT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);

  Main();
}


