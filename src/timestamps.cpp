#include "timestamps.h"

#include <cctype>

#include "timereference.h"

namespace timestamps {

Timestamp::Timestamp(int year, int month, int day, int hour, int minute) :
  valid(true), year(year), month(month), day(day), hour(hour), minute(minute)
{
}

Timestamp::Timestamp() : valid(false) {
}

std::string Timestamp::toString() const {
  if (!valid) {
    return "<invalid>";
  }
  std::string yearstr = std::to_string(year);
  std::string monthstr = std::to_string(month);
  if (month < 10) {
    monthstr = "0" + monthstr;
  }
  std::string daystr = std::to_string(day);
  if (day < 10) {
    daystr = "0" + daystr;
  }
  std::string hourstr = std::to_string(hour);
  if (hour < 10) {
    hourstr = "0" + hourstr;
  }
  std::string minutestr = std::to_string(minute);
  if (minute < 10) {
    minutestr = "0" + minutestr;
  }
  return yearstr + "-" + monthstr + "-" + daystr + " " + hourstr + ":" + minutestr;
}

Timestamp parseTimestamp(const std::string& uglytime) {
  if (isdigit(uglytime[0]) && isdigit(uglytime[1])) {
    return parseWindowsTimestamp(uglytime);
  }
  return parseUNIXTimestamp(uglytime);
}

Timestamp parseUNIXTimestamp(const std::string& uglytime) {
  if (uglytime.empty()) {
    return Timestamp();
  }
  size_t len = uglytime.length();
  size_t pos = 0; // month start at 0
  while (pos < len - 1 && uglytime[++pos] != ' ');
  std::string monthtmp = uglytime.substr(0, pos);
  while (pos < len - 1 && uglytime[++pos] == ' '); // day start at pos
  int start = pos;
  while (pos < len - 1 && uglytime[++pos] != ' ');
  std::string daytmp = uglytime.substr(start, pos - start);
  while (pos < len - 1 && uglytime[++pos] == ' '); // meta start at pos
  std::string meta = uglytime.substr(pos);
  int year;
  int month = 0;
  int day = std::stoi(daytmp);
  int hour;
  int minute;
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
    year = std::stoi(meta);
    hour = 0;
    minute = 0;
  }
  else {
    year = TimeReference::currentYear();
    int currentmonth = TimeReference::currentMonth();
    if (month > currentmonth) {
      year--;
    }
    hour = std::stoi(meta.substr(0, metabreak));
    minute = std::stoi(meta.substr(metabreak + 1));
  }
  return Timestamp(year, month, day, hour, minute);
}

Timestamp parseWindowsTimestamp(const std::string & uglytime) {
  if (uglytime.empty()) {
    return Timestamp();
  }
  size_t len = uglytime.length();
  size_t pos = 0, start = 0; // date start at 0
  while (pos < len - 1 && uglytime[++pos] != ' ');
  std::string datestamp = uglytime.substr(start, pos - start);
  if (datestamp.length() < 8) { // bad format
    return Timestamp();
  }
  while (pos < len - 1 && uglytime[++pos] == ' ');
  std::string timestamp = uglytime.substr(pos);
  if (timestamp.length() < 5) { // bad format
    return Timestamp();
  }
  int year;
  int month;
  int day;
  int hour;
  int minute;
  if (isdigit(datestamp[2])) { // euro format
    year = std::stoi(datestamp.substr(0, 4));
    month = std::stoi(datestamp.substr(3, 2));
    day = std::stoi(datestamp.substr(6, 2));
  }
  else { // US format
    month = std::stoi(datestamp.substr(0, 2));
    day = std::stoi(datestamp.substr(3, 2));
    year = std::stoi(datestamp.substr(6));
    if (datestamp.length() == 8) { // US short year
      year += 2000;
    }
  }
  hour = std::stoi(timestamp.substr(0, 2));
  minute = std::stoi(timestamp.substr(3, 2));
  if (timestamp.length() == 7) { // US format
    hour %= 12;
    if (timestamp[5] == 'P') {
      hour += 12;
    }
  }
  return Timestamp(year, month, day, hour, minute);
}

} // namespace timestamps
