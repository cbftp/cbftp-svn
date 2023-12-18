#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;
class CommandOwner;
struct SpreadJobsFilteringParameters;

class SpreadJobsFilterScreen : public UIWindow {
public:
	SpreadJobsFilterScreen(Ui* ui);
  ~SpreadJobsFilterScreen();
  void initialize(unsigned int row, unsigned int col, const SpreadJobsFilteringParameters& sjfp);
  void redraw() override;
  bool keyPressed(unsigned int ch) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  void command(const std::string& command, const std::string& arg) override;
private:
  MenuSelectOption mso;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
};
