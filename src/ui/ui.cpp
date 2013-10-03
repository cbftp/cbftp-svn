#include "ui.h"

#include <sys/ioctl.h>

#include "../globalcontext.h"
#include "../tickpoke.h"
#include "../iomanager.h"
#include "../sitemanager.h"
#include "../remotecommandhandler.h"
#include "../skiplist.h"
#include "../datafilehandler.h"

#include "legendwindow.h"
#include "infowindow.h"
#include "uiwindow.h"

#include "screens/loginscreen.h"
#include "screens/newkeyscreen.h"
#include "screens/mainscreen.h"
#include "screens/editsitescreen.h"
#include "screens/confirmationscreen.h"
#include "screens/sitestatusscreen.h"
#include "screens/rawdatascreen.h"
#include "screens/browsescreen.h"
#include "screens/addsectionscreen.h"
#include "screens/newracescreen.h"
#include "screens/racestatusscreen.h"
#include "screens/rawcommandscreen.h"
#include "screens/globaloptionsscreen.h"
#include "screens/skiplistscreen.h"
#include "screens/changekeyscreen.h"

extern GlobalContext * global;

UserInterface::UserInterface() {
  sem_init(&initstart, 0, 0);
  sem_init(&initdone, 0, 0);
  sem_init(&keyeventdone, 0, 0);
  main = NULL;
  topwindow = NULL;
  legendenabled = false;
  infoenabled = false;
  pthread_create(&uithread, global->getPthreadAttr(), run, (void *) this);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(uithread, "UserInterface");
#endif
}

bool UserInterface::init() {
  initret = true;
  sem_post(&initstart);
  sem_wait(&initdone);
  return initret;
}

void UserInterface::initIntern() {
  initscr();
  cbreak();
  curs_set(0);
  refresh();
  getmaxyx(stdscr, row, col);
  if (row < 24 || col < 80) {
    uicommunicator.kill();
    printf("Error: terminal too small. 80x24 required. (Current %dx%d)\n", col, row);
    initret = false;
  }
  set_escdelay(25);
  global->getIOManager()->registerStdin(this);
  global->getTickPoke()->startPoke(this, 250, 0);
  sem_post(&initdone);
}



void UserInterface::refreshAll() {
  if (legendenabled) {
    wnoutrefresh(legend);
  }
  if (infoenabled) {
    wnoutrefresh(info);
  }
  wnoutrefresh(main);
  doupdate();
}

void UserInterface::FDData() {
  keyqueue.push_back(getch());
  uicommunicator.emitEvent(UI_EVENT_KEYBOARD);
}

void UserInterface::enableInfo() {
  if (!infoenabled) {
    infoenabled = true;
    mainrow = mainrow - 2;
    wresize(main, mainrow, maincol);
    mvwin(main, 2, 0);
    redrawAll();
    curs_set(0);
    refreshAll();
  }
}

void UserInterface::enableLegend() {
  if (!legendenabled) {
    legendenabled = true;
    mainrow = mainrow - 2;
    wresize(main, mainrow, maincol);
    redrawAll();
    curs_set(0);
    refreshAll();
  }
}

void UserInterface::redrawAll() {
  std::vector<UIWindow *>::iterator it;
  for (it = mainwindows.begin(); it != mainwindows.end(); it++) {
    (*it)->resize(mainrow, maincol);
  }
  if (legendenabled) {
    legendwindow->resize(2, col);
  }
  if (infoenabled) {
    infowindow->resize(2, col);
  }
  if (topwindow != NULL) {
    topwindow->redraw();
  }
}

void UserInterface::disableInfo() {
  if (infoenabled) {
    infoenabled = false;
    mainrow = mainrow + 2;
    wresize(main, mainrow, maincol);
    wmove(main, 0, 0);
    redrawAll();
    curs_set(0);
    refreshAll();
  }
}

void UserInterface::disableLegend() {
  if (legendenabled) {
    legendenabled = false;
    mainrow = mainrow + 2;
    wresize(main, mainrow, maincol);
    redrawAll();
    curs_set(0);
    refreshAll();
  }
}

void UserInterface::tick(int message) {
  uicommunicator.emitEvent(UI_EVENT_POKE);
}

