#pragma once

#include <string>

#define TRANSFERSTATUS_TYPE_FXP 1892
#define TRANSFERSTATUS_TYPE_DOWNLOAD 1893
#define TRANSFERSTATUS_TYPE_UPLOAD 1894

#define TRANSFERSTATUS_STATE_IN_PROGRESS 1901
#define TRANSFERSTATUS_STATE_SUCCESSFUL 1902
#define TRANSFERSTATUS_STATE_FAILED 1903

class TransferStatus {
public:
  TransferStatus(int, std::string, std::string, std::string, std::string, std::string, std::string, unsigned int, unsigned int);
  std::string getSource() const;
  std::string getTarget() const;
  std::string getRelease() const;
  std::string getFile() const;
  std::string getSourcePath() const;
  std::string getTargetPath() const;
  unsigned int sourceSize() const;
  unsigned int targetSize() const;
  unsigned int knownTargetSize() const;
  unsigned int getSpeed() const;
  unsigned int getTimeSpent() const;
  unsigned int getTimeRemaining() const;
  unsigned int getProgress() const;
  std::string getTimestamp() const;
  int getState() const;
  int getType() const;
  bool isAwaited() const;
  void setFinished();
  void setFailed();
  void setAwaited(bool);
  void setTargetSize(unsigned int);
  void interpolateAddSize(unsigned int);
  void setSpeed(unsigned int);
  void setTimeSpent(unsigned int);
private:
  void updateProgress();
  int type;
  std::string source;
  std::string target;
  std::string release;
  std::string file;
  std::string timestamp;
  std::string sourcepath;
  std::string targetpath;
  unsigned int sourcesize;
  unsigned int knowntargetsize;
  unsigned int interpolatedtargetsize;
  unsigned int interpolationfilltargetsize;
  unsigned int speed;
  int state;
  unsigned int timespent;
  unsigned int timeremaining;
  unsigned int progress;
  bool awaited;
};
