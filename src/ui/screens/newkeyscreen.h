#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;

class NewKeyScreen : public UIWindow {
public:
  NewKeyScreen(Ui *);
  ~NewKeyScreen();
  void initialize(unsigned int, unsigned int);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getInfoLabel() const override;
private:
  bool mismatch;
  bool tooshort;
  MenuSelectOption mso;
};
