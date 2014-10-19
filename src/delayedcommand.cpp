#include "delayedcommand.h"

DelayedCommand::DelayedCommand() :
  active(false),
  released(false) {
}

void DelayedCommand::set(std::string command, unsigned long long int triggertime) {
  set(command, triggertime, NULL);
}

void DelayedCommand::set(std::string command, unsigned long long int triggertime, void * arg) {
  this->command = command;
  this->triggertime = triggertime;
  this->arg = arg;
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
