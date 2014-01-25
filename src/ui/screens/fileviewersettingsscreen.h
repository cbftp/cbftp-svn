#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class UICommunicator;
class MenuSelectOptionTextArrow;
class ExternalFileViewing;
class LocalStorage;

class FileViewerSettingsScreen : public UIWindow {
public:
  FileViewerSettingsScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
  std::string getInfoText();
private:
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  UICommunicator * uicommunicator;
  ExternalFileViewing * efv;
  LocalStorage * ls;
};
