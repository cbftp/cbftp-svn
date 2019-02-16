#pragma once

#include <string>

class LocalFile {
public:
  LocalFile(std::string, unsigned long long int, bool, std::string, std::string, int, int, int, int, int);
  std::string getName() const;
  unsigned long long int getSize() const;
  bool isDirectory() const;
  std::string getOwner() const;
  std::string getGroup() const;
  int getYear() const;
  int getMonth() const;
  int getDay() const;
  int getHour() const;
  int getMinute() const;
  void setSize(unsigned long long int size);
private:
  std::string name;
  unsigned long long int size;
  bool isdir;
  std::string owner;
  std::string group;
  int year;
  int month;
  int day;
  int hour;
  int minute;
};
