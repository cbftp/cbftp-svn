#include "connstatetracker.h"
#include <iostream>

ConnStateTracker::ConnStateTracker() {
  state = 0;
  time = 0;
  lastchecked = NULL;
  lastcheckedcount = 0;
}

void ConnStateTracker::delayedCommand(std::string command, int delay) {
  delayedcommands.push_back(DelayedCommand(command, delay + time));
}

void ConnStateTracker::delayedCommand(std::string command, int delay, void * arg) {
  delayedcommands.push_back(DelayedCommand(command, delay + time, arg));
}

void ConnStateTracker::timePassed(int time) {
  this->time += time;
  std::list<DelayedCommand>::iterator it;
  bool found;
  while (true) {
    found = false;
    for (it = delayedcommands.begin(); it != delayedcommands.end(); it++) {
      if (this->time >= it->getDelay()) {
        releasedcommands.push_back(*it);
        delayedcommands.erase(it);
        found = true;
        break;
      }
    }
    if (!found) {
      break;
    }
  }
}

int ConnStateTracker::getTimePassed() {
  return time;
}

void ConnStateTracker::check(SiteRace * sr) {
  if (lastchecked == sr) {
    lastcheckedcount++;
  }
  else {
    lastchecked = sr;
    lastcheckedcount = 1;
  }
}

SiteRace * ConnStateTracker::lastChecked() {
  return lastchecked;
}

int ConnStateTracker::checkCount() {
    return lastcheckedcount;
}

bool ConnStateTracker::hasReleasedCommand() {
  return releasedcommands.size() > 0;
}

DelayedCommand ConnStateTracker::getCommand() {
  if (releasedcommands.size() > 0) {
    DelayedCommand command = releasedcommands.front();
    releasedcommands.pop_front();
    return command;
  }
  //undefined behavior
  return DelayedCommand("error", 0);
}

void ConnStateTracker::setDisconnected() {
  delayedcommands.clear();
  time = 0;
  state = 0;
}

void ConnStateTracker::setIdle() {
  delayedcommands.clear();
  time = 0;
  state = 1;
}

void ConnStateTracker::setReady() {
  delayedcommands.clear();
  time = 0;
  state = 2;
}

void ConnStateTracker::setBusy() {
  delayedcommands.clear();
  time = 0;
  state = 3;
}

bool ConnStateTracker::isDisconnected() {
  return state == 0;
}

bool ConnStateTracker::isIdle() {
  return state == 1;
}

bool ConnStateTracker::isReady() {
  return (state == 1 || state == 2);
}
