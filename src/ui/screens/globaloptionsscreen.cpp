#include "globaloptionsscreen.h"

#include "../../globalcontext.h"
#include "../../remotecommandhandler.h"
#include "../../sitemanager.h"

#include "../uicommunicator.h"
#include "../menuselectoptionelement.h"
#include "../focusablearea.h"
#include "../termint.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionnumarrow.h"

extern GlobalContext * global;

GlobalOptionsScreen::GlobalOptionsScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  unsigned int y = 1;
  unsigned int x = 1;
  rch = global->getRemoteCommandHandler();
  sm = global->getSiteManager();
  mso.addCheckBox(y++, x, "udpenable", "Enable remote commands:", rch->isEnabled());
  mso.addStringField(y++, x, "udpport", "Remote command UDP Port:", global->int2Str(rch->getUDPPort()), false, 5);
  mso.addStringField(y++, x, "udppass", "Remote command password:", rch->getPassword(), true);
  y++;
  mso.addCheckBox(y++, x, "legend", "Show legend bar:", global->getUICommunicator()->legendEnabled());
  y++;
  mso.addStringField(y++, x, "defuser", "Default site username:", sm->getDefaultUserName(), false);
  mso.addStringField(y++, x, "defpass", "Default site password:", sm->getDefaultPassword(), true);
  mso.addIntArrow(y++, x, "deflogins", "Default site login slots:", sm->getDefaultMaxLogins(), 0, 99);
  mso.addIntArrow(y++, x, "defmaxup", "Default site upload slots:", sm->getDefaultMaxUp(), 0, 99);
  mso.addIntArrow(y++, x, "defmaxdn", "Default site download slots:", sm->getDefaultMaxDown(), 0, 99);
  mso.addCheckBox(y++, x, "defsslconn", "Default site AUTH SSL:", sm->getDefaultSSL());
  mso.addCheckBox(y++, x, "defforcesslfxp", "Default site forced SSL FXP:", sm->getDefaultSSLFXPForced());
  mso.addStringField(y++, x, "defidletime", "Default site max idle time (s):", global->int2Str(sm->getDefaultMaxIdleTime()), false);
  y++;
  mso.addTextButtonNoContent(y++, x, "skiplist", "Configure skiplist...");
  mso.addTextButtonNoContent(y++, x, "changekey", "Change encryption key...");
  init(window, row, col);
}

void GlobalOptionsScreen::redraw() {
  werase(window);
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

void GlobalOptionsScreen::update() {
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

void GlobalOptionsScreen::keyPressed(unsigned int ch) {
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
  MenuSelectOptionElement * msoe;
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
      msoe = mso.getElement(mso.getSelectionPointer());
      if (msoe->getIdentifier() == "skiplist") {
        uicommunicator->newCommand("skiplist");
        return;
      }
      if (msoe->getIdentifier() == "changekey") {
        uicommunicator->newCommand("changekey");
        return;
      }
      activation = msoe->activate();
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = msoe;
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 27: // esc
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 'd':
      bool udpenable = false;
      for(unsigned int i = 0; i < mso.size(); i++) {
        MenuSelectOptionElement * msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "udpenable") {
          udpenable = (((MenuSelectOptionCheckBox *)msoe)->getData());
          if (rch->isEnabled() && !udpenable) {
            rch->setEnabled(false);
          }
        }
        else if (identifier == "udpport") {
          rch->setPort(global->str2Int(((MenuSelectOptionTextField *)msoe)->getData()));
        }
        else if (identifier == "udppass") {
          rch->setPassword(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "defuser") {
          sm->setDefaultUserName(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "defpass") {
          sm->setDefaultPassword(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "deflogins") {
          sm->setDefaultMaxLogins(((MenuSelectOptionNumArrow *)msoe)->getData());
        }
        else if (identifier == "defmaxup") {
          sm->setDefaultMaxUp(((MenuSelectOptionNumArrow *)msoe)->getData());
        }
        else if (identifier == "defmaxdn") {
          sm->setDefaultMaxDown(((MenuSelectOptionNumArrow *)msoe)->getData());
        }
        else if (identifier == "defsslconn") {
          sm->setDefaultSSL(((MenuSelectOptionCheckBox *)msoe)->getData());
        }
        else if (identifier == "defforcesslfxp") {
          sm->setDefaultSSLFXPForced(((MenuSelectOptionCheckBox *)msoe)->getData());
        }
        else if (identifier == "defidletime") {
          sm->setDefaultMaxIdleTime(global->str2Int(((MenuSelectOptionTextField *)msoe)->getData()));
        }
        else if (identifier == "legend") {
          global->getUICommunicator()->showLegend(((MenuSelectOptionCheckBox *)msoe)->getData());
        }
      }
      rch->setEnabled(udpenable);
      uicommunicator->newCommand("return");
      break;
  }
}

std::string GlobalOptionsScreen::getLegendText() {
  return currentlegendtext;
}

std::string GlobalOptionsScreen::getInfoLabel() {
  return "GLOBAL SETTINGS";
}
