#include "loadmonitor.h"

#include <ctime>
#include <thread>

#include "core/tickpoke.h"

#include "globalcontext.h"
#include "loadmonitorcallback.h"


#define LOAD_MONITOR_CHECK_INTERVAL_MS 250
#define HISTORY_LENGTH_SECONDS 60
#define PERFLEVEL_MIN 1
#define PERFLEVEL_MAX 9
#define PERFLEVEL_THROTTLE_THRESHOLD_PC 82
#define PERFLEVEL_THROTTLE_THRESHOLD_TOLERANCE 10

LoadMonitor::LoadMonitor() : lasttimeallms(0), lasttimeworkerms(0), perflevel(PERFLEVEL_MAX),
  allhistory(HISTORY_LENGTH_SECONDS * (1000 / LOAD_MONITOR_CHECK_INTERVAL_MS), 0),
  workerhistory(HISTORY_LENGTH_SECONDS * (1000 / LOAD_MONITOR_CHECK_INTERVAL_MS), 0),
  perflevelhistory(HISTORY_LENGTH_SECONDS * (1000 / LOAD_MONITOR_CHECK_INTERVAL_MS), PERFLEVEL_MAX),
  numcores(std::thread::hardware_concurrency()),
  throttletoppc(PERFLEVEL_THROTTLE_THRESHOLD_PC + PERFLEVEL_THROTTLE_THRESHOLD_TOLERANCE),
  throttlebottompc(PERFLEVEL_THROTTLE_THRESHOLD_PC - PERFLEVEL_THROTTLE_THRESHOLD_TOLERANCE)
{
  global->getTickPoke()->startPoke(this, "LoadMonitor", LOAD_MONITOR_CHECK_INTERVAL_MS, 0);
}

LoadMonitor::~LoadMonitor() {
  global->getTickPoke()->stopPoke(this, 0);
}

void LoadMonitor::tick(int message) {
  struct timespec tpall;
  struct timespec tpworker;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tpall);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tpworker);
  unsigned long long int timeallms = tpall.tv_sec * 1000 + tpall.tv_nsec / 1000000;
  unsigned long long int timeworkerms = tpworker.tv_sec * 1000 + tpworker.tv_nsec / 1000000;
  unsigned long long int currtimeallms = timeallms - lasttimeallms;
  unsigned long long int currtimeworkerms = timeworkerms - lasttimeworkerms;
  unsigned int currallpc = currtimeallms * 100 / numcores / LOAD_MONITOR_CHECK_INTERVAL_MS;
  unsigned int currworkerpc = currtimeworkerms * 100 / LOAD_MONITOR_CHECK_INTERVAL_MS;
  allhistory.push_front(currallpc);
  allhistory.pop_back();
  workerhistory.push_front(currworkerpc);
  workerhistory.pop_back();
  bool perflevelchanged = false;
  if ((currallpc > throttletoppc || currworkerpc > throttletoppc) && perflevel > PERFLEVEL_MIN) {
    --perflevel;
    perflevelchanged = true;
  }
  else if ((currallpc < throttlebottompc || currworkerpc < throttlebottompc) && perflevel < PERFLEVEL_MAX) {
    ++perflevel;
    perflevelchanged = true;
  }
  perflevelhistory.push_front(perflevel);
  perflevelhistory.pop_back();
  lasttimeallms = timeallms;
  lasttimeworkerms = timeworkerms;
  if (perflevelchanged) {
    for (LoadMonitorCallback* cb : listeners) {
      cb->recommendedPerformanceLevelChanged(perflevel);
    }
  }
}

unsigned int LoadMonitor::getCurrentRecommendedPerformanceLevel() const {
  return perflevel;
}

unsigned int LoadMonitor::getCurrentCpuUsageAll() const {
  return allhistory.front();
}
unsigned int LoadMonitor::getCurrentCpuUsageWorker() const {
  return workerhistory.front();
}

void LoadMonitor::addListener(LoadMonitorCallback* cb) {
  listeners.push_back(cb);
}

void LoadMonitor::removeListener(LoadMonitorCallback* cb) {
  listeners.remove(cb);
}

const std::list<unsigned int>& LoadMonitor::getCpuUsageAllHistory() const {
  return allhistory;
}

const std::list<unsigned int>& LoadMonitor::getCpuUsageWorkerHistory() const {
  return workerhistory;
}

const std::list<unsigned int>& LoadMonitor::getPerformanceLevelHistory() const {
  return perflevelhistory;
}

int LoadMonitor::getHistoryLengthSeconds() const {
  return HISTORY_LENGTH_SECONDS;
}
