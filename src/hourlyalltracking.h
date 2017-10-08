#pragma once

#include <vector>

class HourlyAllTracking {
public:
  HourlyAllTracking();
  void add(unsigned long long int);
  void tickHour();
  unsigned long long int getLast24Hours() const;
  unsigned long long int getAll() const;
  void set(unsigned long long int);
  void reset();
private:
  std::vector<unsigned long long int> hours;
  unsigned long long int last24hours;
  unsigned long long int all;
};
