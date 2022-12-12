#pragma once

#include "../uiwindow.h"

class UIFile;

class FileInfoScreen : public UIWindow {
public:
  FileInfoScreen(Ui* ui);
  void initialize(unsigned int row, unsigned int col, UIFile* uifile);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  UIFile* uifile;
};
