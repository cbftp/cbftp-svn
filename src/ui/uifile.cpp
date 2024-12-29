#include "uifile.h"

#include <cctype>

#include "../file.h"
#include "../localfile.h"
#include "../util.h"
#include "../timereference.h"

UIFile::UIFile(File* file) :
  name(file->getName()),
  size(file->getSize()),
  sizerepr(util::parseSize(size)),
  owner(file->getOwner()),
  group(file->getGroup()),
  linktarget(file->getLinkTarget()),
  directory(file->isDirectory()),
  softlink(file->isLink()),
  softselected(false),
  hardselected(false)
{
  setLastModified(timestamps::parseTimestamp(file->getLastModified()));
}

UIFile::UIFile(const LocalFile& file) :
  name(file.getName()),
  size(file.getSize()),
  sizerepr(util::parseSize(size)),
  owner(file.getOwner()),
  group(file.getGroup()),
  linktarget(file.getLinkTarget()),
  directory(file.isDirectory()),
  softlink(file.isLink()),
  softselected(false),
  hardselected(false)
{
  setLastModified(timestamps::Timestamp(file.getYear(), file.getMonth(), file.getDay(), file.getHour(), file.getMinute()));
}

std::string UIFile::getName() const {
  return name;
}

std::string UIFile::getLinkTarget() const {
  return linktarget;
}

std::string UIFile::getOwner() const {
  return owner;
}

std::string UIFile::getGroup() const {
  return group;
}

std::string UIFile::getLastModified() const {
  return lastmodifiedrepr;
}

int UIFile::getModifyTime() const {
  return lastmodified;
}

int UIFile::getModifyDate() const {
  return lastmodifieddate;
}

unsigned long long int UIFile::getSize() const {
  return size;
}

std::string UIFile::getSizeRepr() const {
  return sizerepr;
}

bool UIFile::isDirectory() const {
  return directory;
}

bool UIFile::isLink() const {
  return softlink;
}

bool UIFile::isSoftSelected() const {
  return softselected;
}

bool UIFile::isHardSelected() const {
  return hardselected;
}

void UIFile::softSelect() {
  softselected = true;
}

void UIFile::hardSelect() {
  hardselected = true;
  softselected = false;
}

void UIFile::unSoftSelect() {
  softselected = false;
}

void UIFile::unHardSelect() {
  hardselected = false;
}

void UIFile::setLastModified(const timestamps::Timestamp& timestamp) {
  // somewhat incorrect formula, but since the exact stamp will only be used for sorting,
  // there's no need to bother
  lastmodifieddate = ((timestamp.year - 1970) * 372 * 24 * 60) +
                      (timestamp.month * 31 * 24 * 60) +
                      (timestamp.day * 24 * 60);
  lastmodified = lastmodifieddate +
                   (timestamp.hour * 60) +
                   timestamp.minute;
  lastmodifiedrepr = timestamp.toString();
}
