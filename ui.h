#pragma once
#include <iostream>
#include <string>
#include <list>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>
#include <map>

#include "globalcontext.h"
#include "sitemanager.h"
#include "site.h"
#include "sitethread.h"
#include "sitethreadmanager.h"
#include "menuselectsite.h"
#include "menuselectoption.h"

extern GlobalContext * global;

class UserInterface {
  private:
    WINDOW * loginscreen;
    WINDOW * mainscreen;
    WINDOW * editsitescreen;
    WINDOW * confirmationscreen;
    WINDOW * front;
    WINDOW * rawdatascreen;
    std::map<std::string, WINDOW *> sitestatusscreen;
    int col;
    int row;
    int updateinterval;
    bool initret;
    bool tickerrun;
    pthread_t thread;
    sem_t initstart;
    sem_t initdone;
    sem_t update;
    sem_t tickeractivate;
    void putTopRefresh(WINDOW *);
    void loginScreen();
    void mainScreen();
    int editSiteScreen(Site *);
    void siteStatusScreen(Site *);
    int confirmationScreen();
    int rawDataScreen(FTPThread *);
    std::string getStringField(WINDOW *, int, int, std::string, int, int, bool);
    int getNumArrow(WINDOW *, int, int, int);
    bool getCheckBoxBool(WINDOW *, int, int, int);
    void initIntern();
    void startTicker(int);
    void stopTicker();
    static void * runKeyListener(void *);
    static void * runUpdater(void *);
    static void * runTicker(void *);
  public:
    UserInterface();
    void runKeyListenerInstance();
    void runUpdaterInstance();
    void runTickerInstance();
    bool init();
    void kill();
};

