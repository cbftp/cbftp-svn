#pragma once

#include <memory>
#include <vector>

#include "../uiwindow.h"

class Site;
class SiteLogic;

class SiteStatusScreen : public UIWindow {
public:
  SiteStatusScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  void command(const std::string& command, const std::string& arg) override;
private:
  std::string sitename;
  std::shared_ptr<Site> site;
  std::shared_ptr<SiteLogic> st;
  int confirmaction;
};
