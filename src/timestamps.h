#pragma once

#include <string>

namespace timestamps {

struct Timestamp {
public:
  Timestamp(int year, int month, int day, int hour, int minute);
  Timestamp();
  std::string toString() const;
  bool valid;
  int year;
  int month;
  int day;
  int hour;
  int minute;
};

Timestamp parseTimestamp(const std::string& uglytime);
Timestamp parseUNIXTimestamp(const std::string& uglytime);
Timestamp parseWindowsTimestamp(const std::string& uglytime);

} // namespace timestamps
