#include "uifile.h"

#include "../file.h"
#include "../globalcontext.h"
#include "../util.h"

extern GlobalContext * global;

UIFile::UIFile(File * file) {
  name = file->getName();
  directory = file->isDirectory();
  softlink = file->isLink();
  owner = file->getOwner();
  group = file->getGroup();
  size = file->getSize();
  linktarget = file->getLinkTarget();
  sizerepr = util::parseSize(file->getSize());
  parseTimeStamp(file->getLastModified());
  selected = false;
  cursored = false;
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

bool UIFile::isSelected() const {
  return selected;
}

bool UIFile::isCursored() const {
  return cursored;
}

void UIFile::select() {
  selected = true;
}

void UIFile::unSelect() {
  selected = false;
}

void UIFile::cursor() {
  cursored = true;
}

void UIFile::unCursor() {
  cursored = false;
}

void UIFile::parseTimeStamp(std::string uglytime) {
  int pos = 0; // month start at 0
  while (uglytime[++pos] != ' ');
  std::string monthstr = uglytime.substr(0, pos);
  while (uglytime[++pos] == ' '); // day start at pos
  int start = pos;
  while (uglytime[++pos] != ' ');
  std::string daystr = uglytime.substr(start, pos - start);
  while (uglytime[++pos] == ' '); // meta start at pos
  std::string meta = uglytime.substr(pos);
  int year;
  int month = 0;
  int day = util::str2Int(daystr);
  int hour;
  int minute;
  if (monthstr == "Jan") month = 1;
  else if (monthstr == "Feb") month = 2;
  else if (monthstr == "Mar") month = 3;
  else if (monthstr == "Apr") month = 4;
  else if (monthstr == "May") month = 5;
  else if (monthstr == "Jun") month = 6;
  else if (monthstr == "Jul") month = 7;
  else if (monthstr == "Aug") month = 8;
  else if (monthstr == "Sep") month = 9;
  else if (monthstr == "Oct") month = 10;
  else if (monthstr == "Nov") month = 11;
  else if (monthstr == "Dec") month = 12;
  size_t metabreak = meta.find(":");
  if (metabreak == std::string::npos) {
    year = util::str2Int(meta);
    hour = 0;
    minute = 0;
  }
  else {
    year = global->currentYear();
    int currentmonth = global->currentMonth();
    if (month > currentmonth) {
      year--;
    }
    hour = util::str2Int(meta.substr(0, metabreak));
    minute = util::str2Int(meta.substr(metabreak + 1));
  }

  std::string yearstr = util::int2Str(year);
  monthstr = util::int2Str(month);
  if (month < 10) {
    monthstr = "0" + monthstr;
  }
  daystr = util::int2Str(day);
  if (day < 10) {
    daystr = "0" + daystr;
  }
  std::string hourstr = util::int2Str(hour);
  if (hour < 10) {
    hourstr = "0" + hourstr;
  }
  meta = util::int2Str(minute);
  if (minute < 10) {
    meta = "0" + meta;
  }

  // somewhat incorrect formula, but since the exact stamp will only be used for sorting,
  // there's no need to bother
  lastmodifieddate = ((year - 1970) * 372 * 24 * 60) +
                      (month * 31 * 24 * 60) +
                      (day * 24 * 60);
  lastmodified = lastmodifieddate +
                   (hour * 60) +
                   minute;
  lastmodifiedrepr = yearstr + "-" + monthstr + "-" + daystr + " " + hourstr + ":" + meta;
}
