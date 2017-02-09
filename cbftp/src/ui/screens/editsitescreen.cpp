#include "editsitescreen.h"

#include <set>

#include "../../globalcontext.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../proxymanager.h"
#include "../../proxy.h"
#include "../../util.h"
#include "../../settingsloadersaver.h"

#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncontainer.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextarrow.h"

EditSiteScreen::EditSiteScreen(Ui * ui) {
  this->ui = ui;
}

EditSiteScreen::~EditSiteScreen() {

}

void EditSiteScreen::initialize(unsigned int row, unsigned int col, const std::string & operation, const std::string & site) {
  active = false;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one, save changes - [c]ancel, undo changes";
  currentlegendtext = defaultlegendtext;
  this->operation = operation;
  SiteManager * sm = global->getSiteManager();
  std::list<Pointer<Site> > exceptsrclist;
  std::list<Pointer<Site> > exceptdstlist;
  std::string exceptsrc = "";
  std::string exceptdst = "";
  if (operation == "add") {
    this->site = makePointer<Site>("SUNET");
    this->site->setUser(sm->getDefaultUserName());
    this->site->setPass(sm->getDefaultPassword());
    this->site->setMaxLogins(sm->getDefaultMaxLogins());
    this->site->setMaxUp(sm->getDefaultMaxUp());
    this->site->setMaxDn(sm->getDefaultMaxDown());
    this->site->setSSL(sm->getDefaultSSL());
    this->site->setSSLTransferPolicy(sm->getDefaultSSLTransferPolicy());
    this->site->setMaxIdleTime(sm->getDefaultMaxIdleTime());
    std::vector<Pointer<Site> >::const_iterator it;
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      if ((*it)->getTransferTargetPolicy() == SITE_TRANSFER_POLICY_BLOCK) {
        exceptsrc += (*it)->getName() + ",";
      }
      if ((*it)->getTransferSourcePolicy() == SITE_TRANSFER_POLICY_BLOCK) {
        exceptdst += (*it)->getName() + ",";
      }
    }
  }
  else if (operation == "edit") {
    this->site = global->getSiteManager()->getSite(site);
    std::set<Pointer<Site> >::const_iterator it;
    for (it = this->site->exceptSourceSitesBegin(); it != this->site->exceptSourceSitesEnd(); it++) {
      exceptsrc += (*it)->getName() + ",";
    }
    for (it = this->site->exceptTargetSitesBegin(); it != this->site->exceptTargetSitesEnd(); it++) {
      exceptdst += (*it)->getName() + ",";
    }
  }
  if (exceptsrc.length()) {
    exceptsrc = exceptsrc.substr(0, exceptsrc.length() - 1);
  }
  if (exceptdst.length()) {
    exceptdst = exceptdst.substr(0, exceptdst.length() - 1);
  }
  std::string affilstr = "";
  std::set<std::string>::const_iterator it;
  for (it = this->site->affilsBegin(); it != this->site->affilsEnd(); it++) {
    affilstr += *it + " ";
  }
  if (affilstr.length() > 0) {
    affilstr = affilstr.substr(0, affilstr.length() - 1);
  }

  unsigned int y = 1;
  unsigned int x = 1;

  mso.reset();
  mso.addStringField(y++, x, "name", "Name:", this->site->getName(), false);
  Pointer<MenuSelectOptionTextField> msotf = mso.addStringField(y++, x, "addr", "Address:", this->site->getAddressesAsString(), false, 48, 512);
  msotf->setExtraLegendText("Multiple sets of address:port separated by space or semicolon");
  mso.addStringField(y++, x, "user", "Username:", this->site->getUser(), false);
  mso.addStringField(y++, x, "pass", "Password:", this->site->getPass(), true);
  mso.addIntArrow(y++, x, "logins", "Login slots:", this->site->getInternMaxLogins(), 0, 99);
  mso.addIntArrow(y++, x, "maxup", "Upload slots:", this->site->getInternMaxUp(), 0, 99);
  mso.addIntArrow(y++, x, "maxdn", "Download slots:", this->site->getInternMaxDown(), 0, 99);
  mso.addStringField(y++, x, "basepath", "Base path:", this->site->getBasePath().toString(), false, 32, 512);
  Pointer<MenuSelectOptionTextArrow> listcommand = mso.addTextArrow(y++, x, "listcommand", "List command:");
  listcommand->addOption("STAT -l", SITE_LIST_STAT);
  listcommand->addOption("LIST", SITE_LIST_LIST);
  listcommand->setOption(this->site->getListCommand());
  mso.addStringField(y++, x, "idletime", "Max idle time (s):", util::int2Str(this->site->getMaxIdleTime()), false);
  mso.addCheckBox(y++, x, "ssl", "AUTH SSL:", this->site->SSL());
  Pointer<MenuSelectOptionTextArrow> sslfxp = mso.addTextArrow(y++, x, "ssltransfer", "SSL transfers:");
  sslfxp->addOption("Always off", SITE_SSL_ALWAYS_OFF);
  sslfxp->addOption("Prefer off", SITE_SSL_PREFER_OFF);
  sslfxp->addOption("Prefer on", SITE_SSL_PREFER_ON);
  sslfxp->addOption("Always on", SITE_SSL_ALWAYS_ON);
  sslfxp->setOption(this->site->getSSLTransferPolicy());
  mso.addCheckBox(y++, x, "cpsv", "CPSV supported:", this->site->supportsCPSV());
  mso.addCheckBox(y++, x, "pret", "Needs PRET:", this->site->needsPRET());
  mso.addCheckBox(y++, x, "binary", "Force binary mode:", this->site->forceBinaryMode());
  mso.addCheckBox(y++, x, "brokenpasv", "Broken PASV:", this->site->hasBrokenPASV());
  Pointer<MenuSelectOptionTextArrow> useproxy = mso.addTextArrow(y++, x, "useproxy", "Proxy:");
  ProxyManager * pm = global->getProxyManager();
  Proxy * proxy = pm->getDefaultProxy();
  std::string globalproxyname = "None";
  if (proxy != NULL) {
    globalproxyname = proxy->getName();
  }
  useproxy->addOption("(Global) " + globalproxyname, SITE_PROXY_GLOBAL);
  useproxy->addOption("None", SITE_PROXY_NONE);
  for (std::vector<Proxy *>::const_iterator it = pm->begin(); it != pm->end(); it++) {
    useproxy->addOption((*it)->getName(), SITE_PROXY_USE);
  }
  int proxytype = this->site->getProxyType();
  useproxy->setOption(proxytype);
  if (proxytype == SITE_PROXY_USE) {
    useproxy->setOptionText(this->site->getProxy());
  }
  mso.addTextButtonNoContent(y++, x, "skiplist", "Configure skiplist...");
  y++;
  mso.addCheckBox(y++, x, "disabled", "Disabled:", this->site->getDisabled());
  mso.addCheckBox(y++, x, "allowupload", "Allow upload:", this->site->getAllowUpload());
  mso.addCheckBox(y++, x, "allowdownload", "Allow download:", this->site->getAllowDownload());
  Pointer<MenuSelectOptionTextArrow> priority = mso.addTextArrow(y++, x, "priority", "Priority:");
  priority->addOption("Very low", SITE_PRIORITY_VERY_LOW);
  priority->addOption("Low", SITE_PRIORITY_LOW);
  priority->addOption("Normal", SITE_PRIORITY_NORMAL);
  priority->addOption("High", SITE_PRIORITY_HIGH);
  priority->addOption("Very high", SITE_PRIORITY_VERY_HIGH);
  priority->setOption(this->site->getPriority());
  mso.addCheckBox(y++, x, "aggressivemkdir", "Aggressive mkdir:", this->site->getAggressiveMkdir());
  Pointer<MenuSelectOptionTextArrow> sourcepolicy = mso.addTextArrow(y++, x, "sourcepolicy", "Transfer source policy:");
  sourcepolicy->addOption("Allow", SITE_TRANSFER_POLICY_ALLOW);
  sourcepolicy->addOption("Block", SITE_TRANSFER_POLICY_BLOCK);
  sourcepolicy->setOption(this->site->getTransferSourcePolicy());
  mso.addStringField(y++, x, "exceptsrc", "", exceptsrc, false, 60, 512);
  Pointer<MenuSelectOptionTextArrow> targetpolicy = mso.addTextArrow(y++, x, "targetpolicy", "Transfer target policy:");
  targetpolicy->addOption("Allow", SITE_TRANSFER_POLICY_ALLOW);
  targetpolicy->addOption("Block", SITE_TRANSFER_POLICY_BLOCK);
  targetpolicy->setOption(this->site->getTransferTargetPolicy());
  mso.addStringField(y++, x, "exceptdst", "", exceptdst, false, 60, 512);
  mso.addStringField(y++, x, "affils", "Affils:", affilstr, false, 60, 1024);
  y++;
  ms.initialize(y++, x, this->site->sectionsBegin(), this->site->sectionsEnd());
  focusedarea = &mso;
  mso.makeLeavableDown();
  ms.makeLeavableUp();
  mso.enterFocusFrom(0);
  init(row, col);
}

