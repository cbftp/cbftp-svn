#include "infoscreen.h"

#include "../../globalcontext.h"
#include "../../statistics.h"
#include "../../util.h"
#include "../../buildinfo.h"
#include "../../hourlyalltracking.h"
#include "../../settingsloadersaver.h"
#include "../../loadmonitor.h"

#include "../../core/polling.h"
#include "../../core/sslmanager.h"

#include "../ui.h"

InfoScreen::InfoScreen(Ui* ui) : UIWindow(ui, "InfoScreen") {
  allowimplicitgokeybinds = false;
}

void InfoScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  init(row, col);
}

void InfoScreen::redraw() {
  vv->clear();
  ui->hideCursor();
  unsigned long long int sizefxpday = global->getStatistics()->getSizeFXP().getLast24Hours();
  unsigned int filesfxpday = global->getStatistics()->getFilesFXP().getLast24Hours();
  unsigned long long int sizefxpall = global->getStatistics()->getSizeFXP().getAll();
  unsigned int filesfxpall = global->getStatistics()->getFilesFXP().getAll();
  unsigned long long int sizeupday = global->getStatistics()->getSizeUp().getLast24Hours();
  unsigned int filesupday = global->getStatistics()->getFilesUp().getLast24Hours();
  unsigned long long int sizeupall = global->getStatistics()->getSizeUp().getAll();
  unsigned int filesupall = global->getStatistics()->getFilesUp().getAll();
  unsigned long long int sizedownday = global->getStatistics()->getSizeDown().getLast24Hours();
  unsigned int filesdownday = global->getStatistics()->getFilesDown().getLast24Hours();
  unsigned long long int sizedownall = global->getStatistics()->getSizeDown().getAll();
  unsigned int filesdownall = global->getStatistics()->getFilesDown().getAll();

  unsigned int i = 1;
  vv->putStr(i++, 1, "Compile time: " + BuildInfo::compileTime());
  vv->putStr(i++, 1, "Version tag: " + BuildInfo::version());
  vv->putStr(i++, 1, "Distribution tag: " + BuildInfo::tag());
  vv->putStr(i++, 1, "OpenSSL version: " + Core::SSLManager::version());
  vv->putStr(i++, 1, "Polling syscall: " + Core::Polling::type());
  vv->putStr(i++, 1, std::string("Data file encryption: ") + (global->getSettingsLoaderSaver()->getState() == DataFileState::EXISTS_DECRYPTED ? "Enabled" : "Disabled"));
  i++;
  vv->putStr(i++, 1, "Traffic measurements");
  vv->putStr(i++, 1, "FXP      last 24 hours: " + util::parseSize(sizefxpday) + ", " +
               std::to_string(filesfxpday) + " files - All time: " + util::parseSize(sizefxpall) + ", " +
               std::to_string(filesfxpall) + " files");
  vv->putStr(i++, 1, "Upload   last 24 hours: " + util::parseSize(sizeupday) + ", " +
                 std::to_string(filesupday) + " files - All time: " + util::parseSize(sizeupall) + ", " +
                 std::to_string(filesupall) + " files");
  vv->putStr(i++, 1, "Download last 24 hours: " + util::parseSize(sizedownday) + ", " +
                 std::to_string(filesdownday) + " files - All time: " + util::parseSize(sizedownall) + ", " +
                 std::to_string(filesdownall) + " files");
  ++i;
  vv->putStr(i++, 1, "All time spread jobs: " + std::to_string(global->getStatistics()->getSpreadJobs()));
  vv->putStr(i++, 1, "All time transfer jobs: " + std::to_string(global->getStatistics()->getTransferJobs()));
  ++i;
  vv->putStr(i++, 1, "Current CPU load total: " + std::to_string(global->getLoadMonitor()->getCurrentCpuUsageAll()) + "%");
  vv->putStr(i++, 1, "Current CPU load worker: " + std::to_string(global->getLoadMonitor()->getCurrentCpuUsageWorker()) + "%");
  vv->putStr(i++, 1, "Current worker queue size: " + std::to_string(global->getLoadMonitor()->getCurrentWorkerQueueSize()));
  vv->putStr(i++, 1, "Current performance level: " + std::to_string(global->getLoadMonitor()->getCurrentRecommendedPerformanceLevel()));
}

bool InfoScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case ' ':
    case 'c':
    case 10:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string InfoScreen::getLegendText() const {
  return "[Esc/Space/Enter/c] Return";
}

std::string InfoScreen::getInfoLabel() const {
  return "GENERAL INFO";
}
