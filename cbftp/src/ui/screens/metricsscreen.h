#pragma once

#include <memory>

#include "../uiwindow.h"

class BrailleGraph;

class MetricsScreen : public UIWindow {
public:
  MetricsScreen(Ui* ui);
  ~MetricsScreen();
  void initialize(unsigned int row, unsigned int col);
  void redraw() override;
  void update() override;
  bool keyPressed(unsigned int ch) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
private:
  std::unique_ptr<BrailleGraph> cpuall;
  std::unique_ptr<BrailleGraph> cpuworker;
  std::unique_ptr<BrailleGraph> workqueuesize;
  std::unique_ptr<BrailleGraph> perflevel;
  std::unique_ptr<BrailleGraph> filelistrefreshrate;
};
