#pragma once

#include <string>
#include <list>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>
#include <map>
#include <iostream>
#include <istream>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "../globalcontext.h"
#include "../tickpoke.h"
#include "../eventreceiver.h"

#include "legendwindow.h"
#include "infowindow.h"
#include "uiwindow.h"
#include "uicommunicator.h"

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

#define TICKLENGTH 250000

extern GlobalContext * global;

class UserInterface : private EventReceiver {
  private:
    WINDOW * main;
    WINDOW * info;
    WINDOW * legend;
    std::vector<UIWindow *> mainwindows;
    UIWindow * topwindow;
    InfoWindow * infowindow;
    LegendWindow * legendwindow;
    int mainrow;
    int maincol;
    int col;
    int row;
    int updateinterval;
    bool initret;
    bool tickerenabled;
    bool legendenabled;
    bool infoenabled;
    std::string eventtext;
    pthread_t uithread;
    pthread_t klthread;
    sem_t initstart;
    sem_t initdone;
    sem_t keyeventdone;
    UICommunicator uicommunicator;
    std::list<UIWindow *> history;
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
    void enableInfo();
    void disableInfo();
    void enableLegend();
    void disableLegend();
    void redrawAll();
    void switchToWindow(UIWindow *);
    static void * runKeyListener(void *);
    static void * runUserInterface(void *);
    void tick(int);
  public:
    UserInterface();
    void runKeyListenerInstance();
    void runUserInterfaceInstance();
    bool init();
    UICommunicator * getCommunicator();
};

