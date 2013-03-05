#include "ui.h"

UserInterface::UserInterface() {
  sem_init(&initstart, 0, 0);
  sem_init(&initdone, 0, 0);
  sem_init(&keyeventdone, 0, 0);
  main = NULL;
  topwindow = NULL;
  legendenabled = false;
  pthread_create(&klthread, global->getPthreadAttr(), runKeyListener, (void *) this);
  pthread_create(&uithread, global->getPthreadAttr(), runUserInterface, (void *) this);
  pthread_setname_np(klthread, "UIKeyThread");
  pthread_setname_np(uithread, "UIThread");
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
  sem_post(&initdone);
}



void UserInterface::refreshAll() {
  if (legendenabled) {
    wnoutrefresh(legend);
  }
  wnoutrefresh(main);
  doupdate();
}

void UserInterface::stopTicker() {
  //tickerrun = false;
}

void UserInterface::startTicker(int updateinterval) {
  //this->updateinterval = updateinterval;
  //tickerrun = true;
  //sem_post(&tickeractivate);
}

void UserInterface::runKeyListenerInstance() {
  fd_set readfds;
  int select_result;
  while(1) {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    select_result = select(STDIN_FILENO+1, &readfds, NULL, NULL, NULL);
    if (select_result < 0) {
      perror("Error in select() on stdin");
    }
    else if (FD_ISSET(STDIN_FILENO, &readfds)) {
      //incoming keypress!
      uicommunicator.emitEvent("keyboard");
      sem_wait(&keyeventdone);
    }
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
    legendwindow->resize(row, col);
  }
  if (topwindow != NULL) {
    topwindow->redraw();
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

void UserInterface::runUserInterfaceInstance() {
  sem_wait(&initstart);
  initIntern();
  mainrow = row;
  maincol = col;
  main = newwin(row, col, 0, 0);
  legend = newwin(2, col, row - 2, 0);
  UIWindow * startscreen;
  UIWindow * confirmationscreen;
  UIWindow * mainscreen;
  UIWindow * editsitescreen;
  UIWindow * sitestatusscreen;
  UIWindow * rawdatascreen;
  UIWindow * rawcommandscreen;
  UIWindow * browsescreen;
  UIWindow * addsectionscreen;
  UIWindow * newracescreen;
  UIWindow * racestatusscreen;
  sem_t * eventsem = uicommunicator.getEventSem();
  global->getTickPoke()->startPoke(eventsem, 250, 0);
  legendwindow = new LegendWindow(legend, 2, col);
  std::list<UIWindow *> history;
  if (global->getDataFileHandler()->fileExists()) {
    startscreen = new LoginScreen(main, &uicommunicator, mainrow, maincol);
    mainwindows.push_back(startscreen);
  }
  else {
    enableLegend();
    startscreen = new NewKeyScreen(main, &uicommunicator, mainrow, maincol);
    legendwindow->setText(startscreen->getLegendText());
    mainwindows.push_back(startscreen);
  }
  topwindow = startscreen;
  keypad(stdscr, TRUE);
  noecho();
  refreshAll();
  while(1) {
    std::string currentevent = uicommunicator.awaitEvent();
    if (global->getTickPoke()->isPoked(eventsem)) {
      global->getTickPoke()->getMessage(eventsem);
      if (topwindow->autoUpdate()) {
        topwindow->update();
      }
      if (legendenabled) {
        legendwindow->update();
      }
      refreshAll();
    }
    else if (currentevent == "keyboard") {
      int ch = getch();
      sem_post(&keyeventdone);
      topwindow->keyPressed(ch);
    }
    else if (currentevent == "update") {
      topwindow->update();
      refreshAll();
    }
    else if (currentevent == "updatelegend") {
      legendwindow->update();
      refreshAll();
    }
    else if (currentevent == "resize") {
      struct winsize size;
      if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0) {
        resizeterm(size.ws_row, size.ws_col);
      }
      endwin();
      timeout(0);
      while (getch() != ERR);
      timeout(-1);
      refresh();
      getmaxyx(stdscr, row, col);
      maincol = col;
      if (legendenabled) {
    	  mainrow = row - 2;
      }
      else {
    	  mainrow = row;
      }
      wresize(main, mainrow, maincol);
      wresize(legend, 2, col);
      mvwin(legend, row - 2, 0);
      redrawAll();
      refreshAll();
    }

    if (uicommunicator.hasNewCommand()) {
      std::string command = uicommunicator.getCommand();
      if (command == "editsite") {
        history.push_back(topwindow);
        editsitescreen = new EditSiteScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(editsitescreen->getLegendText());
        mainwindows.push_back(editsitescreen);
        topwindow = editsitescreen;
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "racestatus") {
        history.push_back(topwindow);
        racestatusscreen = new RaceStatusScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(racestatusscreen->getLegendText());
        mainwindows.push_back(racestatusscreen);
        topwindow = racestatusscreen;
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "addsection") {
        history.push_back(topwindow);
        addsectionscreen = new AddSectionScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(addsectionscreen->getLegendText());
        mainwindows.push_back(addsectionscreen);
        topwindow = addsectionscreen;
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "newrace") {
        history.push_back(topwindow);
        newracescreen = new NewRaceScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(newracescreen->getLegendText());
        mainwindows.push_back(newracescreen);
        topwindow = newracescreen;
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "sitestatus") {
        history.push_back(topwindow);
        sitestatusscreen = new SiteStatusScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(sitestatusscreen->getLegendText());
        mainwindows.push_back(sitestatusscreen);
        topwindow = sitestatusscreen;
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "rawdata") {
        history.push_back(topwindow);
        rawdatascreen = new RawDataScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(rawdatascreen->getLegendText());
        mainwindows.push_back(rawdatascreen);
        topwindow = rawdatascreen;
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command  == "rawcommand") {
        history.push_back(topwindow);
        rawcommandscreen = new RawCommandScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(rawcommandscreen->getLegendText());
        mainwindows.push_back(rawcommandscreen);
        topwindow = rawcommandscreen;
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "rawdatajump") {
        rawdatascreen = new RawDataScreen(main, &uicommunicator, mainrow, maincol);
        mainwindows.push_back(rawdatascreen);
        topwindow = rawdatascreen;
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
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "redraw") {
        topwindow->redraw();
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "key") {
        std::string key = uicommunicator.getArg1();
        uicommunicator.checkoutCommand();
        bool result = global->getDataFileHandler()->tryDecrypt(key);
        if (result) {
          global->getSiteManager()->readSites();
          enableLegend();
          mainscreen = new MainScreen(main, &uicommunicator, mainrow, maincol);
          legendwindow->setText(mainscreen->getLegendText());
          mainwindows.push_back(mainscreen);
          topwindow = mainscreen;
          refreshAll();
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
        legendwindow->setText(mainscreen->getLegendText());
        mainwindows.push_back(mainscreen);
        topwindow = mainscreen;
        refreshAll();
      }
      if (command == "browse") {
        history.push_back(topwindow);
        browsescreen = new BrowseScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(browsescreen->getLegendText());
        mainwindows.push_back(browsescreen);
        topwindow = browsescreen;
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "confirmation") {
        history.push_back(topwindow);
        uicommunicator.checkoutCommand();
        confirmationscreen = new ConfirmationScreen(main, &uicommunicator, mainrow, maincol);
        legendwindow->setText(confirmationscreen->getLegendText());
        mainwindows.push_back(mainscreen);
        topwindow = confirmationscreen;
        refreshAll();
      }
      else if (command == "main") {
        mainscreen->redraw();
        topwindow = mainscreen;
        legendwindow->setText(mainscreen->getLegendText());
        refreshAll();
        uicommunicator.checkoutCommand();
      }
      else if (command == "return") {
        topwindow = history.back();
        history.pop_back();
        uicommunicator.checkoutCommand();
        legendwindow->setText(topwindow->getLegendText());
        topwindow->redraw();
        refreshAll();
      }
      else if (command == "yes" || command == "no") {
        topwindow = history.back();
        history.pop_back();
        legendwindow->setText(topwindow->getLegendText());
        topwindow->update();
        refreshAll();
      }
    }
  }
}

void * UserInterface::runKeyListener(void * arg) {
  ((UserInterface *) arg)->runKeyListenerInstance();
  return NULL;
}

void * UserInterface::runUserInterface(void * arg) {
  ((UserInterface *) arg)->runUserInterfaceInstance();
  return NULL;
}

UICommunicator * UserInterface::getCommunicator() {
  return &uicommunicator;
}
