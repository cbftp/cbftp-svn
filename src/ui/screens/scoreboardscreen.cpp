#include "scoreboardscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../resizableelement.h"

#include "../../globalcontext.h"
#include "../../scoreboard.h"
#include "../../engine.h"
#include "../../scoreboardelement.h"
#include "../../sitelogic.h"
#include "../../site.h"
#include "../../util.h"

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
  unsigned int y = 0;
  table.clear();
  Pointer<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;
  msotb = table.addTextButtonNoContent(y, 1, "filename", "FILE NAME");
  msal->addElement(msotb, 1, RESIZE_CUTEND, true);
  msotb = table.addTextButtonNoContent(y, 2, "sites", "SITES");
  msal->addElement(msotb, 2, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 3, "potential", "POTENTIAL");
  msal->addElement(msotb, 3, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 4, "score", "SCORE");
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  y++;
  std::vector<ScoreBoardElement *>::const_iterator it;
  for (it = scoreboard->begin(); it != scoreboard->end() && y < row; it++, y++) {
    std::string sites = (*it)->getSource()->getSite()->getName() + " -> " + (*it)->getDestination()->getSite()->getName();
    msal = table.addAdjustableLine();
    msotb = table.addTextButtonNoContent(y, 1, "filename", (*it)->fileName());
    msal->addElement(msotb, 1, RESIZE_CUTEND, true);
    msotb = table.addTextButtonNoContent(y, 2, "sites", sites);
    msal->addElement(msotb, 2, RESIZE_REMOVE);
    msotb = table.addTextButtonNoContent(y, 3, "potential", util::int2Str((*it)->getSource()->getPotential()));
    msal->addElement(msotb, 3, RESIZE_REMOVE);
    msotb = table.addTextButtonNoContent(y, 4, "score", util::int2Str((*it)->getScore()));
    msal->addElement(msotb, 4, RESIZE_REMOVE);
  }
  table.adjustLines(col - 3);
  bool highlight;
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<ResizableElement> re = table.getElement(i);
    highlight = false;
    if (table.getSelectionPointer() == i) {
      //highlight = true; // later problem
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
}

void ScoreBoardScreen::update() {
  redraw();
}

bool ScoreBoardScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case 10: // enter
    case 'c':
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string ScoreBoardScreen::getLegendText() const {
  return "[Esc/c/Enter] Return";
}

std::string ScoreBoardScreen::getInfoLabel() const {
  return "SCOREBOARD";
}

std::string ScoreBoardScreen::getInfoText() const {
  std::string size = util::int2Str(scoreboard->size());
  std::string max = util::int2Str((int)scoreboard->getElementVector().size());
  return "Size: " + size + "  Max: " + max;
}
