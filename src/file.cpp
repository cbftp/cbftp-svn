#include "file.h"

#include <stdlib.h>
#include <cctype>

#include "util.h"

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

  softlink = false;
  directory = false;
  if (statline[0] == 'd') directory = true;
  else if (statline[0] == 'l') softlink = true;
  owner = util::trim(statline.substr(15, 8)); // field with fixed pos and len
  group = util::trim(statline.substr(24, 8)); // field with fixed pos and len
  int pos = 32, start = pos;
  while (statline[++pos] == ' '); // size start at pos
  start = pos;
  while (statline[++pos] != ' '); // size end at pos - 1
  std::string sizetmp = statline.substr(start, pos - start);
  size = atol(sizetmp.c_str());
  while (statline[++pos] == ' '); // month start at pos
  start = pos;
  while (statline[++pos] != ' '); // month end at pos - 1
  while (statline[++pos] == ' '); // day start at pos
  while (statline[++pos] != ' '); // day end at pos - 1
  while (statline[++pos] == ' '); // time/year start at pos
  while (statline[++pos] != ' '); // time/year end at pos - 1
  lastmodified = statline.substr(start, pos - start);
  while (statline[++pos] == ' '); // name start at pos
  start = pos;
  while (statline[++pos] != '\r'); // name end at pos - 1
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

bool File::isDirectory() const {
  return directory;
}

bool File::isLink() const {
  return softlink;
}

std::string File::getOwner() const {
  return owner;
}

std::string File::getGroup() const {
  return group;
}

unsigned long long int File::getSize() const {
  return size;
}

std::string File::getLastModified() const {
  return lastmodified;
}

std::string File::getName() const {
  return name;
}

std::string File::getLinkTarget() const {
  return linktarget;
}

std::string File::getExtension() const {
  return extension;
}

bool File::updateFlagSet() const {
  return updateflag;
}

unsigned int File::getUpdateSpeed() const {
  return updatespeed;
}

Site * File::getUpdateSrc() const {
  return updatesrc;
}

std::string File::getUpdateDst() const {
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

bool File::isDownloading() const {
  return downloading > 0;
}

void File::finishDownload() {
  downloading--;
}

void File::upload() {
  uploading = true;
}

bool File::isUploading() const {
  return uploading;
}

void File::finishUpload() {
  uploading = false;
}

int File::getTouch() const {
  return touch;
}

std::string File::getExtension(std::string file) const {
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
