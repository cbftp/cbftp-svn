#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionTextArrow;
class ExternalFileViewing;
class LocalStorage;

class FileViewerSettingsScreen : public UIWindow {
public:
  FileViewerSettingsScreen(Ui *);
  ~FileViewerSettingsScreen();
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  Pointer<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  ExternalFileViewing * efv;
  LocalStorage * ls;
};
