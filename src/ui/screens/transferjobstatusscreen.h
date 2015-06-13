#pragma once

#include <string>
#include <map>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../pointer.h"

class TransferJob;
class Ui;
class TransferStatus;
class MenuSelectOptionElement;

class TransferJobStatusScreen : public UIWindow {
public:
  TransferJobStatusScreen(Ui *);
  ~TransferJobStatusScreen();
  void initialize(unsigned int, unsigned int, std::string);
  void redraw();
  void update();
  void command(std::string, std::string);
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  static std::string getRoute(Pointer<TransferJob>);
private:
  void addTransferDetails(unsigned int, Pointer<TransferStatus>);
  void addTransferDetails(unsigned int, std::string, std::string, std::string,
      std::string, std::string, std::string, int);
  Pointer<TransferJob> transferjob;
  std::string filename;
  MenuSelectOption table;
  MenuSelectOption mso;
  std::map<Pointer<MenuSelectOptionElement>, int> progressmap;
  bool active;
  Pointer<MenuSelectOptionElement> activeelement;
  std::string defaultlegendtext;
  std::string currentlegendtext;
  std::string abortedlegendtext;
};
