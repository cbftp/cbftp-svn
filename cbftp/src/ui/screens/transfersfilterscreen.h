#pragma once

#include <list>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../core/pointer.h"

class TransferManager;
class TransferStatus;
class MenuSelectOptionElement;
class CommandOwner;
class Race;
class TransferJob;
class TransferFilteringParameters;

class TransfersFilterScreen : public UIWindow {
public:
  TransfersFilterScreen(Ui *);
  ~TransfersFilterScreen();
  void initialize(unsigned int, unsigned int, const TransferFilteringParameters &);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  void command(const std::string & command, const std::string & arg);
private:
  std::string getSpreadJobsText() const;
  std::string getTransferJobsText() const;
  MenuSelectOption mso;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  Pointer<MenuSelectOptionElement> activeelement;
  std::list<std::string> selectedspreadjobs;
  std::list<std::string> selectedtransferjobs;
};
