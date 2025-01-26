#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class CommandOwner;
struct SpreadJobsFilteringParameters;

class SpreadJobsFilterScreen : public UIWindow {
public:
	SpreadJobsFilterScreen(Ui* ui);
  ~SpreadJobsFilterScreen();
  void initialize(unsigned int row, unsigned int col, const SpreadJobsFilteringParameters& sjfp);
  void redraw() override;
  bool keyPressed(unsigned int ch) override;
  std::string getInfoLabel() const override;
  void command(const std::string& command, const std::string& arg) override;
private:
  bool onDeactivated(const std::shared_ptr<MenuSelectOptionElement>& msoe) override;
  MenuSelectOption mso;
};
