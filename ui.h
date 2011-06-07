#pragma once
#include <iostream>
#include <string>
#include <list>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>

#include "globalcontext.h"

extern GlobalContext * global;

class UserInterface {
  private:
    WINDOW * loginscreen;
    WINDOW * front;
    int col;
    int row;
    void refreshFront();
    void showLoginScreen();
    std::string getStringField(WINDOW *, int, int, std::string, int, int, bool);
  public:
    UserInterface();
    bool init();
    void kill();
};
