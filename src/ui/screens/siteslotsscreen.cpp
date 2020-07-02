#include "siteslotsscreen.h"

#include "../../site.h"

#include "../ui.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextarrow.h"
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
  std::shared_ptr<MenuSelectOptionTextArrow> logins = mso.addTextArrow(y++, x, "logins", "Login slots:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxup = mso.addTextArrow(y++, x, "maxup", "Upload slots:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxdn = mso.addTextArrow(y++, x, "maxdn", "Download slots:");
  y++;
  mso.addCheckBox(y++, x, "freeslot", "Leave one slot free:", modsite->getLeaveFreeSlot());
  std::shared_ptr<MenuSelectOptionTextArrow> maxdnpre = mso.addTextArrow(y++, x, "maxdnpre", "Download slots on download-only spread jobs:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxdncomplete = mso.addTextArrow(y++, x, "maxdncomplete", "Download slots on complete spread jobs:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxdntransferjob = mso.addTextArrow(y++, x, "maxdntransferjob", "Download slots on transfer jobs:");
  logins->addOption("Many", -1);
  maxup->addOption("All", -1);
  maxdn->addOption("All", -1);
  maxdnpre->addOption("All", -1);
  maxdncomplete->addOption("All", -1);
  maxdntransferjob->addOption("All", -1);
  maxdnpre->addOption("Normal", -2);
  maxdncomplete->addOption("Normal", -2);
  maxdntransferjob->addOption("Normal", -2);
  for (unsigned int i = 0; i < 100; ++i) {
    std::string num = std::to_string(i);
    if (i > 0) {
      logins->addOption(num, i);
    }
    maxup->addOption(num, i);
    maxdn->addOption(num, i);
    maxdnpre->addOption(num, i);
    maxdncomplete->addOption(num, i);
    maxdntransferjob->addOption(num, i);
  }
  logins->setOption(modsite->getInternMaxLogins());
  maxup->setOption(modsite->getInternMaxUp());
  maxdn->setOption(modsite->getInternMaxDown());
  maxdnpre->setOption(modsite->getInternMaxDownPre());
  maxdncomplete->setOption(modsite->getInternMaxDownComplete());
  maxdntransferjob->setOption(modsite->getInternMaxDownTransferJob());
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
      std::shared_ptr<MenuSelectOptionTextArrow> logins = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("logins"));
      std::shared_ptr<MenuSelectOptionTextArrow> maxup = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxup"));
      std::shared_ptr<MenuSelectOptionTextArrow> maxdn = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxdn"));
      std::shared_ptr<MenuSelectOptionTextArrow> maxdnpre = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxdnpre"));
      std::shared_ptr<MenuSelectOptionTextArrow> maxdncomplete = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxdncomplete"));
      std::shared_ptr<MenuSelectOptionTextArrow> maxdntransferjob = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxdntransferjob"));
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
