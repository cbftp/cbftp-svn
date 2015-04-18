#pragma once

#include <map>
#include <string>

#include "../../pointer.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Engine;
class Race;

class AllRacesScreen : public UIWindow {
public:
  AllRacesScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  static void addRaceTableHeader(unsigned int, MenuSelectOption &, std::string);
  static void addRaceDetails(unsigned int, MenuSelectOption &, Pointer<Race>);
private:
  static void addRaceTableRow(unsigned int, MenuSelectOption &, bool, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string);
  MenuSelectOption table;
  Engine * engine;
  bool hascontents;
};
