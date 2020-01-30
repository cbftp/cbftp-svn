#include <unistd.h>
#include <csignal>
#include <cstdio>

#include "core/workmanager.h"
#include "core/iomanager.h"
#include "core/tickpoke.h"
#include "core/threading.h"
#include "core/logger.h"
#include "core/signal.h"
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
#include "statistics.h"
#include "sectionmanager.h"
#include "httpserver.h"
#include "restapi.h"

namespace {

class Main {
public:
  Main() {
    TimeReference::updateTime();

    Core::WorkManager* wm = new Core::WorkManager();
    Core::TickPoke* tp = new Core::TickPoke(*wm);
    Core::IOManager* iom = new Core::IOManager(*wm, *tp);

    std::shared_ptr<EventLog> el = std::make_shared<EventLog>();
    Core::setLogger(el);

    global->linkCore(wm, tp, iom, el);

    SettingsLoaderSaver* sls = new SettingsLoaderSaver();
    LocalStorage* ls = new LocalStorage();
    Engine* e = new Engine();
    SiteManager* sm = new SiteManager();
    SiteLogicManager* slm = new SiteLogicManager();
    TransferManager* tm = new TransferManager();
    RemoteCommandHandler* rch = new RemoteCommandHandler();
    SkipList* sl = new SkipList();
    ProxyManager* pm = new ProxyManager();
    ExternalFileViewing* efv = new ExternalFileViewing();
    TimeReference* tr = new TimeReference();
    Statistics* s = new Statistics();
    SectionManager* secm = new SectionManager();
    HTTPServer* httprv = new HTTPServer();
    RestApi* ra = new RestApi();

    UIBase* uibase = UIBase::instance();

    global->linkComponents(sls, e, uibase, sm, slm, tm, rch, sl, pm, ls, efv,
                           tr, s, secm, httprv, ra);

    Core::Threading::setCurrentThreadName("cbftp");

    if (!uibase->init()) exit(1);
    wm->init("cbftp");
    iom->init("cbftp");
    sls->init();
    tp->tickerLoop();
    global->getExternalFileViewing()->killAll();
    uibase->kill();
    sls->saveSettings();
  }
};

void sighandler(int sig) {
  global->getTickPoke()->breakLoop();
}

} // namespace

int main(int argc, char* argv[]) {
  Core::registerSignalHandler(SIGABRT, sighandler);
  Core::registerSignalHandler(SIGTERM, sighandler);
  Core::registerSignalHandler(SIGINT, sighandler);
  Core::registerSignalHandler(SIGQUIT, sighandler);

  Main();
}


