#pragma once

#include "../../core/pointer.h"

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Engine;
class ScoreBoard;

class ScoreBoardScreen : public UIWindow {
public:
  ScoreBoardScreen(Ui *);
  ~ScoreBoardScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  Engine * engine;
  Pointer<ScoreBoard> scoreboard;
  MenuSelectOption table;
  unsigned int currentviewspan;
};
