#include "file.h"

#include <stdlib.h>
#include <cctype>

File::File(std::string name, std::string user) {
  directory = false;
  softlink = false;
  linktarget = "";
  owner = user;
  group = user;
  size = 0;
  lastmodified = "0";
  this->name = name;
  extension = getExtension(name);
  touch = 0;
  updateflag = false;
  downloading = 0;
  uploading = false;
}

File::File(std::string statline, int touch) {
  int start, pos = 0;
  softlink = false;
  directory = false;
  if (statline[pos] == 'd') directory = true;
  else if (statline[pos] == 'l') softlink = true;
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
  if (softlink) {
    size_t arrowpos = name.find(" -> ");
    if (arrowpos != std::string::npos) {
      linktarget = name.substr(arrowpos + 4);
      name = name.substr(0, arrowpos);
    }
    else {
      linktarget = "";
    }
  }
  extension = getExtension(name);
  this->touch = touch;
  updateflag = false;
  downloading = 0;
  uploading = false;
}

bool File::isDirectory() {
  return directory;
}

bool File::isLink() {
  return softlink;
}

std::string File::getOwner() {
  return owner;
}

std::string File::getGroup() {
  return group;
}

unsigned long long int File::getSize() {
  return size;
}

std::string File::getLastModified() {
  return lastmodified;
}

std::string File::getName() {
  return name;
}

std::string File::getLinkTarget() {
  return linktarget;
}

std::string File::getExtension() {
  return extension;
}

unsigned int File::getCurrentSpeed() {
  return 1024;
}

bool File::updateFlagSet() {
  return updateflag;
}

unsigned int File::getUpdateSpeed() {
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

bool File::setSize(unsigned long long int size) {
  bool changed = this->size != size;
  this->size = size;
  return changed;
}

bool File::setLastModified(std::string lastmodified) {
  bool changed = this->lastmodified.compare(lastmodified) != 0;
  this->lastmodified = lastmodified;
  return changed;
}

bool File::setOwner(std::string owner) {
  bool changed = this->owner.compare(owner) != 0;
  this->owner = owner;
  return changed;
}

bool File::setGroup(std::string group) {
  bool changed = this->group.compare(group) != 0;
  this->group = group;
  return changed;
}

void File::setTouch(int touch) {
  this->touch = touch;
}

void File::download() {
  downloading++;
}

bool File::isDownloading() {
  return downloading > 0;
}

void File::finishDownload() {
  downloading--;
}

void File::upload() {
  uploading = true;
}

bool File::isUploading() {
  return uploading;
}

void File::finishUpload() {
  uploading = false;
}

int File::getTouch() {
  return touch;
}

std::string File::getExtension(std::string file) {
  std::string extension;
  size_t suffixdotpos = file.rfind(".");
  if (suffixdotpos != std::string::npos && suffixdotpos > 0) {
    extension = file.substr(suffixdotpos + 1);
  }
  for (unsigned int i = 0; i < extension.length(); i++) {
    extension[i] = tolower(extension[i]);
  }
  return extension;
}
