#include "uicommunicator.h"

#include <ncurses.h>
#include <vector>

#include "../globalcontext.h"
#include "../datafilehandler.h"

extern GlobalContext * global;

UICommunicator::UICommunicator() {
  sem_init(&eventsem, 0, 0);
  pthread_mutex_init(&event_mutex, NULL);
  newcommand = false;
  careaboutbackend = false;
  legendenabled = true;
  died = false;
}

void UICommunicator::newCommand(std::string command) {
  newCommand(command, "", "", "");
}

void UICommunicator::newCommand(std::string command, std::string arg1) {
  newCommand(command, arg1, "", "");
}

void UICommunicator::newCommand(std::string command, std::string arg1, std::string arg2) {
  newCommand(command, arg1, arg2, "");
}

void UICommunicator::newCommand(std::string command, std::string arg1, std::string arg2, std::string arg3) {
  this->command = command;
  this->arg1 = arg1;
  this->arg2 = arg2;
  this->arg3 = arg3;
  newcommand = true;
}
void UICommunicator::expectBackendPush() {
  careaboutbackend = true;
}

void UICommunicator::backendPush() {
  if (careaboutbackend) {
    emitEvent(UI_EVENT_UPDATE);
  }
}

void UICommunicator::windowChanged() {
  careaboutbackend = false;
}

void UICommunicator::checkoutCommand() {
  newcommand = false;
}

bool UICommunicator::hasNewCommand() {
  return newcommand;
}

std::string UICommunicator::getCommand() {
  return command;
}

std::string UICommunicator::getArg1() {
  return arg1;
}

std::string UICommunicator::getArg2() {
  return arg2;
}

std::string UICommunicator::getArg3() {
  return arg3;
}

sem_t * UICommunicator::getEventSem() {
  return &eventsem;
}

void UICommunicator::emitEvent(int event) {
  pthread_mutex_lock(&event_mutex);
  eventqueue.push_back(event);
  pthread_mutex_unlock(&event_mutex);
  sem_post(&eventsem);
}

int UICommunicator::awaitEvent() {
  int event;
  sem_wait(&eventsem);
  pthread_mutex_lock(&event_mutex);
  event = eventqueue.front();
  eventqueue.pop_front();
  pthread_mutex_unlock(&event_mutex);
  return event;
}

void UICommunicator::kill() {
  emitEvent(UI_EVENT_KILL);
  for (int i = 0; i < 10; i++) {
    usleep(100000);
    if (died) {
      break;
    }
  }
  if (!died) {
    endwin();
  }
}

void UICommunicator::dead() {
  died = true;
}

void UICommunicator::terminalSizeChanged() {
  emitEvent(UI_EVENT_RESIZE);
}

bool UICommunicator::legendEnabled() {
  return legendenabled;
}

void UICommunicator::showLegend(bool show) {
  if (show) {
    emitEvent(UI_EVENT_SHOWLEGEND);
  }
  else {
    emitEvent(UI_EVENT_HIDELEGEND);
  }
  legendenabled = show;
}

void UICommunicator::readConfiguration() {
  std::vector<std::string> lines;
  global->getDataFileHandler()->getDataFor("UICommunicator", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("legend")) {
      if (!value.compare("true")) {
        showLegend(true);
      }
      else {
        showLegend(false);
      }
    }
  }
}

void UICommunicator::writeState() {
  if (legendEnabled()) {
    global->getDataFileHandler()->addOutputLine("UICommunicator", "legend=true");
  }
  else {
    global->getDataFileHandler()->addOutputLine("UICommunicator", "legend=false");
  }
}