void EditSiteScreen::redraw() {
  ui->erase();
  if (mso.getElement("sourcepolicy").get<MenuSelectOptionTextArrow>()->getData() == SITE_TRANSFER_POLICY_ALLOW) {
    mso.getElement("exceptsrc")->setLabel("Block transfers from:");
  }
  else {
    mso.getElement("exceptsrc")->setLabel("Allow transfers from:");
  }
  if (mso.getElement("targetpolicy").get<MenuSelectOptionTextArrow>()->getData() == SITE_TRANSFER_POLICY_ALLOW) {
    mso.getElement("exceptdst")->setLabel("Block transfers to:");
  }
  else {
    mso.getElement("exceptdst")->setLabel("Allow transfers to:");
  }
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  int headrow = ms.getHeaderRow();
  int headcol = ms.getHeaderCol();
  ui->printStr(headrow, headcol, "Sections");
  unsigned int selected = ms.getSelectionPointer();
  highlight = false;
  if (ms.isFocused() && selected == 0) {
    highlight = true;
  }
  ui->printStr(headrow, headcol + 9, ms.getElement(0)->getContentText(), highlight);
  for (unsigned int i = 0; i < ms.size(); i++) {
    const MenuSelectOptionContainer * msoc = ms.getSectionContainer(i);
    for (unsigned int j = 0; j < 3; j++) {
      highlight = ((i * 3) + 1 + j) == selected;
      int indentation = 0;
      if (j == 1) {
        indentation = 12;
      }
      else if (j == 2) {
        indentation = 43;
      }
      ui->printStr(headrow + 1 + i, headcol + indentation, msoc->getOption(j)->getContentText(), highlight);
    }
  }
}

