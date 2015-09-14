#include "uifile.h"

#include <cctype>

#include "../file.h"
#include "../localfile.h"
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
  sizerepr = util::parseSize(size);
  parseTimeStamp(file->getLastModified());
  selected = false;
  cursored = false;
}

UIFile::UIFile(const LocalFile & file) {
  name = file.getName();
  directory = file.isDirectory();
  softlink = false;
  size = file.getSize();
  sizerepr = util::parseSize(size);
  selected = false;
  cursored = false;
  owner = file.getOwner();
  group = file.getGroup();
  setLastModified(file.getYear(), file.getMonth(), file.getDay(), file.getHour(), file.getMinute());
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

void UIFile::parseTimeStamp(const std::string & uglytime) {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  if (isdigit(uglytime[0]) && isdigit(uglytime[1])) {
    parseWindowsTimeStamp(uglytime, year, month, day, hour, minute);
  }
  else {
    parseUNIXTimeStamp(uglytime, year, month, day, hour, minute);
  }
  setLastModified(year, month, day, hour, minute);
}

void UIFile::parseUNIXTimeStamp(const std::string & uglytime, int & year, int & month, int & day, int & hour, int & minute) {
  int pos = 0; // month start at 0
  while (uglytime[++pos] != ' ');
  std::string monthtmp = uglytime.substr(0, pos);
  while (uglytime[++pos] == ' '); // day start at pos
  int start = pos;
  while (uglytime[++pos] != ' ');
  std::string daytmp = uglytime.substr(start, pos - start);
  while (uglytime[++pos] == ' '); // meta start at pos
  std::string meta = uglytime.substr(pos);
  month = 0;
  day = util::str2Int(daytmp);
  if (monthtmp == "Jan") month = 1;
  else if (monthtmp == "Feb") month = 2;
  else if (monthtmp == "Mar") month = 3;
  else if (monthtmp == "Apr") month = 4;
  else if (monthtmp == "May") month = 5;
  else if (monthtmp == "Jun") month = 6;
  else if (monthtmp == "Jul") month = 7;
  else if (monthtmp == "Aug") month = 8;
  else if (monthtmp == "Sep") month = 9;
  else if (monthtmp == "Oct") month = 10;
  else if (monthtmp == "Nov") month = 11;
  else if (monthtmp == "Dec") month = 12;
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
}

void UIFile::parseWindowsTimeStamp(const std::string & uglytime, int & year, int & month, int & day, int & hour, int & minute) {
  int pos = 0, start = 0; // date start at 0
  while (uglytime[++pos] != ' ');
  std::string datestamp = uglytime.substr(start, pos - start);
  while (uglytime[++pos] == ' ');
  std::string timestamp = uglytime.substr(pos);
  if (isdigit(datestamp[2])) { // euro format
    year = util::str2Int(datestamp.substr(0, 4));
    month = util::str2Int(datestamp.substr(3, 2));
    day = util::str2Int(datestamp.substr(6, 2));
  }
  else { // US format
    month = util::str2Int(datestamp.substr(0, 2));
    day = util::str2Int(datestamp.substr(3, 2));
    year = util::str2Int(datestamp.substr(6));
    if (datestamp.length() == 8) { // US short year
      year += 2000;
    }
  }
  hour = util::str2Int(timestamp.substr(0, 2));
  minute = util::str2Int(timestamp.substr(3, 2));
  if (timestamp.length() == 7) { // US format
    hour %= 12;
    if (timestamp[5] == 'P') {
      hour += 12;
    }
  }
}

void UIFile::setLastModified(int year, int month, int day, int hour, int minute) {
  std::string yearstr = util::int2Str(year);
  std::string monthstr = util::int2Str(month);
  if (month < 10) {
    monthstr = "0" + monthstr;
  }
  std::string daystr = util::int2Str(day);
  if (day < 10) {
    daystr = "0" + daystr;
  }
  std::string hourstr = util::int2Str(hour);
  if (hour < 10) {
    hourstr = "0" + hourstr;
  }
  std::string minutestr = util::int2Str(minute);
  if (minute < 10) {
    minutestr = "0" + minutestr;
  }

  // somewhat incorrect formula, but since the exact stamp will only be used for sorting,
  // there's no need to bother
  lastmodifieddate = ((year - 1970) * 372 * 24 * 60) +
                      (month * 31 * 24 * 60) +
                      (day * 24 * 60);
  lastmodified = lastmodifieddate +
                   (hour * 60) +
                   minute;
  lastmodifiedrepr = yearstr + "-" + monthstr + "-" + daystr + " " + hourstr + ":" + minutestr;
}
