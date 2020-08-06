#include "metricsscreen.h"

#include "../../globalcontext.h"
#include "../../loadmonitor.h"
#include "../../util.h"

#include "../ui.h"
#include "../braillegraph.h"

namespace {

void drawAllIn(Ui* ui, const std::list<BrailleGraph::GraphChar>& list, unsigned int startrow) {
  for (const BrailleGraph::GraphChar& ch : list) {
    ui->printChar(ch.row + startrow, ch.col, ch.ch);
  }
}

void drawGraph(Ui* ui, const std::unique_ptr<BrailleGraph>& graph, unsigned int startrow = 0) {
  drawAllIn(ui, graph->getStaticBorderChars(), startrow);
  drawAllIn(ui, graph->getChangingBorderChars(), startrow);
  drawAllIn(ui, graph->getGraphChars(), startrow);
}

}

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
  cpuall->setData(global->getLoadMonitor()->getCpuUsageAllHistory());
  cpuworker->setData(global->getLoadMonitor()->getCpuUsageWorkerHistory());
  perflevel->setData(global->getLoadMonitor()->getPerformanceLevelHistory());
  init(row, col);
}

void MetricsScreen::redraw() {
  unsigned int graphheight = row / 3;
  cpuall->resize(graphheight, col, true);
  cpuworker->resize(graphheight, col, true);
  perflevel->resize(graphheight, col, true);
  update();
}

void MetricsScreen::update() {
  ui->erase();
  ui->hideCursor();
  cpuall->addNewData(global->getLoadMonitor()->getUnseenCpuUsageAllHistory());
  cpuworker->addNewData(global->getLoadMonitor()->getUnseenCpuUsageWorkerHistory());
  perflevel->addNewData(global->getLoadMonitor()->getUnseenPerformanceLevelHistory());
  drawGraph(ui, cpuall);
  drawGraph(ui, cpuworker, cpuall->rows());
  drawGraph(ui, perflevel, cpuall->rows() + cpuworker->rows());
}

bool MetricsScreen::keyPressed(unsigned int ch) {
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

std::string MetricsScreen::getLegendText() const {
  return "[Esc/Space/Enter/c] Return";
}

std::string MetricsScreen::getInfoLabel() const {
  return "METRICS";
}

std::string MetricsScreen::getInfoText() const {
  return "History for the latest " + std::to_string(global->getLoadMonitor()->getHistoryLengthSeconds()) + " seconds";
}
