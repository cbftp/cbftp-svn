#pragma once

#include <string>
#include <list>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>
#include <map>
#include <sys/select.h>

#include "globalcontext.h"
#include "sitemanager.h"
#include "site.h"
#include "sitethread.h"
#include "sitethreadmanager.h"
#include "uiwindow.h"
#include "uiwindowcommand.h"
#include "loginscreen.h"
#include "mainscreen.h"
#include "editsitescreen.h"
#include "confirmationscreen.h"
#include "sitestatusscreen.h"
#include "rawdatascreen.h"

extern GlobalContext * global;

class UserInterface {
  private:
    WINDOW * main;
    UIWindow * topwindow;
    int col;
    int row;
    int updateinterval;
    bool initret;
    bool tickerrun;
    std::string eventtext;
    pthread_t thread[3];
    sem_t initstart;
    sem_t initdone;
    sem_t update;
    sem_t event;
    sem_t event_ready;
    sem_t tickeractivate;
    UIWindowCommand windowcommand;
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
    static void * runUserInterface(void *);
    static void * runTicker(void *);
  public:
    UserInterface();
    void runKeyListenerInstance();
    void runUserInterfaceInstance();
    void runTickerInstance();
    bool init();
    void kill();
    void updateMain();
};

