#include "ui.h"

UserInterface::UserInterface() {
  sem_init(&initstart, 0, 0);
  sem_init(&initdone, 0, 0);
  sem_init(&event, 0, 0);
  sem_init(&event_ready, 0, 1);
  sem_init(&keyeventdone, 0, 0);
  main = NULL;
  tickerenabled = false;
  legendenabled = false;
  pthread_create(&thread[0], global->getPthreadAttr(), runKeyListener, (void *) this);
  pthread_create(&thread[1], global->getPthreadAttr(), runUserInterface, (void *) this);
  pthread_create(&thread[2], global->getPthreadAttr(), runTicker, (void *) this);
}

bool UserInterface::init() {
  initret = true;
  sem_post(&initstart);
  sem_wait(&initdone);
  return initret;
}

void UserInterface::initIntern() {
  initscr();
  curs_set(0);
  getmaxyx(stdscr, row, col);
  if (row < 24 || col < 80) {
    kill();
    printf("Error: terminal too small. 80x24 required. (Current %dx%d)\n", col, row);
    initret = false;
  }
  sem_post(&initdone);
}

void UserInterface::kill() {
  endwin();
}

void UserInterface::putTopRefresh(WINDOW * window) {
  touchwin(window);
  wnoutrefresh(window);
  doupdate();
}

void UserInterface::updateMain() {

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
      sem_wait(&event_ready);
      eventtext = "keyboard";
      sem_post(&event);
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
    putTopRefresh(legend);
    putTopRefresh(main);
  }
}

void UserInterface::redrawAll() {
  std::vector<UIWindow *>::iterator it;
  for (it = mainwindows.begin(); it != mainwindows.end(); it++) {
    (*it)->redraw(mainrow, maincol);
  }
  legendwindow->redraw(2, maincol);
  curs_set(0);
}

void UserInterface::disableLegend() {
  if (legendenabled) {
    legendenabled = false;
    mainrow = mainrow + 2;
    wresize(main, mainrow, maincol);
    redrawAll();
    curs_set(0);
  }
}

