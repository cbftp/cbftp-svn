#pragma once

#include "../uiwindow.h"

class InfoScreen : public UIWindow {
public:
  InfoScreen(Ui* ui);
  void initialize(unsigned int row, unsigned int col);
  void redraw() override;
  bool keyPressed(unsigned int ch) override;
  std::string getInfoLabel() const override;
  void command(const std::string& command, const std::string& arg) override;
private:
  int confirmaction;
};
