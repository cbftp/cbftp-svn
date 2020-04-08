#include "metricsscreen.h"

#include "../../globalcontext.h"
#include "../../loadmonitor.h"
#include "../../util.h"

#include "../ui.h"
#include "../braillegraph.h"

MetricsScreen::MetricsScreen(Ui* ui) : UIWindow(ui, "MetricsScreen") {
  allowimplicitgokeybinds = false;
  autoupdate = true;
}

MetricsScreen::~MetricsScreen() {

}

void MetricsScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  unsigned int graphheight = row / 3;
  cpuall = std::unique_ptr<BrailleGraph>(new BrailleGraph(graphheight, col, "CPU load total", "%", 0, 100));
  cpuworker = std::unique_ptr<BrailleGraph>(new BrailleGraph(graphheight, col, "CPU load worker", "%", 0, 100));
  perflevel = std::unique_ptr<BrailleGraph>(new BrailleGraph(graphheight, col, "Performance level", "", 1, 9));
  init(row, col);
}

void MetricsScreen::redraw() {
  ui->erase();
  ui->hideCursor();
  unsigned int graphheight = row / 3;
  cpuall->resize(graphheight, col);
  cpuworker->resize(graphheight, col);
  perflevel->resize(graphheight, col);
  cpuall->setData(global->getLoadMonitor()->getCpuUsageAllHistory());
  cpuworker->setData(global->getLoadMonitor()->getCpuUsageWorkerHistory());
  perflevel->setData(global->getLoadMonitor()->getPerformanceLevelHistory());

  for (unsigned int y = 0; y < cpuall->rows(); ++y) {
    for (unsigned int x = 0; x < cpuall->cols(); ++x) {
      unsigned int chr = cpuall->getChar(y, x);
      if (chr) {
        ui->printChar(y, x, chr);
      }
    }
  }
  for (unsigned int y = 0; y < cpuworker->rows(); ++y) {
    for (unsigned int x = 0; x < cpuworker->cols(); ++x) {
      unsigned int chr = cpuworker->getChar(y, x);
      if (chr) {
        ui->printChar(y + cpuall->rows(), x, chr);
      }
    }
  }
  for (unsigned int y = 0; y < perflevel->rows(); ++y) {
    for (unsigned int x = 0; x < perflevel->cols(); ++x) {
      unsigned int chr = perflevel->getChar(y, x);
      if (chr) {
        ui->printChar(y + cpuall->rows() + cpuworker->rows(), x, chr);
      }
    }
  }
}

void MetricsScreen::update() {
  redraw();
}

bool MetricsScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case ' ':
    case 10:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string MetricsScreen::getLegendText() const {
  return "[Any] Return";
}

std::string MetricsScreen::getInfoLabel() const {
  return "METRICS";
}

std::string MetricsScreen::getInfoText() const {
  return "History for the latest " + std::to_string(global->getLoadMonitor()->getHistoryLengthSeconds()) + " seconds";
}
