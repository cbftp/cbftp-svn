#include "delayedcommand.h"

DelayedCommand::DelayedCommand(std::string command, int delay) {
  this->command = command;
  this->delay = delay;
}

std::string DelayedCommand::getCommand() {
  return command;
}

int DelayedCommand::getDelay() {
  return delay;
}