void UserInterface::runInstance() {
  sem_wait(&initstart);
  initIntern();
  mainrow = row;
  maincol = col;
  main = newwin(row, col, 0, 0);
  legend = newwin(2, col, row - 2, 0);
  info = newwin(2, col, 0, 0);
  UIWindow * startscreen = NULL;
  UIWindow * confirmationscreen = NULL;
  UIWindow * mainscreen = NULL;
  UIWindow * editsitescreen = NULL;
  UIWindow * sitestatusscreen = NULL;
  UIWindow * rawdatascreen = NULL;
  UIWindow * rawcommandscreen = NULL;
  UIWindow * browsescreen = NULL;
  UIWindow * addsectionscreen = NULL;
  UIWindow * newracescreen = NULL;
  UIWindow * racestatusscreen = NULL;
  UIWindow * globaloptionsscreen = NULL;
  UIWindow * skiplistscreen = NULL;
  UIWindow * changekeyscreen = NULL;
  legendwindow = new LegendWindow(legend, 2, col);
  infowindow = new InfoWindow(info, 2, col);
  if (global->getDataFileHandler()->fileExists()) {
    startscreen = new LoginScreen(main, &uicommunicator, mainrow, maincol);
    mainwindows.push_back(startscreen);
  }
  else {
    enableInfo();
    if (uicommunicator.legendEnabled()) {
      enableLegend();
    }
    startscreen = new NewKeyScreen(main, &uicommunicator, mainrow, maincol);
    legendwindow->setText(startscreen->getLegendText());

    mainwindows.push_back(startscreen);
  }
  topwindow = startscreen;
  infowindow->setLabel(topwindow->getInfoLabel());
  keypad(stdscr, TRUE);
  noecho();
  std::cin.putback('#'); // needed to be able to peek properly
  refreshAll();
  int ch;
  struct winsize size;
  bool trytodraw;
  while(1) {
    int currentevent = uicommunicator.awaitEvent();
    switch (currentevent) {
      case UI_EVENT_POKE:
        if (topwindow->autoUpdate()) {
          topwindow->update();
        }
        if (legendenabled) {
          legendwindow->update();
        }
        if (infoenabled) {
          infowindow->setText(topwindow->getInfoText());
          infowindow->update();
        }
        refreshAll();
        break;
      case UI_EVENT_KEYBOARD:
        ch = keyqueue.front();
        keyqueue.pop_front();
        topwindow->keyPressed(ch);
        break;
      case UI_EVENT_UPDATE:
        topwindow->update();
        refreshAll();
        break;
      case UI_EVENT_SHOWLEGEND:
        enableLegend();
        break;
      case UI_EVENT_HIDELEGEND:
        disableLegend();
        break;
      case UI_EVENT_RESIZE:
        trytodraw = false;
        if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0) {
          if (size.ws_row >= 5 && size.ws_col >= 10) {
            trytodraw = true;
          }
        }
        if (trytodraw) {
          resizeterm(size.ws_row, size.ws_col);
          endwin();
          timeout(0);
          while (getch() != ERR);
          timeout(-1);
          refresh();
          getmaxyx(stdscr, row, col);
          maincol = col;
          mainrow = row;
          if (legendenabled) {
            mainrow = mainrow - 2;
          }
          if (infoenabled) {
            mainrow = mainrow - 2;
          }
          wresize(main, mainrow, maincol);
          wresize(legend, 2, col);
          wresize(info, 2, col);
          mvwin(legend, row - 2, 0);
          redrawAll();
          refreshAll();
        }
        break;
      case UI_EVENT_KILL:
        endwin();
        uicommunicator.dead();
        return;
    }
    if (uicommunicator.hasNewCommand()) {
      std::string command = uicommunicator.getCommand();
      if (command == "editsite") {
        editsitescreen = new EditSiteScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(editsitescreen);

      }
      else if (command == "racestatus") {
        racestatusscreen = new RaceStatusScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(racestatusscreen);
      }
      else if (command == "returnracestatus") {
        history.clear();
        topwindow = mainscreen;
        racestatusscreen = new RaceStatusScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(racestatusscreen);
      }
      else if (command == "addsection") {
        addsectionscreen = new AddSectionScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(addsectionscreen);
      }
      else if (command == "newrace") {
        newracescreen = new NewRaceScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(newracescreen);
      }
      else if (command == "sitestatus") {
        sitestatusscreen = new SiteStatusScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(sitestatusscreen);
      }
      else if (command == "rawdata") {
        rawdatascreen = new RawDataScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(rawdatascreen);
      }
      else if (command  == "rawcommand") {
        rawcommandscreen = new RawCommandScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(rawcommandscreen);
      }
      else if (command == "browse") {
        browsescreen = new BrowseScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(browsescreen);
      }
      else if (command == "globaloptions") {
        globaloptionsscreen = new GlobalOptionsScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(globaloptionsscreen);
      }
      else if (command == "confirmation") {
        confirmationscreen = new ConfirmationScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(confirmationscreen);
      }
      else if (command == "skiplist") {
        skiplistscreen = new SkipListScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(skiplistscreen);
      }
      else if (command == "changekey") {
        changekeyscreen = new ChangeKeyScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(changekeyscreen);
      }
      else if (command == "rawdatajump") {
        rawdatascreen = new RawDataScreen(main, &uicommunicator, mainrow, maincol);
        mainwindows.push_back(rawdatascreen);
        topwindow = rawdatascreen;
        infowindow->setLabel(topwindow->getInfoLabel());
        infowindow->setText(topwindow->getInfoText());
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "update") {
        topwindow->update();
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "updatesetlegend") {
        topwindow->update();
        legendwindow->setText(topwindow->getLegendText());
        infowindow->setText(topwindow->getInfoText());
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "updatesetinfo") {
        topwindow->update();
        infowindow->setText(topwindow->getInfoText());
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "updatesetinfolegend") {
        topwindow->update();
        legendwindow->setText(topwindow->getLegendText());
        infowindow->setText(topwindow->getInfoText());
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "redraw") {
        topwindow->redraw();
        infowindow->setText(topwindow->getInfoText());
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "key") {
        std::string key = uicommunicator.getArg1();
        uicommunicator.checkoutCommand();
        bool result = global->getDataFileHandler()->tryDecrypt(key);
        if (result) {
          global->getSiteManager()->readConfiguration();
          global->getRemoteCommandHandler()->readConfiguration();
          uicommunicator.readConfiguration();
          global->getSkipList()->readConfiguration();
          enableInfo();
          if (uicommunicator.legendEnabled()) {
            enableLegend();
          }
          mainscreen = new MainScreen(main, &uicommunicator, mainrow, maincol);
          switchToWindow(mainscreen);
        }
        else {
          topwindow->update();
          refreshAll();
        }
      }
      else if (command == "newkey") {
        std::string key = uicommunicator.getArg1();
        uicommunicator.checkoutCommand();
        global->getDataFileHandler()->newDataFile(key);
        mainscreen = new MainScreen(main, &uicommunicator, mainrow, maincol);
        switchToWindow(mainscreen);
      }
      else if (command == "main") {
        mainscreen->redraw();
        topwindow = mainscreen;
        legendwindow->setText(topwindow->getLegendText());
        infowindow->setLabel(topwindow->getInfoLabel());
        infowindow->setText(topwindow->getInfoText());
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "return") {
        topwindow = history.back();
        history.pop_back();
        uicommunicator.checkoutCommand();
        legendwindow->setText(topwindow->getLegendText());
        infowindow->setLabel(topwindow->getInfoLabel());
        infowindow->setText(topwindow->getInfoText());
        topwindow->redraw();
        refreshAll();
      }
      else if (command == "yes" || command == "no") {
        topwindow = history.back();
        history.pop_back();
        legendwindow->setText(topwindow->getLegendText());
        infowindow->setLabel(topwindow->getInfoLabel());
        infowindow->setText(topwindow->getInfoText());
        topwindow->update();
        refreshAll();
      }
    }
  }
}

void UserInterface::switchToWindow(UIWindow * window) {
  history.push_back(topwindow);
  legendwindow->setText(window->getLegendText());
  infowindow->setLabel(window->getInfoLabel());
  infowindow->setText(window->getInfoText());
  mainwindows.push_back(window);
  topwindow = window;
  refreshAll();
  uicommunicator.checkoutCommand();
}

void * UserInterface::run(void * arg) {
  ((UserInterface *) arg)->runInstance();
  return NULL;
}

UICommunicator * UserInterface::getCommunicator() {
  return &uicommunicator;
}
