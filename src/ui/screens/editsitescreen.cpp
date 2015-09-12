#include "editsitescreen.h"

#include "../../globalcontext.h"
#include "../../sitemanager.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../proxymanager.h"
#include "../../proxy.h"
#include "../../util.h"

#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncontainer.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextarrow.h"

extern GlobalContext * global;

EditSiteScreen::EditSiteScreen(Ui * ui) {
  this->ui = ui;
}

EditSiteScreen::~EditSiteScreen() {

}

void EditSiteScreen::initialize(unsigned int row, unsigned int col, std::string operation, std::string site) {
  active = false;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one, save changes - [c]ancel, undo changes";
  currentlegendtext = defaultlegendtext;
  this->operation = operation;
  SiteManager * sm = global->getSiteManager();
  std::list<Site *> blockedsrclist;
  std::list<Site *> blockeddstlist;
  if (operation == "add") {
    modsite = Site("SUNET");
    modsite.setUser(sm->getDefaultUserName());
    modsite.setPass(sm->getDefaultPassword());
    modsite.setMaxLogins(sm->getDefaultMaxLogins());
    modsite.setMaxUp(sm->getDefaultMaxUp());
    modsite.setMaxDn(sm->getDefaultMaxDown());
    modsite.setSSL(sm->getDefaultSSL());
    modsite.setSSLTransferPolicy(sm->getDefaultSSLTransferPolicy());
    modsite.setMaxIdleTime(sm->getDefaultMaxIdleTime());
  }
  else if (operation == "edit") {
    this->site = global->getSiteManager()->getSite(site);
    modsite = Site(*this->site);
    blockedsrclist = sm->getBlocksToSite(this->site);
    blockeddstlist = sm->getBlocksFromSite(this->site);
  }
  std::string affilstr = "";
  std::map<std::string, bool>::const_iterator it;
  for (it = modsite.affilsBegin(); it != modsite.affilsEnd(); it++) {
    affilstr += it->first + " ";
  }
  if (affilstr.length() > 0) {
    affilstr = affilstr.substr(0, affilstr.length() - 1);
  }
  std::string bannedgroupsstr = "";
  for (it = modsite.bannedGroupsBegin(); it != modsite.bannedGroupsEnd(); it++) {
    bannedgroupsstr += it->first + " ";
  }
  if (bannedgroupsstr.length() > 0) {
    bannedgroupsstr = bannedgroupsstr.substr(0, bannedgroupsstr.length() - 1);
  }
  std::string blockedsrc = "";
  for (std::list<Site *>::iterator it = blockedsrclist.begin(); it != blockedsrclist.end(); it++) {
    blockedsrc += (*it)->getName() + ",";
  }
  blockedsrc = blockedsrc.substr(0, blockedsrc.length() - 1);
  std::string blockeddst = "";
  for (std::list<Site *>::iterator it = blockeddstlist.begin(); it != blockeddstlist.end(); it++) {
    blockeddst += (*it)->getName() + ",";
  }
  blockeddst = blockeddst.substr(0, blockeddst.length() - 1);
  unsigned int y = 1;
  unsigned int x = 1;
  std::string addrport = modsite.getAddress();
  std::string port = modsite.getPort();
  if (port != "21") {
    addrport += ":" + port;
  }
  mso.clear();
  mso.addStringField(y++, x, "name", "Name:", modsite.getName(), false);
  mso.addStringField(y++, x, "addr", "Address:", addrport, false, 32, 256);
  mso.addStringField(y++, x, "user", "Username:", modsite.getUser(), false);
  mso.addStringField(y++, x, "pass", "Password:", modsite.getPass(), true);
  mso.addIntArrow(y++, x, "logins", "Login slots:", modsite.getInternMaxLogins(), 0, 99);
  mso.addIntArrow(y++, x, "maxup", "Upload slots:", modsite.getInternMaxUp(), 0, 99);
  mso.addIntArrow(y++, x, "maxdn", "Download slots:", modsite.getInternMaxDown(), 0, 99);
  mso.addStringField(y++, x, "basepath", "Base path:", modsite.getBasePath(), false, 32, 512);
  Pointer<MenuSelectOptionTextArrow> listcommand = mso.addTextArrow(y++, x, "listcommand", "List command:");
  listcommand->addOption("STAT -l", SITE_LIST_STAT);
  listcommand->addOption("LIST", SITE_LIST_LIST);
  listcommand->setOption(modsite.getListCommand());
  mso.addStringField(y++, x, "idletime", "Max idle time (s):", util::int2Str(modsite.getMaxIdleTime()), false);
  mso.addCheckBox(y++, x, "ssl", "AUTH SSL:", modsite.SSL());
  Pointer<MenuSelectOptionTextArrow> sslfxp = mso.addTextArrow(y++, x, "ssltransfer", "SSL transfers:");
  sslfxp->addOption("Always off", SITE_SSL_ALWAYS_OFF);
  sslfxp->addOption("Prefer off", SITE_SSL_PREFER_OFF);
  sslfxp->addOption("Prefer on", SITE_SSL_PREFER_ON);
  sslfxp->addOption("Always on", SITE_SSL_ALWAYS_ON);
  sslfxp->setOption(modsite.getSSLTransferPolicy());
  mso.addCheckBox(y++, x, "cpsv", "CPSV supported:", modsite.supportsCPSV());
  mso.addCheckBox(y++, x, "pret", "Needs PRET:", modsite.needsPRET());
  mso.addCheckBox(y++, x, "binary", "Force binary mode:", modsite.forceBinaryMode());
  mso.addCheckBox(y++, x, "brokenpasv", "Broken PASV:", modsite.hasBrokenPASV());
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
  int proxytype = modsite.getProxyType();
  useproxy->setOption(proxytype);
  if (proxytype == SITE_PROXY_USE) {
    useproxy->setOptionText(modsite.getProxy());
  }
  y++;
  mso.addCheckBox(y++, x, "allowupload", "Allow upload:", modsite.getAllowUpload());
  mso.addCheckBox(y++, x, "allowdownload", "Allow download:", modsite.getAllowDownload());
  mso.addIntArrow(y++, x, "rank", "Rank (" + util::int2Str(SITE_RANK_USE_GLOBAL) + " for global):", modsite.getRank(), 0, SITE_RANK_MAX);
  mso.addIntArrow(y++, x, "ranktolerance", "Rank tolerance (" + util::int2Str(SITE_RANK_USE_GLOBAL) + " for global):", modsite.getRankTolerance(), 0, SITE_RANK_MAX);
  mso.addStringField(y++, x, "blockedsrc", "Block transfers from:", blockedsrc, false, 60, 512);
  mso.addStringField(y++, x, "blockeddst", "Block transfers to:", blockeddst, false, 60, 512);
  mso.addStringField(y++, x, "affils", "Affils:", affilstr, false, 60, 1024);
  mso.addStringField(y++, x, "bannedgroups", "Banned groups:", bannedgroupsstr, false, 60, 1024);
  y++;
  ms.initialize(y++, x, modsite.sectionsBegin(), modsite.sectionsEnd());
  focusedarea = &mso;
  mso.makeLeavableDown();
  ms.makeLeavableUp();
  mso.enterFocusFrom(0);
  init(row, col);
}

