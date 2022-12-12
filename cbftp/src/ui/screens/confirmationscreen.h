#pragma once

#include "../uiwindow.h"

enum class ConfirmationMode {
  INFO,
  NORMAL,
  STRONG
};

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(Ui *);
  void initialize(unsigned int row, unsigned int col, const std::string & message, ConfirmationMode mode);
  void redraw() override;
  bool keyPressed(unsigned int ch) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  std::string message;
  ConfirmationMode mode;
  int strongconfirmstep;
};
