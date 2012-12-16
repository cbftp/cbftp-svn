#include "delayedcommand.h"

DelayedCommand::DelayedCommand(std::string command, int delay) {
  this->command = command;
  this->delay = delay;
  this->arg = NULL;
}

DelayedCommand::DelayedCommand(std::string command, int delay, void * arg) {
  this->command = command;
  this->delay = delay;
  this->arg = arg;
}

DelayedCommand::DelayedCommand(const DelayedCommand & delayedcommand) {
  this->command = delayedcommand.command;
  this->delay = delayedcommand.delay;
  this->arg = delayedcommand.arg;
}

std::string DelayedCommand::getCommand() {
  return command;
}

int DelayedCommand::getDelay() {
  return delay;
}

void * DelayedCommand::getArg() {
  return arg;
}