void EditSiteScreen::update() {
  if (defocusedarea != NULL) {
    if (defocusedarea == &mso) {
      Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
      ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
    }
    else if (defocusedarea == &ms) {
      int headrow = ms.getHeaderRow();
      int headcol = ms.getHeaderCol();
      int lastsel = ms.getLastSelectionPointer();
      if (lastsel == 0) {
        ui->printStr(headrow, headcol + 9, ms.getElement(0)->getContentText());
      }
      else {
        const MenuSelectOptionContainer * msoc = ms.getSectionContainer((lastsel - 1) / 3);
        int internalid = (lastsel - 1) % 3;
        int add = 0;
        if (internalid == 1) add = 12;
        else if (internalid == 2) add = 43;
        ui->printStr(headrow + 1 + ((lastsel - 1) / 3), headcol + add, msoc->getOption(internalid)->getContentText());
      }
    }
    defocusedarea = NULL;
  }
  if (focusedarea == &mso) {
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
  else if (focusedarea == &ms) {
    if (ms.needsRedraw()) {
      redraw();
      return;
    }
    int headrow = ms.getHeaderRow();
    int headcol = ms.getHeaderCol();
    int lastsel = ms.getLastSelectionPointer();
    int sel = ms.getSelectionPointer();
    if (lastsel == 0) {
      ui->printStr(headrow, headcol + 9, ms.getElement(0)->getContentText());
    }
    else {
      const MenuSelectOptionContainer * msoc = ms.getSectionContainer((lastsel - 1) / 3);
      int internalid = (lastsel - 1) % 3;
      int add = 0;
      if (internalid == 1) add = 12;
      else if (internalid == 2) add = 43;
      ui->printStr(headrow + 1 + ((lastsel - 1) / 3), headcol + add, msoc->getOption(internalid)->getContentText());
    }
    if (sel == 0) {
      ui->printStr(headrow, headcol + 9, ms.getElement(0)->getContentText(), true);
    }
    else {
      const MenuSelectOptionContainer * msoc = ms.getSectionContainer((sel - 1) / 3);
      int internalid = (sel - 1) % 3;
      int add = 0;
      if (internalid == 1) add = 12;
      else if (internalid == 2) add = 43;
      ui->printStr(headrow + 1 + ((sel - 1) / 3), headcol + add, msoc->getOption(internalid)->getContentText(), true);
      int cursorpos = msoc->getOption(internalid)->cursorPosition();
      if (active && cursorpos >= 0) {
        ui->showCursor();
        ui->moveCursor(headrow + 1 + ((sel - 1) / 3), headcol + add + cursorpos);
      }
      else {
        ui->hideCursor();
      }
    }
  }
}

void EditSiteScreen::command(const std::string & command, const std::string & arg) {
  if (command == "returnselectsites") {
    activeelement.get<MenuSelectOptionTextField>()->setText(arg);
    redraw();
  }
}

bool EditSiteScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      std::string identifier = activeelement->getIdentifier();
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      if (identifier == "sourcepolicy" || identifier == "targetpolicy") {
        ui->redraw();
      }
      else {
        ui->update();
      }
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  bool changedname = false;
  std::list<std::string> exceptsrclist;
  std::list<std::string> exceptdstlist;
  std::string sitename;
  switch(ch) {
    case KEY_UP:
      if (focusedarea->goUp()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
          focusedarea->enterFocusFrom(2);
        }
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &ms;
          focusedarea->enterFocusFrom(0);
        }
        ui->update();
      }
      return true;
    case KEY_LEFT:
      if (focusedarea->goLeft()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
      }
      return true;
    case KEY_RIGHT:
      if (focusedarea->goRight()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
      }
      return true;
    case 10: {
      Pointer<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (msoe->getIdentifier() == "skiplist") {
        ui->goSkiplist((SkipList *)&site->getSkipList());
        return true;
      }
      activation = focusedarea->activateSelected();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (activeelement->getIdentifier() == "exceptsrc") {
        activeelement->deactivate();
        active = false;
        std::string preselectstr = activeelement.get<MenuSelectOptionTextField>()->getData();
        std::list<Pointer<Site> > preselected;
        fillPreselectionList(preselectstr, &preselected);
        std::list<Pointer<Site> > excluded;
        excluded.push_back(site);
        std::string action = "Block";
        if (mso.getElement("sourcepolicy").get<MenuSelectOptionTextArrow>()->getData() == SITE_TRANSFER_POLICY_BLOCK) {
          action = "Allow";
        }
        ui->goSelectSites(action + " race transfers from these sites", preselected, excluded);
        return true;
      }
      if (activeelement->getIdentifier() == "exceptdst") {
        activeelement->deactivate();
        active = false;
        std::string preselectstr = activeelement.get<MenuSelectOptionTextField>()->getData();
        std::list<Pointer<Site> > preselected;
        fillPreselectionList(preselectstr, &preselected);
        std::list<Pointer<Site> > excluded;
        excluded.push_back(site);
        std::string action = "Block";
        if (mso.getElement("targetpolicy").get<MenuSelectOptionTextArrow>()->getData() == SITE_TRANSFER_POLICY_BLOCK) {
          action = "Allow";
        }
        ui->goSelectSites(action + " race transfers to these sites", preselected, excluded);
        return true;
      }
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      return true;
    }
    case 'd':
      if (operation == "add") {
        site = makePointer<Site>();
      }
      for(unsigned int i = 0; i < mso.size(); i++) {
        Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "name") {
          std::string newname = msoe.get<MenuSelectOptionTextField>()->getData();
          if (newname != site->getName()) {
            changedname = true;
          }
          site->setName(newname);
        }
        else if (identifier == "addr") {
          std::string addrports = msoe.get<MenuSelectOptionTextField>()->getData();
          site->setAddresses(addrports);
        }
        else if (identifier == "user") {
          site->setUser(msoe.get<MenuSelectOptionTextField>()->getData());
        }
        else if (identifier == "pass") {
          site->setPass(msoe.get<MenuSelectOptionTextField>()->getData());
        }
        else if (identifier == "basepath") {
          site->setBasePath(msoe.get<MenuSelectOptionTextField>()->getData());
        }
        else if (identifier == "logins") {
          site->setMaxLogins(msoe.get<MenuSelectOptionNumArrow>()->getData());
        }
        else if (identifier == "maxup") {
          site->setMaxUp(msoe.get<MenuSelectOptionNumArrow>()->getData());
        }
        else if (identifier == "maxdn") {
          site->setMaxDn(msoe.get<MenuSelectOptionNumArrow>()->getData());
        }
        else if (identifier == "pret") {
          site->setPRET(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "binary") {
          site->setForceBinaryMode(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "ssl") {
          site->setSSL(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "ssltransfer") {
          site->setSSLTransferPolicy(msoe.get<MenuSelectOptionTextArrow>()->getData());
        }
        else if (identifier == "cpsv") {
          site->setSupportsCPSV(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "listcommand") {
          site->setListCommand(msoe.get<MenuSelectOptionTextArrow>()->getData());
        }
        else if (identifier == "disabled") {
          site->setDisabled(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "allowupload") {
          site->setAllowUpload(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "allowdownload") {
          site->setAllowDownload(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "priority") {
          site->setPriority(msoe.get<MenuSelectOptionTextArrow>()->getData());
        }
        else if (identifier == "brokenpasv") {
          site->setBrokenPASV(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "idletime") {
          site->setMaxIdleTime(util::str2Int(msoe.get<MenuSelectOptionTextField>()->getData()));
        }
        else if (identifier == "useproxy") {
          int proxytype = msoe.get<MenuSelectOptionTextArrow>()->getData();
          site->setProxyType(proxytype);
          if (proxytype == SITE_PROXY_USE) {
            site->setProxy(msoe.get<MenuSelectOptionTextArrow>()->getDataText());
          }
        }
        else if (identifier == "affils") {
          std::string affils = msoe.get<MenuSelectOptionTextField>()->getData();
          site->clearAffils();
          size_t pos;
          while ((pos = affils.find(",")) != std::string::npos) {
            affils[pos] = ' ';
          }
          while ((pos = affils.find(";")) != std::string::npos) {
            affils[pos] = ' ';
          }
          std::list<std::string> affilslist = util::split(affils);
          for (std::list<std::string>::const_iterator it = affilslist.begin(); it != affilslist.end(); it++) {
            site->addAffil(*it);
          }
        }
        else if (identifier == "sourcepolicy") {
          site->setTransferSourcePolicy(msoe.get<MenuSelectOptionTextArrow>()->getData());
        }
        else if (identifier == "targetpolicy") {
          site->setTransferTargetPolicy(msoe.get<MenuSelectOptionTextArrow>()->getData());
        }
        else if (identifier == "exceptsrc") {
          std::string sitestr = msoe.get<MenuSelectOptionTextField>()->getData();
          exceptsrclist = util::split(sitestr, ",");
        }
        else if (identifier == "exceptdst") {
          std::string sitestr = msoe.get<MenuSelectOptionTextField>()->getData();
          exceptdstlist = util::split(sitestr, ",");
        }
        else if (identifier == "aggressivemkdir") {
          site->setAggressiveMkdir(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
      }
      site->clearSections();
      for (unsigned int i = 0; i < ms.size(); i++) {
        const MenuSelectOptionContainer * msoc = ms.getSectionContainer(i);
        std::string name = msoc->getOption(0).get<MenuSelectOptionTextField>()->getData();
        std::string path = msoc->getOption(1).get<MenuSelectOptionTextField>()->getData();
        if (name.length() > 0 && path.length() > 0) {
          site->addSection(name, path);
        }
      }

      if (operation == "add") {
        global->getSiteManager()->addSite(site);
      }
      else {
        global->getSiteManager()->sortSites();
      }

      sitename = site->getName();
      global->getSiteManager()->resetSitePairsForSite(sitename);
      for (std::list<std::string>::const_iterator it = exceptsrclist.begin(); it != exceptsrclist.end(); it++) {
        global->getSiteManager()->addExceptSourceForSite(sitename, *it);
      }
      for (std::list<std::string>::const_iterator it = exceptdstlist.begin(); it != exceptdstlist.end(); it++) {
        global->getSiteManager()->addExceptTargetForSite(sitename, *it);
      }

      global->getSiteLogicManager()->getSiteLogic(site->getName())->setNumConnections(site->getMaxLogins());
      if (changedname) {
        global->getSiteLogicManager()->getSiteLogic(site->getName())->updateName();
      }

      global->getSettingsLoaderSaver()->saveSettings();
      ui->returnToLast();
      return true;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string EditSiteScreen::getLegendText() const {
  return currentlegendtext;
}

std::string EditSiteScreen::getInfoLabel() const {
  return "SITE OPTIONS";
}

void EditSiteScreen::fillPreselectionList(const std::string & preselectstr, std::list<Pointer<Site> > * list) const {
  std::list<std::string> preselectlist = util::split(preselectstr, ",");
  for (std::list<std::string>::const_iterator it = preselectlist.begin(); it != preselectlist.end(); it++) {
    Pointer<Site> site = global->getSiteManager()->getSite(*it);
    list->push_back(site);
  }
}
