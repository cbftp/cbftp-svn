#pragma once

#include <vector>

#include "hourlyalltracking.h"
#include "core/eventreceiver.h"

enum StatsDirection {
  STATS_UP,
  STATS_DOWN,
  STATS_FXP
};

class Statistics : public EventReceiver {
public:
  Statistics();
  void tick(int);
  void addTransferStatsFile(StatsDirection, unsigned long long int size);
  void addSpreadJob();
  void addTransferJob();
  unsigned long long int getSizeDownLast24Hours() const;
  unsigned int getFilesDownLast24Hours() const;
  unsigned long long int getSizeUpLast24Hours() const;
  unsigned int getFilesUpLast24Hours() const;
  unsigned long long int getSizeFXPLast24Hours() const;
  unsigned int getFilesFXPLast24Hours() const;
  unsigned long long int getSizeDownAll() const;
  unsigned int getFilesDownAll() const;
  unsigned long long int getSizeUpAll() const;
  unsigned int getFilesUpAll() const;
  unsigned long long int getSizeFXPAll() const;
  unsigned int getFilesFXPAll() const;
  unsigned int getSpreadJobs() const;
  unsigned int getTransferJobs() const;
  void setSizeDown(unsigned long long int);
  void setFilesDown(unsigned int);
  void setSizeUp(unsigned long long int);
  void setFilesUp(unsigned int);
  void setSizeFXP(unsigned long long int);
  void setFilesFXP(unsigned int);
  void setSpreadJobs(unsigned int);
  void setTransferJobs(unsigned int);
private:
  HourlyAllTracking sizedown;
  HourlyAllTracking filesdown;
  HourlyAllTracking sizeup;
  HourlyAllTracking filesup;
  HourlyAllTracking sizefxp;
  HourlyAllTracking filesfxp;
  unsigned int spreadjobs;
  unsigned int transferjobs;
};
