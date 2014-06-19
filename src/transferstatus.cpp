#include "transferstatus.h"

#include "globalcontext.h"

TransferStatus::TransferStatus(int type, std::string source, std::string target,
    std::string release, std::string file, std::string sourcepath,
    std::string targetpath, unsigned int sourcesize, unsigned int assumedspeed) :
    type(type), source(source), target(target), release(release), file(file),
    timestamp(GlobalContext::ctimeLog()), sourcepath(sourcepath),
    targetpath(targetpath), sourcesize(sourcesize), knowntargetsize(0),
    interpolatedtargetsize(0), interpolationfilltargetsize(0), speed(assumedspeed),
    finished(false), timespent(0), progress(0) {
  if (!this->speed) {
    this->speed = 1;
  }
  if (!this->sourcesize) {
    this->sourcesize = 1;
  }
  this->timeremaining = this->sourcesize / this->speed;
}

std::string TransferStatus::getSource() {
  return source;
}

std::string TransferStatus::getTarget() {
  return target;
}

std::string TransferStatus::getRelease() {
  return release;
}

std::string TransferStatus::getFile() {
  return file;
}

std::string TransferStatus::getSourcePath() {
  return sourcepath;
}

std::string TransferStatus::getTargetPath() {
  return targetpath;
}

unsigned int TransferStatus::sourceSize() {
  return sourcesize;
}

unsigned int TransferStatus::targetSize() {
  return interpolatedtargetsize;
}

unsigned int TransferStatus::knownTargetSize() {
  return knowntargetsize;
}

unsigned int TransferStatus::getSpeed() {
  return speed;
}

unsigned int TransferStatus::getTimeSpent() {
  return timespent;
}

unsigned int TransferStatus::getTimeRemaining() {
  return timeremaining;
}

unsigned int TransferStatus::getProgress() {
  return progress;
}

std::string TransferStatus::getTimestamp() {
  return timestamp;
}

void TransferStatus::setFinished() {
  finished = true;
  progress = 100;
  timeremaining = 0;
}

void TransferStatus::setTargetSize(unsigned int targetsize) {
  // the appearing size (interpolatedtargetsize) cannot shrink. knownsize is
  // only used for determining when speed recalculation is necessary
  knowntargetsize = targetsize;
  if (interpolatedtargetsize < knowntargetsize) {
    interpolatedtargetsize = knowntargetsize;
    updateProgress();
  }
  interpolationfilltargetsize = knowntargetsize;
}

void TransferStatus::interpolateAddSize(unsigned int interpolateaddsize) {
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
