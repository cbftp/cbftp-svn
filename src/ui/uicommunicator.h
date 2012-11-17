#pragma once

#include <map>
#include <string>
#include <semaphore.h>
#include <ncurses.h>

class UICommunicator {
private:
  bool newcommand;
  std::string command;
  std::string arg1;
  std::string arg2;
  std::string arg3;
  bool careaboutbackend;
  std::string eventtext;
  sem_t event;
  sem_t event_ready;
public:
  UICommunicator();
  void newCommand(std::string);
  void newCommand(std::string, std::string);
  void newCommand(std::string, std::string, std::string);
  void newCommand(std::string, std::string, std::string, std::string);
  void expectBackendPush();
  void backendPush();
  void windowChanged();
  void checkoutCommand();
  bool hasNewCommand();
  std::string getCommand();
  std::string getArg1();
  std::string getArg2();
  std::string getArg3();
  sem_t * getEventSem();
  void emitEvent(std::string);
  std::string awaitEvent();
  void kill();
  void terminalSizeChanged();
};
