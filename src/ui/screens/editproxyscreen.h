#pragma once

#include "../../proxy.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;

class EditProxyScreen : public UIWindow {
public:
  EditProxyScreen(Ui *);
  ~EditProxyScreen();
  void initialize(unsigned int, unsigned int, std::string, std::string);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  Proxy * proxy;
  Proxy modproxy;
  std::shared_ptr<MenuSelectOptionTextArrow> authmethod;
  std::shared_ptr<MenuSelectOptionTextArrow> ampm;
  int latestauthmethod;
  std::string operation;
};
