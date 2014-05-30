#pragma once

#include "../uiwindow.h"

class Engine;
class ScoreBoard;

class ScoreBoardScreen : public UIWindow {
public:
  ScoreBoardScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
  std::string getInfoText();
private:
  Engine * engine;
  ScoreBoard * scoreboard;
};
