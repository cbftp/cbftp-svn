#pragma once

#include <string>

class DelayedCommand {
private:
  std::string command;
  unsigned long long int triggertime;
  void * arg;
  bool active;
  bool released;
  bool persisting;
public:
  DelayedCommand();
  void set(std::string, unsigned long long int, void *, bool);
  void currentTime(unsigned long long int);
  void reset();
  void weakReset();
  std::string getCommand() const;
  void * getArg() const;
  bool isActive() const;
  bool isReleased() const;
  bool isPersisting() const;
};
