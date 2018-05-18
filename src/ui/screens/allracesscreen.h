#pragma once

#include <map>
#include <string>

#include "../../core/pointer.h"

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
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  static void addRaceTableHeader(unsigned int, MenuSelectOption &, const std::string &);
  static void addRaceDetails(unsigned int, MenuSelectOption &, Pointer<Race>);
private:
  static void addRaceTableRow(unsigned int, MenuSelectOption &, unsigned int, bool, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);
  MenuSelectOption table;
  Engine * engine;
  bool hascontents;
  unsigned int currentviewspan;
  unsigned int ypos;
  int temphighlightline;
};
