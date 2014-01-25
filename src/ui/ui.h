#pragma once

#include <string>
#include <list>
#include <vector>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>

#include "uicommunicator.h"
#include "../eventreceiver.h"

#define TICKLENGTH 250000

class UIWindow;
class InfoWindow;
class LegendWindow;

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
    std::list<int> keyqueue;
    void FDData();
    void refreshAll();
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
    static void * run(void *);
    void tick(int);
    void globalKeyBinds(int);
  public:
    UserInterface();
    void runInstance();
    bool init();
    UICommunicator * getCommunicator();
};