void UserInterface::runUserInterfaceInstance() {
  sem_wait(&initstart);
  initIntern();
  mainrow = row;
  maincol = col;
  main = newwin(row, col, 0, 0);
  legend = newwin(2, col, row-2, 0);
  UIWindow * startscreen;
  UIWindow * confirmationscreen;
  UIWindow * mainscreen;
  UIWindow * editsitescreen;
  UIWindow * sitestatusscreen;
  UIWindow * rawdatascreen;
  legendwindow = new LegendWindow(legend, row, col);
  std::list<UIWindow *> history;
  if (global->getDataFileHandler()->fileExists()) {
    startscreen = new LoginScreen(main, &windowcommand, mainrow, maincol);
    mainwindows.push_back(startscreen);
  }
  else {
    enableLegend();

    startscreen = new NewKeyScreen(main, &windowcommand, mainrow, maincol);
    legendwindow->setText(startscreen->getLegendText());
    mainwindows.push_back(startscreen);
  }
  topwindow = startscreen;
  putTopRefresh(main);
  keypad(main, TRUE);
  noecho();
  while(1) {
    sem_wait(&event);
    std::string currentevent = eventtext;
    sem_post(&event_ready);
    if (currentevent == "keyboard") {
      int ch = wgetch(main);
      sem_post(&keyeventdone);
      topwindow->keyPressed(ch);
    }
    else if (currentevent == "update") {
      windowcommand.checkoutCommand();
      topwindow->update();
      putTopRefresh(main);
    }
    else if (currentevent == "updatelegend") {
      windowcommand.checkoutCommand();
      legendwindow->update();
      putTopRefresh(legend);
      putTopRefresh(main);
    }
    else if (currentevent == "resize") {
      windowcommand.checkoutCommand();
      getmaxyx(stdscr, row, col);
      topwindow->redraw(row, col);
      putTopRefresh(main);
    }

    if (windowcommand.hasNewCommand()) {
      std::string command = windowcommand.getCommand();
      if (command == "editsite") {
        history.push_back(topwindow);
        editsitescreen = new EditSiteScreen(main, &windowcommand, mainrow, maincol);
        legendwindow->setText(editsitescreen->getLegendText());
        mainwindows.push_back(editsitescreen);
        topwindow = editsitescreen;
        putTopRefresh(legend);
        putTopRefresh(main);
        windowcommand.checkoutCommand();
      }
      else if (command == "sitestatus") {
        history.push_back(topwindow);
        sitestatusscreen = new SiteStatusScreen(main, &windowcommand, mainrow, maincol);
        legendwindow->setText(sitestatusscreen->getLegendText());
        mainwindows.push_back(sitestatusscreen);
        topwindow = sitestatusscreen;
        putTopRefresh(legend);
        putTopRefresh(main);
        windowcommand.checkoutCommand();
      }
      else if (command == "rawdata") {
        history.push_back(topwindow);
        rawdatascreen = new RawDataScreen(main, &windowcommand, mainrow, maincol);
        legendwindow->setText(rawdatascreen->getLegendText());
        mainwindows.push_back(rawdatascreen);
        topwindow = rawdatascreen;
        putTopRefresh(legend);
        putTopRefresh(main);
        windowcommand.checkoutCommand();
      }
      else if (command == "rawdatajump") {
        rawdatascreen = new RawDataScreen(main, &windowcommand, mainrow, maincol);
        mainwindows.push_back(rawdatascreen);
        topwindow = rawdatascreen;
        putTopRefresh(main);
        windowcommand.checkoutCommand();
      }
      else if (command == "update") {
        topwindow->update();
        putTopRefresh(main);
        windowcommand.checkoutCommand();
      }
      else if (command == "updatesetlegend") {
        topwindow->update();
        legendwindow->setText(topwindow->getLegendText());
        putTopRefresh(legend);
        putTopRefresh(main);
        windowcommand.checkoutCommand();
      }
      else if (command == "redraw") {
        topwindow->redraw();
        putTopRefresh(main);
        windowcommand.checkoutCommand();
      }
      else if (command == "key") {
        std::string key = windowcommand.getArg1();
        windowcommand.checkoutCommand();
        bool result = global->getDataFileHandler()->tryDecrypt(key);
        if (result) {
          global->getSiteManager()->readSites();
          enableLegend();
          mainscreen = new MainScreen(main, &windowcommand, mainrow, maincol);
          legendwindow->setText(mainscreen->getLegendText());
          mainwindows.push_back(mainscreen);
          topwindow = mainscreen;
          putTopRefresh(legend);
          putTopRefresh(main);
        }
        else {
          topwindow->update();
          putTopRefresh(main);
        }
      }
      else if (command == "newkey") {
        std::string key = windowcommand.getArg1();
        windowcommand.checkoutCommand();
        global->getDataFileHandler()->newDataFile(key);
        mainscreen = new MainScreen(main, &windowcommand, mainrow, maincol);
        legendwindow->setText(mainscreen->getLegendText());
        mainwindows.push_back(mainscreen);
        topwindow = mainscreen;
        putTopRefresh(legend);
        putTopRefresh(main);
      }
      else if (command == "confirmation") {
        history.push_back(topwindow);
        windowcommand.checkoutCommand();
        confirmationscreen = new ConfirmationScreen(main, &windowcommand, mainrow, maincol);
        legendwindow->setText(confirmationscreen->getLegendText());
        mainwindows.push_back(mainscreen);
        topwindow = confirmationscreen;
        putTopRefresh(legend);
        putTopRefresh(main);
      }
      else if (command == "main") {
        mainscreen->redraw();
        topwindow = mainscreen;
        legendwindow->setText(mainscreen->getLegendText());
        putTopRefresh(legend);
        putTopRefresh(main);
        windowcommand.checkoutCommand();
      }
      else if (command == "return") {
        topwindow = history.back();
        history.pop_back();
        windowcommand.checkoutCommand();
        legendwindow->setText(topwindow->getLegendText());
        topwindow->redraw();
        putTopRefresh(legend);
        putTopRefresh(main);
      }
      else if (command == "yes" || command == "no") {
        topwindow = history.back();
        history.pop_back();
        legendwindow->setText(topwindow->getLegendText());
        topwindow->update();
        putTopRefresh(legend);
        putTopRefresh(main);
      }
    }
  }
}

void UserInterface::runTickerInstance() {
  int i = 0;
  while(1) {
    usleep(TICKLENGTH);
    if (tickerenabled) {
      sem_wait(&event_ready);
      eventtext = "update";
      sem_post(&event);
    }
    if (legendenabled) {
      sem_wait(&event_ready);
      eventtext = "updatelegend";
      sem_post(&event);
    }
  }
}

void * UserInterface::runKeyListener(void * arg) {
  ((UserInterface *) arg)->runKeyListenerInstance();
}

void * UserInterface::runUserInterface(void * arg) {
  ((UserInterface *) arg)->runUserInterfaceInstance();
}

void * UserInterface::runTicker(void * arg) {
  ((UserInterface *) arg)->runTickerInstance();
}
