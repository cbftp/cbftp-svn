#pragma once

#include <string>
#include <list>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>
#include <map>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "../globalcontext.h"
#include "../tickpoke.h"

#include "uiwindow.h"
#include "uicommunicator.h"
#include "loginscreen.h"
#include "newkeyscreen.h"
#include "mainscreen.h"
#include "editsitescreen.h"
#include "confirmationscreen.h"
#include "sitestatusscreen.h"
#include "rawdatascreen.h"
#include "legendwindow.h"
#include "browsescreen.h"

#define TICKLENGTH 250000

extern GlobalContext * global;

class UserInterface {
  private:
    WINDOW * main;
    WINDOW * legend;
    std::vector<UIWindow *> mainwindows;
    UIWindow * topwindow;
    LegendWindow * legendwindow;
    int mainrow;
    int maincol;
    int col;
    int row;
    int updateinterval;
    bool initret;
    bool tickerenabled;
    bool legendenabled;
    std::string eventtext;
    pthread_t thread[2];
    sem_t initstart;
    sem_t initdone;
    sem_t keyeventdone;
    UICommunicator uicommunicator;
    void refreshAll();
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
    void enableLegend();
    void disableLegend();
    void redrawAll();
    static void * runKeyListener(void *);
    static void * runUserInterface(void *);
  public:
    UserInterface();
    void runKeyListenerInstance();
    void runUserInterfaceInstance();
    bool init();
    UICommunicator * getCommunicator();
};

