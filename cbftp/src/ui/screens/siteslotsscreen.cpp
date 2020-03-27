#include "siteslotsscreen.h"

#include "../../site.h"

#include "../ui.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptionelement.h"

SiteSlotsScreen::SiteSlotsScreen(Ui* ui) : UIWindow(ui, "SiteSlotsScreen") {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

SiteSlotsScreen::~SiteSlotsScreen() {

}

void SiteSlotsScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site) {
  active = false;
  modsite = site;
  unsigned int y = 4;
  unsigned int x = 1;
  mso.reset();
  mso.addIntArrow(y++, x, "logins", "Login slots:", modsite->getInternMaxLogins(), 0, 99);
  mso.addIntArrow(y++, x, "maxup", "Upload slots:", modsite->getInternMaxUp(), 0, 99);
  mso.addIntArrow(y++, x, "maxdn", "Download slots:", modsite->getInternMaxDown(), 0, 99);
  y++;
  mso.addCheckBox(y++, x, "freeslot", "Leave one slot free:", modsite->getLeaveFreeSlot());
  mso.addIntArrow(y++, x, "maxdnpre", "Download slots on download-only spread jobs:", modsite->getInternMaxDownPre(), 0, 99);
  mso.addIntArrow(y++, x, "maxdncomplete", "Download slots on complete spread jobs:", modsite->getInternMaxDownComplete(), 0, 99);
  mso.addIntArrow(y++, x, "maxdntransferjob", "Download slots on transfer jobs:", modsite->getInternMaxDownTransferJob(), 0, 99);
  mso.enterFocusFrom(0);
  init(row, col);
}

void SiteSlotsScreen::redraw() {
  ui->erase();
  ui->printStr(1, 1, "Setting the download/upload slots counters to 0 means same as the number of logins.");
  ui->printStr(2, 1, "Setting the special download slots counters to 0 means same as the number of download slots.");
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void SiteSlotsScreen::update() {
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
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

bool SiteSlotsScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      ui->setLegend();
      ui->update();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(action) {
    case KEYACTION_UP:
      mso.goUp();
      ui->update();
      return true;
    case KEYACTION_DOWN:
      mso.goDown();
      ui->update();
      return true;
    case KEYACTION_ENTER:
    {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      activation = msoe->activate();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->setLegend();
      ui->update();
      return true;
    }
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DONE:
    {
      std::shared_ptr<MenuSelectOptionNumArrow> logins = std::static_pointer_cast<MenuSelectOptionNumArrow>(mso.getElement("logins"));
      std::shared_ptr<MenuSelectOptionNumArrow> maxup = std::static_pointer_cast<MenuSelectOptionNumArrow>(mso.getElement("maxup"));
      std::shared_ptr<MenuSelectOptionNumArrow> maxdn = std::static_pointer_cast<MenuSelectOptionNumArrow>(mso.getElement("maxdn"));
      std::shared_ptr<MenuSelectOptionNumArrow> maxdnpre = std::static_pointer_cast<MenuSelectOptionNumArrow>(mso.getElement("maxdnpre"));
      std::shared_ptr<MenuSelectOptionNumArrow> maxdncomplete = std::static_pointer_cast<MenuSelectOptionNumArrow>(mso.getElement("maxdncomplete"));
      std::shared_ptr<MenuSelectOptionNumArrow> maxdntransferjob = std::static_pointer_cast<MenuSelectOptionNumArrow>(mso.getElement("maxdntransferjob"));
      std::shared_ptr<MenuSelectOptionCheckBox> freeslot = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("freeslot"));
      modsite->setMaxLogins(logins->getData());
      modsite->setMaxUp(maxup->getData());
      modsite->setMaxDn(maxdn->getData());
      modsite->setMaxDnPre(maxdnpre->getData());
      modsite->setMaxDnComplete(maxdncomplete->getData());
      modsite->setMaxDnTransferJob(maxdntransferjob->getData());
      modsite->setLeaveFreeSlot(freeslot->getData());
      ui->returnToLast();
      return true;
    }
  }
  return false;
}

std::string SiteSlotsScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string SiteSlotsScreen::getInfoLabel() const {
  return "ADVANCED SLOT SETTINGS FOR: "  + modsite->getName();
}
