#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionTextArrow;
class ExternalFileViewing;
class LocalStorage;

class FileViewerSettingsScreen : public UIWindow {
public:
  FileViewerSettingsScreen(Ui *);
  void initialize(unsigned int, unsigned int);
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
  ExternalFileViewing * efv;
  LocalStorage * ls;
};
