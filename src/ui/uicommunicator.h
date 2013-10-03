#pragma once

#include <string>
#include <semaphore.h>
#include <list>

#define UI_EVENT_KILL 1450
#define UI_EVENT_RESIZE 1451
#define UI_EVENT_UPDATE 1452
#define UI_EVENT_KEYBOARD 1453
#define UI_EVENT_POKE 1454
#define UI_EVENT_SHOWLEGEND 1455
#define UI_EVENT_HIDELEGEND 1456

class UICommunicator {
private:
  bool newcommand;
  std::string command;
  std::string arg1;
  std::string arg2;
  std::string arg3;
  bool careaboutbackend;
  std::list<int> eventqueue;
  pthread_mutex_t event_mutex;
  sem_t eventsem;
  bool legendenabled;
  bool died;
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
  void emitEvent(int);
  int awaitEvent();
  void kill();
  void dead();
  void terminalSizeChanged();
  bool legendEnabled();
  void showLegend(bool);
  void readConfiguration();
  void writeState();
};
