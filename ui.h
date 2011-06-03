#pragma once
#include <string>
#include <list>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>

class UserInterface {
  private:
    WINDOW * loginscreen;
    int col;
    int row;
  public:
    UserInterface();
    bool init();
    void kill();
};
