#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionTextArrow;
class LocalStorage;

class FileViewerSettingsScreen : public UIWindow {
public:
  FileViewerSettingsScreen(Ui *);
  ~FileViewerSettingsScreen();
  void initialize(unsigned int, unsigned int);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getInfoLabel() const override;
private:
  MenuSelectOption mso;
  LocalStorage * ls;
};
