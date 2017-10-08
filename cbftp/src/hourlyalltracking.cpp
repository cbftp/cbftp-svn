#include "hourlyalltracking.h"

HourlyAllTracking::HourlyAllTracking() : hours(24), last24hours(0), all(0) {

}

void HourlyAllTracking::add(unsigned long long int value) {
  hours[0] += value;
  last24hours += value;
  all += value;
}

void HourlyAllTracking::tickHour() {
  last24hours -= hours[23];
  for (int i = 23; i > 0; --i) {
    hours[i] = hours[i - 1];
  }
  hours[0] = 0;
}

unsigned long long int HourlyAllTracking::getLast24Hours() const {
  return last24hours;
}

unsigned long long int HourlyAllTracking::getAll() const {
  return all;
}

void HourlyAllTracking::set(unsigned long long int count) {
  all = count;
}

void HourlyAllTracking::reset() {
  hours.clear();
  hours.resize(24);
  last24hours = 0;
  all = 0;
}
