#include "uicommunicator.h"

UICommunicator::UICommunicator() {
  sem_init(&event, 0, 0);
  sem_init(&event_ready, 0, 1);
  newcommand = false;
  careaboutbackend = false;
}

void UICommunicator::newCommand(std::string command) {
  newCommand(command, "", "");
}

void UICommunicator::newCommand(std::string command, std::string arg1) {
  newCommand(command, arg1, "");
}

void UICommunicator::newCommand(std::string command, std::string arg1, std::string arg2) {
  this->command = command;
  this->arg1 = arg1;
  this->arg2 = arg2;
  newcommand = true;
}

void UICommunicator::expectBackendPush() {
  careaboutbackend = true;
}

void UICommunicator::backendPush() {
  if (careaboutbackend) {
    emitEvent("update");
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

void UICommunicator::emitEvent(std::string eventtext) {
  sem_wait(&event_ready);
  this->eventtext = eventtext;
  sem_post(&event);
}

std::string UICommunicator::awaitEvent() {
  std::string eventtext;
  sem_wait(&event);
  eventtext = this->eventtext;
  sem_post(&event_ready);
  return eventtext;
}

void UICommunicator::kill() {
  endwin();
}

void UICommunicator::terminalSizeChanged() {
  emitEvent("resize");
}
