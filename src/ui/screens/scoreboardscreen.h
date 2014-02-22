#pragma once

#include "../uiwindow.h"

class UICommunicator;
class Engine;
class ScoreBoard;

class ScoreBoardScreen : public UIWindow {
public:
  ScoreBoardScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
  std::string getInfoText();
private:
  UICommunicator * uicommunicator;
  Engine * engine;
  ScoreBoard * scoreboard;
};
