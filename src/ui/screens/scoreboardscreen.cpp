#include "scoreboardscreen.h"

#include "../uicommunicator.h"
#include "../termint.h"

#include "../../globalcontext.h"
#include "../../scoreboard.h"
#include "../../engine.h"
#include "../../scoreboardelement.h"
#include "../../sitelogic.h"
#include "../../site.h"

extern GlobalContext * global;

ScoreBoardScreen::ScoreBoardScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  engine = global->getEngine();
  scoreboard = engine->getScoreBoard();
  autoupdate = true;
  init(window, row, col);
}

void ScoreBoardScreen::redraw() {
  werase(window);
  std::vector<ScoreBoardElement *>::iterator it;
  unsigned int i = 2;
  TermInt::printStr(window, 0, 1, "Filename");
  TermInt::printStr(window, 1, 1, "--------");
  TermInt::printStr(window, 0, col - 7, "Score");
  TermInt::printStr(window, 1, col - 7, "-----");
  TermInt::printStr(window, 0, col - 21, "Sites");
  TermInt::printStr(window, 1, col - 21, "-----");
  for (it = scoreboard->begin(); it != scoreboard->end() && i < row; it++, i++) {
    std::string score = global->int2Str((*it)->getScore());
    std::string src = (*it)->getSource()->getSite()->getName();
    if (src.length() > 4) {
      src = src.substr(0, 4);
    }
    std::string dst = (*it)->getDestination()->getSite()->getName();
    if (dst.length() > 4) {
      dst = dst.substr(0, 4);
    }
    TermInt::printStr(window, i, 1, (*it)->fileName());
    TermInt::printStr(window, i, col - 21, src + " -> " + dst);
    TermInt::printStr(window, i, col - score.length() - 2, score);
  }
}

void ScoreBoardScreen::update() {
  redraw();
}

void ScoreBoardScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case 10: // enter
    case 'c':
      uicommunicator->newCommand("return");
      break;
  }
}

std::string ScoreBoardScreen::getLegendText() {
  return "[Esc/c/Enter] Return";
}

std::string ScoreBoardScreen::getInfoLabel() {
  return "SCOREBOARD";
}

std::string ScoreBoardScreen::getInfoText() {
  std::string size = global->int2Str(scoreboard->size());
  std::string max = global->int2Str((int)scoreboard->getElementVector()->size());
  return "Size: " + size + "  Max: " + max;
}
