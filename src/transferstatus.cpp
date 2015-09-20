#include "transferstatus.h"

#include "util.h"

TransferStatus::TransferStatus(int type, std::string source, std::string target,
    std::string release, std::string file, std::string sourcepath,
    std::string targetpath, unsigned long long int sourcesize, unsigned int assumedspeed) :
    type(type), source(source), target(target), release(release), file(file),
    timestamp(util::ctimeLog()), sourcepath(sourcepath),
    targetpath(targetpath), sourcesize(sourcesize), knowntargetsize(0),
    interpolatedtargetsize(0), interpolationfilltargetsize(0), speed(assumedspeed),
    state(TRANSFERSTATUS_STATE_IN_PROGRESS), timespent(0), progress(0),
    awaited(false) {
  if (!this->speed) {
    this->speed = 1024;
  }
  if (!this->sourcesize) {
    this->sourcesize = 1;
  }
  this->timeremaining = this->sourcesize / (this->speed * 1024);
}

std::string TransferStatus::getSource() const {
  return source;
}

std::string TransferStatus::getTarget() const {
  return target;
}

std::string TransferStatus::getRelease() const {
  return release;
}

std::string TransferStatus::getFile() const {
  return file;
}

std::string TransferStatus::getSourcePath() const {
  return sourcepath;
}

std::string TransferStatus::getTargetPath() const {
  return targetpath;
}

unsigned long long int TransferStatus::sourceSize() const {
  return sourcesize;
}

unsigned long long int TransferStatus::targetSize() const {
  return interpolatedtargetsize;
}

unsigned long long int TransferStatus::knownTargetSize() const {
  return knowntargetsize;
}

unsigned int TransferStatus::getSpeed() const {
  return speed;
}

unsigned int TransferStatus::getTimeSpent() const {
  return timespent;
}

unsigned int TransferStatus::getTimeRemaining() const {
  return timeremaining;
}

unsigned int TransferStatus::getProgress() const {
  return progress;
}

std::string TransferStatus::getTimestamp() const {
  return timestamp;
}

int TransferStatus::getState() const {
  return state;
}

int TransferStatus::getType() const {
  return type;
}

bool TransferStatus::isAwaited() const {
  return awaited;
}

void TransferStatus::setFinished() {
  state = TRANSFERSTATUS_STATE_SUCCESSFUL;
  progress = 100;
  timeremaining = 0;
}

void TransferStatus::setFailed() {
  state = TRANSFERSTATUS_STATE_FAILED;
  timeremaining = 0;
}

void TransferStatus::setAwaited(bool awaited) {
  this->awaited = awaited;
}

void TransferStatus::setTargetSize(unsigned long long int targetsize) {
  // the appearing size (interpolatedtargetsize) cannot shrink. knownsize is
  // only used for determining when speed recalculation is necessary
  knowntargetsize = targetsize;
  if (interpolatedtargetsize < knowntargetsize) {
    interpolatedtargetsize = knowntargetsize;
    updateProgress();
  }
  interpolationfilltargetsize = knowntargetsize;
}

void TransferStatus::interpolateAddSize(unsigned long long int interpolateaddsize) {
  if (interpolationfilltargetsize < interpolatedtargetsize) {
    interpolationfilltargetsize += interpolateaddsize;
    if (interpolationfilltargetsize > interpolatedtargetsize) {
      interpolatedtargetsize = interpolationfilltargetsize;
      updateProgress();
    }
  }
  else if (interpolatedtargetsize + interpolateaddsize <= sourcesize) {
    interpolatedtargetsize += interpolateaddsize;
    interpolationfilltargetsize = interpolatedtargetsize;
    updateProgress();
  }
}

void TransferStatus::updateProgress() {
  unsigned long long int temptargetsize = interpolatedtargetsize;
  progress = (100 * temptargetsize) / sourcesize;
  if (progress > 100) {
    progress = 100;
  }
}
void TransferStatus::setSpeed(unsigned int speed) {
  this->speed = speed;
  if (!this->speed) {
    this->speed = 10;
  }
}

void TransferStatus::setTimeSpent(unsigned int timespent) {
  this->timespent = timespent;
  timeremaining = (sourcesize - interpolatedtargetsize) / (speed * 1024);
}
