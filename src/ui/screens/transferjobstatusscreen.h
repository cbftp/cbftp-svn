#pragma once

#include <string>

#include "../uiwindow.h"

class TransferJob;
class Ui;

class TransferJobStatusScreen : public UIWindow {
public:
  TransferJobStatusScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string);
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  TransferJob * transferjob;
  std::string filename;
};
