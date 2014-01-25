#include "fileviewersettingsscreen.h"

#include "../uicommunicator.h"
#include "../termint.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextbutton.h"

#include "../../globalcontext.h"
#include "../../externalfileviewing.h"
#include "../../localstorage.h"

extern GlobalContext * global;

FileViewerSettingsScreen::FileViewerSettingsScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  active = false;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  efv = global->getExternalFileViewing();
  ls = global->getLocalStorage();
  unsigned int y = 1;
  unsigned int x = 1;
  mso.addStringField(y++, x, "temppath", "Temporary download path:", ls->getTempPath(), false, 128, 128);
  y++;
  mso.addStringField(y++, x, "video", "Video viewer:", efv->getVideoViewer(), false);
  mso.addStringField(y++, x, "audio", "Audio viewer:", efv->getAudioViewer(), false);
  mso.addStringField(y++, x, "image", "Image viewer:", efv->getImageViewer(), false);
  mso.addStringField(y++, x, "pdf", "PDF viewer:", efv->getPDFViewer(), false);
  mso.makeLeavableDown();
  mso.enterFocusFrom(0);
  init(window, row, col);
}

void FileViewerSettingsScreen::redraw() {
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

void FileViewerSettingsScreen::update() {
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

void FileViewerSettingsScreen::keyPressed(unsigned int ch) {
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
      if (mso.goUp()) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_DOWN:
      if (mso.goDown()) {
        uicommunicator->newCommand("update");
      }
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
    case 'd':
      for(unsigned int i = 0; i < mso.size(); i++) {
        MenuSelectOptionElement * msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "temppath") {
          ls->setTempPath(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "video") {
          efv->setVideoViewer(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "audio") {
          efv->setAudioViewer(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "image") {
          efv->setImageViewer(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "pdf") {
          efv->setPDFViewer(((MenuSelectOptionTextField *)msoe)->getData());
        }
      }
      uicommunicator->newCommand("return");
      break;
    case 27: // esc
    case 'c':
      uicommunicator->newCommand("return");
      break;
  }
}

std::string FileViewerSettingsScreen::getLegendText() {
  return currentlegendtext;
}

std::string FileViewerSettingsScreen::getInfoLabel() {
  return "EXTERNAL FILE VIEWER SETTINGS";
}

std::string FileViewerSettingsScreen::getInfoText() {
  return "";
}
