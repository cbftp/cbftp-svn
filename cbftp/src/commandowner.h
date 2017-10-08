#pragma once

#include "core/eventreceiver.h"
#include "statistics.h"

#define COMMANDOWNER_SITERACE 543
#define COMMANDOWNER_TRANSFERJOB 544

class FileList;
class SiteLogic;
class Path;

class CommandOwner : public EventReceiver {
public:
  virtual ~CommandOwner() {
  }
  virtual int classType() const = 0;
  virtual void fileListUpdated(SiteLogic *, FileList *) = 0;
  virtual FileList * getFileListForFullPath(SiteLogic *, const Path &) const = 0;
  virtual void addTransferStatsFile(StatsDirection, const std::string &, unsigned long long int, unsigned int) { }
};
