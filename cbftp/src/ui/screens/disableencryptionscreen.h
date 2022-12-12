#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;

class DisableEncryptionScreen : public UIWindow {
public:
  DisableEncryptionScreen(Ui *);
  ~DisableEncryptionScreen();
  void initialize(unsigned int, unsigned int);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  bool active;
  bool mismatch;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
};
