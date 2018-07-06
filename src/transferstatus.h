#pragma once

#include <string>
#include <list>

#include "path.h"

enum TransferStatusType {
  TRANSFERSTATUS_TYPE_FXP,
  TRANSFERSTATUS_TYPE_DOWNLOAD,
  TRANSFERSTATUS_TYPE_UPLOAD
};

enum TransferStatusState {
  TRANSFERSTATUS_STATE_IN_PROGRESS,
  TRANSFERSTATUS_STATE_SUCCESSFUL,
  TRANSFERSTATUS_STATE_FAILED,
  TRANSFERSTATUS_STATE_DUPE,
  TRANSFERSTATUS_STATE_ABORTED
};

class TransferStatusCallback;
class FileList;

class TransferStatus {
public:
  TransferStatus(int, std::string, std::string, std::string, std::string, FileList *, const Path &, FileList *, const Path &, unsigned long long int, unsigned int, int, int, bool, bool);
  std::string getSource() const;
  std::string getTarget() const;
  std::string getRelease() const;
  std::string getFile() const;
  const Path & getSourcePath() const;
  const Path & getTargetPath() const;
  FileList * getSourceFileList() const;
  FileList * getTargetFileList() const;
  unsigned long long int sourceSize() const;
  unsigned long long int targetSize() const;
  unsigned long long int knownTargetSize() const;
  unsigned int getSpeed() const;
  unsigned int getTimeSpent() const;
  int getTimeRemaining() const;
  unsigned int getProgress() const;
  std::string getTimestamp() const;
  TransferStatusCallback * getCallback() const;
  int getState() const;
  int getType() const;
  bool isAwaited() const;
  int getSourceSlot() const;
  int getTargetSlot() const;
  bool getSSL() const;
  bool getDefaultActive() const;
  std::string getPassiveAddress() const;
  std::string getCipher() const;
  bool getSSLSessionReused() const;
  const std::list<std::string> & getLogLines() const;
  void setFinished();
  void setFailed();
  void setDupe();
  void setAborted();
  void setAwaited(bool);
  void setCallback(TransferStatusCallback *);
  void setTargetSize(unsigned long long int);
  void interpolateAddSize(unsigned long long int);
  void setSpeed(unsigned int);
  void setTimeSpent(unsigned int);
  void setPassiveAddress(const std::string &);
  void setCipher(const std::string &);
  void setSSLSessionReused(bool);
  void setSourceSize(unsigned long long int size);
  void addLogLine(const std::string &);
private:
  void updateProgress();
  int type;
  std::string source;
  std::string target;
  std::string release;
  std::string file;
  std::string timestamp;
  Path sourcepath;
  Path targetpath;
  unsigned long long int sourcesize;
  unsigned long long int knowntargetsize;
  unsigned long long int interpolatedtargetsize;
  unsigned long long int interpolationfilltargetsize;
  unsigned int speed;
  int state;
  unsigned int timespent;
  int timeremaining;
  unsigned int progress;
  bool awaited;
  TransferStatusCallback * callback;
  FileList * fls;
  FileList * fld;
  int srcslot;
  int dstslot;
  bool ssl;
  bool defaultactive;
  std::string passiveaddr;
  std::string cipher;
  bool sslsessionreused;
  std::list<std::string> loglines;
};