void EditSiteScreen::redraw() {
  ui->erase();
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

void EditSiteScreen::command(std::string command, std::string arg) {
  if (command == "returnselectsites") {
    activeelement.get<MenuSelectOptionTextField>()->setText(arg);
    redraw();
  }
}

void EditSiteScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      return;
    }
    activeelement->inputChar(ch);
    ui->update();
    return;
  }
  bool activation;
  bool changedname = false;
  std::list<std::string> blocksrclist;
  std::list<std::string> blockdstlist;
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
      break;
    case KEY_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &ms;
          focusedarea->enterFocusFrom(0);
        }
        ui->update();
      }
      break;
    case KEY_LEFT:
      if (focusedarea->goLeft()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
      }
      break;
    case KEY_RIGHT:
      if (focusedarea->goRight()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
      }
      break;
    case 10:
      activation = focusedarea->activateSelected();
      if (!activation) {
        ui->update();
        break;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (activeelement->getIdentifier() == "blockedsrc") {
        activeelement->deactivate();
        active = false;
        std::string preselectstr = activeelement.get<MenuSelectOptionTextField>()->getData();
        std::list<Site *> preselected;
        fillPreselectionList(preselectstr, &preselected);
        std::list<Site *> excluded;
        excluded.push_back(site);
        ui->goSelectSites("Block race transfers from these sites", preselected, excluded);
        return;
      }
      if (activeelement->getIdentifier() == "blockeddst") {
        activeelement->deactivate();
        active = false;
        std::string preselectstr = activeelement.get<MenuSelectOptionTextField>()->getData();
        std::list<Site *> preselected;
        fillPreselectionList(preselectstr, &preselected);
        std::list<Site *> excluded;
        excluded.push_back(site);
        ui->goSelectSites("Block race transfers to these sites", preselected, excluded);
        return;
      }
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      break;
    case 'd':
      if (operation == "add") {
        site = new Site();
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
          std::string addrport = msoe.get<MenuSelectOptionTextField>()->getData();
          size_t splitpos = addrport.find(":");
          if (splitpos != std::string::npos) {
            site->setAddress(addrport.substr(0, splitpos));
            std::string port = addrport.substr(splitpos + 1);
            if (port.length() == 0) {
              port = "21";
            }
            site->setPort(port);
          }
          else {
            site->setAddress(addrport);
            site->setPort("21");
          }
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
        else if (identifier == "allowupload") {
          site->setAllowUpload(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "allowdownload") {
          site->setAllowDownload(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "brokenpasv") {
          site->setBrokenPASV(msoe.get<MenuSelectOptionCheckBox>()->getData());
        }
        else if (identifier == "idletime") {
          site->setMaxIdleTime(util::str2Int(msoe.get<MenuSelectOptionTextField>()->getData()));
        }
        else if (identifier == "rank") {
          site->setRank(msoe.get<MenuSelectOptionNumArrow>()->getData());
        }
        else if (identifier == "ranktolerance") {
          site->setRankTolerance(msoe.get<MenuSelectOptionNumArrow>()->getData());
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
          size_t start = 0;
          pos = 0;
          bool started = false;
          while(pos < affils.length()) {
            if (!started) {
              if (affils[pos] != ' ') {
                start = pos;
                started = true;
              }
            }
            else {
              if (affils[pos] == ' ') {
                site->addAffil(affils.substr(start, pos - start));
                started = false;
              }
              else if (pos == affils.length() - 1) {
                site->addAffil(affils.substr(start, pos + 1 - start));
              }
            }
            pos++;
          }
        }
        else if (identifier == "bannedgroups") {
          std::string bannedgroups = msoe.get<MenuSelectOptionTextField>()->getData();
          site->clearBannedGroups();
          size_t pos;
          while ((pos = bannedgroups.find(",")) != std::string::npos) {
            bannedgroups[pos] = ' ';
          }
          while ((pos = bannedgroups.find(";")) != std::string::npos) {
            bannedgroups[pos] = ' ';
          }
          size_t start = 0;
          pos = 0;
          bool started = false;
          while(pos < bannedgroups.length()) {
            if (!started) {
              if (bannedgroups[pos] != ' ') {
                start = pos;
                started = true;
              }
            }
            else {
              if (bannedgroups[pos] == ' ') {
                site->addBannedGroup(bannedgroups.substr(start, pos - start));
                started = false;
              }
              else if (pos == bannedgroups.length() - 1) {
                site->addBannedGroup(bannedgroups.substr(start, pos + 1 - start));
              }
            }
            pos++;
          }
        }
        else if (identifier == "blockedsrc") {
          std::string blockedstr = msoe.get<MenuSelectOptionTextField>()->getData();
          while (true) {
            size_t commapos = blockedstr.find(",");
            if (commapos != std::string::npos) {
              blocksrclist.push_back(blockedstr.substr(0, commapos));
              blockedstr = blockedstr.substr(commapos + 1);
            }
            else {
              blocksrclist.push_back(blockedstr);
              break;
            }
          }
        }
        else if (identifier == "blockeddst") {
          std::string blockedstr = msoe.get<MenuSelectOptionTextField>()->getData();
          while (true) {
            size_t commapos = blockedstr.find(",");
            if (commapos != std::string::npos) {
              blockdstlist.push_back(blockedstr.substr(0, commapos));
              blockedstr = blockedstr.substr(commapos + 1);
            }
            else {
              blockdstlist.push_back(blockedstr);
              break;
            }
          }
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
      global->getSiteManager()->clearBlocksForSite(site);
      for (std::list<std::string>::iterator it = blocksrclist.begin(); it != blocksrclist.end(); it++) {
        global->getSiteManager()->addBlockedPair(*it, sitename);
      }
      for (std::list<std::string>::iterator it = blockdstlist.begin(); it != blockdstlist.end(); it++) {
        global->getSiteManager()->addBlockedPair(sitename, *it);
      }
      global->getSiteLogicManager()->getSiteLogic(site->getName())->setNumConnections(site->getMaxLogins());
      if (changedname) {
        global->getSiteLogicManager()->getSiteLogic(site->getName())->updateName();
      }
      ui->returnToLast();
      return;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      break;
  }
}

std::string EditSiteScreen::getLegendText() const {
  return currentlegendtext;
}

std::string EditSiteScreen::getInfoLabel() const {
  return "SITE OPTIONS";
}

void EditSiteScreen::fillPreselectionList(std::string preselectstr, std::list<Site *> * list) const {
  while (true) {
    size_t commapos = preselectstr.find(",");
    if (commapos != std::string::npos) {
      std::string sitename = preselectstr.substr(0, commapos);
      Site * site = global->getSiteManager()->getSite(sitename);
      list->push_back(site);
      preselectstr = preselectstr.substr(commapos + 1);
    }
    else {
      if (preselectstr.length() > 0) {
        Site * site = global->getSiteManager()->getSite(preselectstr);
        list->push_back(site);
      }
      break;
    }
  }
}
