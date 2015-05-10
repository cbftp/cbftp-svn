#include "localfile.h"

LocalFile::LocalFile(std::string name, unsigned long long int size, bool isdir, std::string owner, std::string group, int year, int month, int day, int hour, int minute) :
  name(name),
  size(size),
  isdir(isdir),
  owner(owner),
  group(group),
  year(year),
  month(month),
  day(day),
  hour(hour),
  minute(minute) {
}

std::string LocalFile::getName() const {
  return name;
}

unsigned long long int LocalFile::getSize() const {
  return size;
}

bool LocalFile::isDirectory() const {
  return isdir;
}

std::string LocalFile::getOwner() const {
  return owner;
}

std::string LocalFile::getGroup() const {
  return group;
}

int LocalFile::getYear() const {
  return year;
}

int LocalFile::getMonth() const {
  return month;
}

int LocalFile::getDay() const {
  return day;
}

int LocalFile::getHour() const {
  return hour;
}

int LocalFile::getMinute() const {
  return minute;
}
