#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Engine;
class ScoreBoard;

class ScoreBoardScreen : public UIWindow {
public:
  ScoreBoardScreen(Ui *);
  ~ScoreBoardScreen();
  void initialize(unsigned int, unsigned int);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
private:
  Engine * engine;
  std::shared_ptr<ScoreBoard> scoreboard;
  MenuSelectOption table;
  unsigned int currentviewspan;
};
