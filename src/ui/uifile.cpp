#include "uifile.h"

#include "../globalcontext.h"
#include "../file.h"

extern GlobalContext * global;

UIFile::UIFile(File * file) {
  name = file->getName();
  directory = file->isDirectory();
  softlink = file->isLink();
  owner = file->getOwner();
  group = file->getGroup();
  size = file->getSize();
  linktarget = file->getLinkTarget();
  sizerepr = parseSize(file->getSize());
  parseTimeStamp(file->getLastModified());
  selected = false;
  cursored = false;
}

std::string UIFile::getName() {
  return name;
}

std::string UIFile::getLinkTarget() {
  return linktarget;
}

std::string UIFile::getOwner() {
  return owner;
}

std::string UIFile::getGroup() {
  return group;
}

std::string UIFile::getLastModified() {
  return lastmodifiedrepr;
}

int UIFile::getModifyTime() {
  return lastmodified;
}

int UIFile::getModifyDate() {
  return lastmodifieddate;
}

unsigned long long int UIFile::getSize() {
  return size;
}

std::string UIFile::getSizeRepr() {
  return sizerepr;
}

bool UIFile::isDirectory() {
  return directory;
}

bool UIFile::isLink() {
  return softlink;
}

bool UIFile::isSelected() {
  return selected;
}

bool UIFile::isCursored() {
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

std::string UIFile::parseSize(unsigned long long int size) {
  int iprefix;
  for (iprefix = 0; iprefix < 6 && size / powers[iprefix] >= 1000; iprefix++);
  unsigned long long int currentpower = powers[iprefix];
  std::string result;
  int whole = size / currentpower;
  if (iprefix == 0) {
    result = global->int2Str(whole) + " B";
  }
  else {
    unsigned long long int decim = ((size % currentpower) * sizegranularity) / currentpower + 5;
    if (decim >= sizegranularity) {
      whole++;
      decim = 0;
    }
    std::string decimstr = global->int2Str(decim);
    while (decimstr.length() <= SIZEDECIMALS) {
      decimstr = "0" + decimstr;
    }
    result = global->int2Str(whole) + "." + decimstr.substr(0, SIZEDECIMALS) + " ";
    switch (iprefix) {
      case 1:
        result.append("kB");
        break;
      case 2:
        result.append("MB");
        break;
      case 3:
        result.append("GB");
        break;
      case 4:
        result.append("TB");
        break;
      case 5:
        result.append("PB");
        break;
      case 6:
        result.append("EB");
        break;
    }
  }
  return result;
}

void UIFile::parseTimeStamp(std::string uglytime) {
  std::string monthstr = uglytime.substr(0, 3);
  std::string daystr = uglytime.substr(4, 2);
  std::string meta = uglytime.substr(7, 5);
  int year;
  int month = 0;
  int day = global->str2Int(daystr);
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
    year = global->str2Int(meta);
    hour = 0;
    minute = 0;
  }
  else {
    year = global->currentYear();
    int currentmonth = global->currentMonth();
    if (month > currentmonth) {
      year--;
    }
    hour = global->str2Int(meta.substr(0, metabreak));
    minute = global->str2Int(meta.substr(metabreak + 1));
  }

  std::string yearstr = global->int2Str(year);
  monthstr = global->int2Str(month);
  if (month < 10) {
    monthstr = "0" + monthstr;
  }
  daystr = global->int2Str(day);
  if (day < 10) {
    daystr = "0" + daystr;
  }
  std::string hourstr = global->int2Str(hour);
  if (hour < 10) {
    hourstr = "0" + hourstr;
  }
  meta = global->int2Str(minute);
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

int UIFile::getSizeGranularity() {
  int gran = 1;
  for (int i = 0; i <= SIZEDECIMALS; i++) {
    gran *= 10;
  }
  return gran;
}

std::vector<unsigned long long int> UIFile::getPowers() {
  std::vector<unsigned long long int> vec;
  vec.reserve(7);
  unsigned long long int pow = 1;
  for (int i = 0; i < 7; i++) {
    vec.push_back(pow);
    pow *= 1024;
  }
  return vec;
}

unsigned int UIFile::sizegranularity = UIFile::getSizeGranularity();
std::vector<unsigned long long int> UIFile::powers = UIFile::getPowers();
