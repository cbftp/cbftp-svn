#include "browsescreensub.h"

#include "../resizableelement.h"
#include "../menuselectoption.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../ui.h"

void BrowseScreenSub::command(const std::string & command, const std::string & arg) {

}

UIFileList * BrowseScreenSub::getUIFileList() {
  return nullptr;
}

void BrowseScreenSub::printFlipped(Ui * ui, const std::shared_ptr<ResizableElement> & re) {
  int flipper = 0;
  for (unsigned int i = 0; i < re->getLabelText().length(); i++) {
    ui->printChar(re->getRow(), re->getCol() + i, re->getLabelText()[i], flipper++ % 2);
  }
}

void BrowseScreenSub::addFileDetails(MenuSelectOption & table, unsigned int coloffset, unsigned int y, const std::string & name, const std::string & prepchar, const std::string & size, const std::string & lastmodified, const std::string & owner, bool selectable, bool cursored, UIFile * origin) {
  std::shared_ptr<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;
  msotb = table.addTextButtonNoContent(y, coloffset + 1, "prepchar", prepchar);
  msotb->setSelectable(false);
  msotb->setShortSpacing();
  msal->addElement(msotb, 4, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, coloffset + 3, "name", name);
  if (!selectable) {
    msotb->setSelectable(false);
  }
  msotb->setOrigin(origin);
  if (cursored) {
    table.setPointer(msotb);
  }
  msal->addElement(msotb, 5, 0, RESIZE_WITHDOTS, true);
  msotb = table.addTextButtonNoContent(y, coloffset + 3, "size", size);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 3, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, coloffset + 3, "lastmodified", lastmodified);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, coloffset + 3, "owner", owner);
  msotb->setSelectable(false);
  msal->addElement(msotb, 1, RESIZE_REMOVE);
}

std::string BrowseScreenSub::targetName(const std::list<std::pair<std::string, bool>> & files) {
  std::string target;
  int total = files.size();
  if (total == 1) {
    target = files.front().first;
  }
  else {
    target = std::to_string(total) + " items";
  }
  return target;
}
