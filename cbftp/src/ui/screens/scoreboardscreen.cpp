#include "scoreboardscreen.h"

#include "../ui.h"

#include "../../globalcontext.h"
#include "../../scoreboard.h"
#include "../../engine.h"
#include "../../scoreboardelement.h"
#include "../../sitelogic.h"
#include "../../site.h"
#include "../../util.h"

extern GlobalContext * global;

ScoreBoardScreen::ScoreBoardScreen(Ui * ui) {
  this->ui = ui;
}

ScoreBoardScreen::~ScoreBoardScreen() {

}

void ScoreBoardScreen::initialize(unsigned int row, unsigned int col) {
  engine = global->getEngine();
  scoreboard = engine->getScoreBoard();
  autoupdate = true;
  init(row, col);
}

void ScoreBoardScreen::redraw() {
  ui->erase();
  std::vector<ScoreBoardElement *>::const_iterator it;
  unsigned int i = 2;
  ui->printStr(0, 1, "Filename");
  ui->printStr(1, 1, "--------");
  ui->printStr(0, col - 7, "Score");
  ui->printStr(1, col - 7, "-----");
  ui->printStr(0, col - 21, "Sites");
  ui->printStr(1, col - 21, "-----");
  for (it = scoreboard->begin(); it != scoreboard->end() && i < row; it++, i++) {
    std::string score = util::int2Str((*it)->getScore());
    std::string src = (*it)->getSource()->getSite()->getName();
    if (src.length() > 4) {
      src = src.substr(0, 4);
    }
    std::string dst = (*it)->getDestination()->getSite()->getName();
    if (dst.length() > 4) {
      dst = dst.substr(0, 4);
    }
    ui->printStr(i, 1, (*it)->fileName());
    ui->printStr(i, col - 21, src + " -> " + dst);
    ui->printStr(i, col - score.length() - 2, score);
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
      ui->returnToLast();
      break;
  }
}

std::string ScoreBoardScreen::getLegendText() const {
  return "[Esc/c/Enter] Return";
}

std::string ScoreBoardScreen::getInfoLabel() const {
  return "SCOREBOARD";
}

std::string ScoreBoardScreen::getInfoText() const {
  std::string size = util::int2Str(scoreboard->size());
  std::string max = util::int2Str((int)scoreboard->getElementVector()->size());
  return "Size: " + size + "  Max: " + max;
}
