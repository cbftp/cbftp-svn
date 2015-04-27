#include "file.h"

#include <stdlib.h>
#include <cctype>

File::File(std::string name, std::string user) :
  name(name),
  extension(getExtension(name)),
  size(0),
  owner(user),
  group(user),
  lastmodified("0"),
  updatespeed(0),
  updatesrc(NULL),
  updateflag(false),
  directory(false),
  softlink(false),
  touch(0),
  uploading(false),
  downloading(0)
{
}

File::File(const std::string & statline, int touch) :
  updateflag(false),
  directory(false),
  softlink(false),
  touch(touch),
  uploading(false),
  downloading(0)
{
  if (isdigit(statline[0]) && isdigit(statline[1])) {
    parseMSDOSSTATLine(statline);
  }
  else {
    parseUNIXSTATLine(statline);
  }
  extension = getExtension(name);
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

}

void File::parseUNIXSTATLine(const std::string & statline) {
  int start, pos = 0;
  if (statline[pos] == 'd') directory = true;
  else if (statline[pos] == 'l') softlink = true;
  while (statline[++pos] != ' ');
  while (statline[++pos] == ' ');
  while (statline[++pos] != ' ');
  int spacestart = pos;
  while (statline[++pos] == ' '); // user start at pos? possibly missing field
  if (pos - spacestart < 9) {
    start = pos;
    while (statline[++pos] != ' ');
    owner = statline.substr(start, pos - start);
  }
  else {
    owner = "";
    pos = spacestart + 9;
  }
  spacestart = pos;
  while (statline[++pos] == ' '); // group start at pos? possibly missing field
  if (pos - spacestart < 9) {
    start = pos;
    while (statline[++pos] != ' ');
    group = statline.substr(start, pos - start);
  }
  else {
    group = "";
    pos = spacestart + 9;
  }
  while (statline[++pos] == ' '); // size start at pos
  start = pos;
  while (statline[++pos] != ' ');
  std::string sizetmp = statline.substr(start, pos - start);
  size = atol(sizetmp.c_str());
  while (statline[++pos] == ' '); // month start at pos
  start = pos;
  while (statline[++pos] != ' ');
  while (statline[++pos] == ' '); // day start at pos
  while (statline[++pos] != ' ');
  while (statline[++pos] == ' '); // time/year start at pos
  while (statline[++pos] != ' ');
  lastmodified = statline.substr(start, pos - start);
  while (statline[++pos] == ' '); // name start at pos
  start = pos;
  while (statline[++pos] != '\r');
  name = statline.substr(start, pos - start);
}

void File::parseMSDOSSTATLine(const std::string & statline) {
  int start = 0, pos = 0; // date start at pos
  while (statline[++pos] != ' ');
  while (statline[++pos] == ' '); // time start at pos
  while (statline[++pos] != ' ');
  lastmodified = statline.substr(start, pos - start);
  while (statline[++pos] == ' '); // dirtag or size start at pos
  start = pos;
  while (statline[++pos] != ' ');
  std::string dirorsize = statline.substr(start, pos - start);
  if (dirorsize == "<DIR>") {
    directory = true;
    size = 0;
  }
  else {
    size = atol(digitsOnly(dirorsize).c_str());
  }
  while (statline[++pos] == ' '); // name start at pos
  start = pos;
  while (statline[++pos] != '\r');
  name = statline.substr(start, pos - start);
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

bool File::setLastModified(const std::string & lastmodified) {
  bool changed = this->lastmodified.compare(lastmodified) != 0;
  this->lastmodified = lastmodified;
  return changed;
}

bool File::setOwner(const std::string & owner) {
  bool changed = this->owner.compare(owner) != 0;
  this->owner = owner;
  return changed;
}

bool File::setGroup(const std::string & group) {
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

std::string File::getExtension(const std::string & file) const {
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

std::string File::digitsOnly(const std::string & input) const {
  std::string output;
  for (unsigned int i = 0; i < input.length(); i++) {
    if (isdigit(input[i])) {
      output += input[i];
    }
  }
  return output;
}
