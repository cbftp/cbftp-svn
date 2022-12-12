#pragma once

#include <list>
#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class TransferManager;
class TransferStatus;
class MenuSelectOptionElement;
class CommandOwner;
class Race;
class TransferJob;
struct TransferFilteringParameters;

class TransfersFilterScreen : public UIWindow {
public:
  TransfersFilterScreen(Ui *);
  ~TransfersFilterScreen();
  void initialize(unsigned int, unsigned int, const TransferFilteringParameters &);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  void command(const std::string & command, const std::string & arg) override;
private:
  std::string getSpreadJobsText() const;
  std::string getTransferJobsText() const;
  MenuSelectOption mso;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  std::list<std::string> selectedspreadjobs;
  std::list<std::string> selectedtransferjobs;
};
