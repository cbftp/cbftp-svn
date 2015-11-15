#include "fileviewersettingsscreen.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextbutton.h"

#include "../../globalcontext.h"
#include "../../externalfileviewing.h"
#include "../../localstorage.h"

extern GlobalContext * global;

FileViewerSettingsScreen::FileViewerSettingsScreen(Ui * ui) {
  this->ui = ui;
}

FileViewerSettingsScreen::~FileViewerSettingsScreen() {

}

void FileViewerSettingsScreen::initialize(unsigned int row, unsigned int col) {
  active = false;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  efv = global->getExternalFileViewing();
  ls = global->getLocalStorage();
  unsigned int y = 1;
  unsigned int x = 1;
  mso.clear();
  mso.addStringField(y++, x, "temppath", "Temporary download path:", ls->getTempPath(), false, 128, 128);
  y++;
  mso.addStringField(y++, x, "video", "Video viewer:", efv->getVideoViewer(), false);
  mso.addStringField(y++, x, "audio", "Audio viewer:", efv->getAudioViewer(), false);
  mso.addStringField(y++, x, "image", "Image viewer:", efv->getImageViewer(), false);
  mso.addStringField(y++, x, "pdf", "PDF viewer:", efv->getPDFViewer(), false);
  mso.makeLeavableDown();
  mso.enterFocusFrom(0);
  init(row, col);
}

void FileViewerSettingsScreen::redraw() {
  ui->erase();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void FileViewerSettingsScreen::update() {
  Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    ui->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool FileViewerSettingsScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      if (mso.goUp()) {
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (mso.goDown()) {
        ui->update();
      }
      return true;
    case 10:
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      return true;
    case 'd':
      for(unsigned int i = 0; i < mso.size(); i++) {
        Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "temppath") {
          ls->setTempPath(msoe.get<MenuSelectOptionTextField>()->getData());
        }
        else if (identifier == "video") {
          efv->setVideoViewer(msoe.get<MenuSelectOptionTextField>()->getData());
        }
        else if (identifier == "audio") {
          efv->setAudioViewer(msoe.get<MenuSelectOptionTextField>()->getData());
        }
        else if (identifier == "image") {
          efv->setImageViewer(msoe.get<MenuSelectOptionTextField>()->getData());
        }
        else if (identifier == "pdf") {
          efv->setPDFViewer(msoe.get<MenuSelectOptionTextField>()->getData());
        }
      }
      ui->returnToLast();
      return true;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string FileViewerSettingsScreen::getLegendText() const {
  return currentlegendtext;
}

std::string FileViewerSettingsScreen::getInfoLabel() const {
  return "EXTERNAL FILE VIEWER SETTINGS";
}
