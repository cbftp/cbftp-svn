#include "statistics.h"

#include "core/tickpoke.h"
#include "globalcontext.h"

Statistics::Statistics() : spreadjobs(0), transferjobs(0) {
  global->getTickPoke()->startPoke(this, "Statistics", 60 * 60 * 1000, 0);
}

void Statistics::tick(int message) {
  sizedown.tickHour();
  filesdown.tickHour();
  sizeup.tickHour();
  filesup.tickHour();
  sizefxp.tickHour();
  filesfxp.tickHour();
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

unsigned long long int Statistics::getSizeDownLast24Hours() const {
  return sizedown.getLast24Hours();
}

unsigned int Statistics::getFilesDownLast24Hours() const {
  return filesdown.getLast24Hours();
}

unsigned long long int Statistics::getSizeUpLast24Hours() const {
  return sizeup.getLast24Hours();
}

unsigned int Statistics::getFilesUpLast24Hours() const {
  return filesup.getLast24Hours();
}

unsigned long long int Statistics::getSizeFXPLast24Hours() const {
  return sizefxp.getLast24Hours();
}

unsigned int Statistics::getFilesFXPLast24Hours() const {
  return filesfxp.getLast24Hours();
}

unsigned long long int Statistics::getSizeDownAll() const {
  return sizedown.getAll();
}

unsigned int Statistics::getFilesDownAll() const {
  return filesdown.getAll();
}

unsigned long long int Statistics::getSizeUpAll() const {
  return sizeup.getAll();
}

unsigned int Statistics::getFilesUpAll() const {
  return filesup.getAll();
}
unsigned long long int Statistics::getSizeFXPAll() const {
  return sizefxp.getAll();
}
unsigned int Statistics::getFilesFXPAll() const {
  return filesfxp.getAll();
}

unsigned int Statistics::getSpreadJobs() const {
  return spreadjobs;
}

unsigned int Statistics::getTransferJobs() const {
  return transferjobs;
}

void Statistics::setSizeDown(unsigned long long int size) {
  sizedown.set(size);
}
void Statistics::setFilesDown(unsigned int files) {
  filesdown.set(files);
}

void Statistics::setSizeUp(unsigned long long int size) {
  sizeup.set(size);
}

void Statistics::setFilesUp(unsigned int files) {
  filesup.set(files);
}

void Statistics::setSizeFXP(unsigned long long int size) {
  sizefxp.set(size);
}

void Statistics::setFilesFXP(unsigned int files) {
  filesfxp.set(files);
}

void Statistics::setSpreadJobs(unsigned int jobs) {
  spreadjobs = jobs;
}

void Statistics::setTransferJobs(unsigned int jobs) {
  transferjobs = jobs;
}
