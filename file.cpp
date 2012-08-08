#include "file.h"

File::File(std::string name, std::string user) {
  directory = false;
  owner = user;
  group = user;
  size = 0;
  lastmodified = "0";
  this->name = name;
  touch = 0;
  updateflag = false;
}

File::File(std::string statline, int touch) {
  int start, pos = 0;
  if (statline[pos] == 'd') directory = true;
  else directory = false;
  while(statline[++pos] != ' ');
  while(statline[++pos] == ' ');
  while(statline[++pos] != ' ');
  while(statline[++pos] == ' ');
  start = pos;
  while (statline[++pos] != ' ');
  owner = statline.substr(start, pos - start);
  while (statline[++pos] == ' ');
  start = pos;
  while (statline[++pos] != ' ');
  group = statline.substr(start, pos - start);
  while (statline[++pos] == ' ');
  start = pos;
  while (statline[++pos] != ' ');
  std::string sizetmp = statline.substr(start, pos - start);
  size = atol(sizetmp.c_str());
  while (statline[++pos] == ' ');
  start = pos;
  pos += 12;
  lastmodified = statline.substr(start, 12);
  while (statline[++pos] == ' ');
  start = pos;
  while (statline[++pos] != '\r');
  name = statline.substr(start, pos - start);
  int suffixdotpos = name.rfind(".");
  if (suffixdotpos > 0) {
    extension = name.substr(suffixdotpos + 1);
  }
  else {
    extension = "";
  }
  this->touch = touch;
  updateflag = false;
}

bool File::isDirectory() {
  return directory;
}

std::string File::getOwner() {
  return owner;
}

std::string File::getGroup() {
  return group;
}

long int File::getSize() {
  return size;
}

std::string File::getLastModified() {
  return lastmodified;
}

std::string File::getName() {
  return name;
}

std::string File::getExtension() {
  return extension;
}

int File::getCurrentSpeed() {
  return 1024;
}

bool File::updateFlagSet() {
  return updateflag;
}

int File::getUpdateSpeed() {
  return updatespeed;
}

Site * File::getUpdateSrc() {
  return updatesrc;
}

std::string File::getUpdateDst() {
  return updatedst;
}

void File::setUpdateFlag(Site * src, std::string dst, int speed) {
  updatesrc = src;
  updatedst = dst;
  updatespeed = speed;
  updateflag = true;
}

void File::unsetUpdateFlag() {
  updateflag = false;
}

void File::setSize(long int size) {
  this->size = size;
}

void File::setLastModified(std::string lastmodified) {
  this->lastmodified = lastmodified;
}

void File::setOwner(std::string owner) {
  this->owner = owner;
}

void File::setGroup(std::string group) {
  this->group = group;
}

void File::setTouch(int touch) {
  this->touch = touch;
}

int File::getTouch() {
  return touch;
}
