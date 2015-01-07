#include "delayedcommand.h"

DelayedCommand::DelayedCommand() :
  active(false),
  released(false),
  persisting(false) {
}

void DelayedCommand::set(std::string command, unsigned long long int triggertime, void * arg, bool persisting) {
  this->command = command;
  this->triggertime = triggertime;
  this->arg = arg;
  this->persisting = persisting;
  this->active = true;
  this->released = false;
}

void DelayedCommand::currentTime(unsigned long long int time) {
  if (active && !released && time >= triggertime) {
    released = true;
  }
}

void DelayedCommand::reset() {
  active = false;
  released = false;
}

void DelayedCommand::weakReset() {
  if (!persisting) {
    reset();
  }
}

std::string DelayedCommand::getCommand() const {
  return command;
}

void * DelayedCommand::getArg() const {
  return arg;
}

bool DelayedCommand::isActive() const {
  return active;
}

bool DelayedCommand::isReleased() const {
  return released;
}
