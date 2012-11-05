#include "addsectionscreen.h"

AddSectionScreen::AddSectionScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  unsigned int y = 3;
  unsigned int x = 1;
  modsite = global->getSiteManager()->getSite(uicommunicator->getArg1());
  mso.addStringField(y++, x, "name", "Name:", "", false);
  mso.addStringField(y++, x, "path", "Path:", uicommunicator->getArg2(), false);
  init(window, row, col);
}

void AddSectionScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "-== ADD SECTION ==-");
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void AddSectionScreen::update() {
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  wattron(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  wattroff(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    curs_set(1);
    TermInt::moveCursor(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void AddSectionScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      uicommunicator->newCommand("updatesetlegend");
      return;
    }
    activeelement->inputChar(ch);
    uicommunicator->newCommand("update");
    return;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      mso.goUp();
      uicommunicator->newCommand("update");
      break;
    case KEY_DOWN:
      mso.goDown();
      uicommunicator->newCommand("update");
      break;
    case 10:

      activation = mso.getElement(mso.getSelectionPointer())->activate();
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 'd':
      MenuSelectOptionTextField * field1 = (MenuSelectOptionTextField *)mso.getElement(0);
      MenuSelectOptionTextField * field2 = (MenuSelectOptionTextField *)mso.getElement(1);
      std::string name = field1->getData();
      std::string path = field2->getData();
      modsite->addSection(name, path);
      uicommunicator->newCommand("return");
      break;
  }
}

std::string AddSectionScreen::getLegendText() {
  return currentlegendtext;
}
