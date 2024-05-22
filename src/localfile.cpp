#include "localfile.h"

LocalFile::LocalFile(const std::string& name, unsigned long long int size, LocalFileType type, const std::string& owner, const std::string& group, int year, int month, int day, int hour, int minute, const std::string& linktarget, bool download) :
  name(name),
  size(size),
  type(type),
  owner(owner),
  group(group),
  linktarget(linktarget),
  year(year),
  month(month),
  day(day),
  hour(hour),
  minute(minute),
  touch(0),
  download(download)
{
}

std::string LocalFile::getName() const {
  return name;
}

unsigned long long int LocalFile::getSize() const {
  return size;
}

bool LocalFile::isFile() const {
  return type == LocalFileType::FILE;
}

bool LocalFile::isDirectory() const {
  return type == LocalFileType::DIR;
}

bool LocalFile::isLink() const {
  return type == LocalFileType::LINK;
}

std::string LocalFile::getOwner() const {
  return owner;
}

std::string LocalFile::getGroup() const {
  return group;
}

std::string LocalFile::getLinkTarget() const {
  return linktarget;
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

void LocalFile::setSize(unsigned long long int size) {
  this->size = size;
}

int LocalFile::getTouch() const {
  return touch;
}

void LocalFile::setTouch(int touch) {
  this->touch = touch;
}

bool LocalFile::isDownloading() const {
  return download;
}

void LocalFile::setDownloading() {
  download = true;
}

void LocalFile::finishDownload() {
  download = false;
}
