#include "statistics.h"

#include "core/tickpoke.h"
#include "globalcontext.h"

Statistics::Statistics() : spreadjobs(0), transferjobs(0) {
  global->getTickPoke()->startPoke(this, "Statistics", 60 * 1000, 0);
}

void Statistics::tick(int message) {
  sizedown.tickMinute();
  filesdown.tickMinute();
  sizeup.tickMinute();
  filesup.tickMinute();
  sizefxp.tickMinute();
  filesfxp.tickMinute();
}

void Statistics::addTransferStatsFile(StatsDirection direction, unsigned long long int size) {
  switch (direction) {
    case STATS_UP:
      sizeup.add(size);
      filesup.add(1);
      break;
    case STATS_DOWN:
      sizedown.add(size);
      filesdown.add(1);
      break;
    case STATS_FXP:
      sizefxp.add(size);
      filesfxp.add(1);
      break;
  }
}

void Statistics::addSpreadJob() {
  ++spreadjobs;
}

void Statistics::addTransferJob() {
  ++transferjobs;
}

const HourlyAllTracking& Statistics::getSizeDown() const {
  return sizedown;
}

const HourlyAllTracking& Statistics::getSizeUp() const {
  return sizeup;
}

const HourlyAllTracking& Statistics::getSizeFXP() const {
  return sizefxp;
}

const HourlyAllTracking& Statistics::getFilesDown() const {
  return filesdown;
}

const HourlyAllTracking& Statistics::getFilesUp() const {
  return filesup;
}

const HourlyAllTracking& Statistics::getFilesFXP() const {
  return filesfxp;
}

HourlyAllTracking& Statistics::getSizeDown() {
  return sizedown;
}

HourlyAllTracking& Statistics::getSizeUp() {
  return sizeup;
}

HourlyAllTracking& Statistics::getSizeFXP() {
  return sizefxp;
}

HourlyAllTracking& Statistics::getFilesDown() {
  return filesdown;
}

HourlyAllTracking& Statistics::getFilesUp() {
  return filesup;
}

HourlyAllTracking& Statistics::getFilesFXP() {
  return filesfxp;
}

unsigned int Statistics::getSpreadJobs() const {
  return spreadjobs;
}

unsigned int Statistics::getTransferJobs() const {
  return transferjobs;
}

void Statistics::setSpreadJobs(unsigned int jobs) {
  spreadjobs = jobs;
}

void Statistics::setTransferJobs(unsigned int jobs) {
  transferjobs = jobs;
}
