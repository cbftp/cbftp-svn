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

extern GlobalContext * global;

class UserInterface {
  private:
    WINDOW * loginscreen;
    WINDOW * mainscreen;
    WINDOW * editsitescreen;
    WINDOW * confirmationscreen;
    WINDOW * front;
    std::map<std::string, WINDOW *> sitestatusscreen;
    int col;
    int row;
    bool initret;
    pthread_t thread;
    sem_t action;
    sem_t initdone;
    void putTopRefresh(WINDOW *);
    void loginScreen();
    void mainScreen();
    int editSiteScreen(Site *);
    void siteStatusScreen(Site *);
    int confirmationScreen();
    std::string getStringField(WINDOW *, int, int, std::string, int, int, bool);
    void initIntern();
    static void * run(void *);
  public:
    UserInterface();
    void runInstance();
    bool init();
    void kill();
};

