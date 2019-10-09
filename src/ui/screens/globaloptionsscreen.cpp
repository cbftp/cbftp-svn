#include "globaloptionsscreen.h"

#include "../../core/iomanager.h"
#include "../../globalcontext.h"
#include "../../remotecommandhandler.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../localstorage.h"
#include "../../settingsloadersaver.h"
#include "../../engine.h"
#include "../../transferprotocol.h"
#include "../../httpserver.h"

#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextarrow.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"

GlobalOptionsScreen::GlobalOptionsScreen(Ui * ui) {
  this->ui = ui;
}

GlobalOptionsScreen::~GlobalOptionsScreen() {

}

void GlobalOptionsScreen::initialize(unsigned int row, unsigned int col) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel - [S]kiplist";
  currentlegendtext = defaultlegendtext;
  active = false;
  unsigned int y = 1;
  unsigned int x = 1;
  rch = global->getRemoteCommandHandler();
  sm = global->getSiteManager();
  ls = global->getLocalStorage();
  mso.reset();
  defaultinterface = mso.addTextArrow(y++, x, "defaultinterface", "Default network interface:");
  defaultinterface->addOption("Unspecified", 0);
  interfacemap[0] = "";
  std::list<std::pair<std::string, std::string> > interfaces = global->getIOManager()->listInterfaces();
  int interfaceid = 1;
  bool hasdefault = global->getIOManager()->hasDefaultInterface();
  std::string defaultinterfacename;
  if (hasdefault) {
    defaultinterfacename = global->getIOManager()->getDefaultInterface();
  }
  for (std::list<std::pair<std::string, std::string> >::iterator it = interfaces.begin(); it != interfaces.end(); it++) {
    if (it->first == "lo") {
      continue;
    }
    interfacemap[interfaceid] = it->first;
    defaultinterface->addOption(it->first + ", " + it->second, interfaceid++);
    if (hasdefault && it->first == defaultinterfacename) {
      defaultinterface->setOption(interfaceid - 1);
    }
  }
  std::shared_ptr<MenuSelectOptionTextArrow> transferproto = mso.addTextArrow(y++, x, "transferprotocol", "Local transfer protocol:");
  transferproto->addOption("IPv4 only", static_cast<int>(TransferProtocol::IPV4_ONLY));
  transferproto->addOption("Prefer IPv4", static_cast<int>(TransferProtocol::PREFER_IPV4));
  transferproto->addOption("Prefer IPv6", static_cast<int>(TransferProtocol::PREFER_IPV6));
  transferproto->addOption("IPv6 only", static_cast<int>(TransferProtocol::IPV6_ONLY));
  transferproto->setOption(static_cast<int>(ls->getTransferProtocol()));
  int firstport = ls->getActivePortFirst();
  int lastport = ls->getActivePortLast();
  std::string portrange = std::to_string(firstport) + ":" + std::to_string(lastport);
  mso.addStringField(y++, x, "activeportrange", "Active mode port range:", portrange, false, 11);
  mso.addCheckBox(y++, x, "useactiveaddress", "Use active mode address:", ls->getUseActiveModeAddress());
  mso.addStringField(y++, x, "activeaddress4", "Active mode address IPv4:", ls->getActiveModeAddress4(), false, 64);
  mso.addStringField(y++, x, "activeaddress6", "Active mode address IPv6:", ls->getActiveModeAddress6(), false, 64);
  y++;
  mso.addCheckBox(y++, x, "tcpenable", "Enable HTTPS/JSON API:", global->getHTTPServer()->getEnabled());
  mso.addStringField(y++, x, "tcpport", "HTTPS/JSON API Port:", std::to_string(global->getHTTPServer()->getPort()), false, 5);
  mso.addCheckBox(y++, x, "udpenable", "Enable UDP API:", rch->isEnabled());
  mso.addStringField(y++, x, "udpport", "UDP API Port:", std::to_string(rch->getUDPPort()), false, 5);
  mso.addStringField(y++, x, "apipass", "API password:", rch->getPassword(), true);
  std::shared_ptr<MenuSelectOptionTextArrow> bell = mso.addTextArrow(y++, x, "udpbell", "Remote command bell:");
  bell->addOption("Disabled", static_cast<int>(RemoteCommandNotify::DISABLED));
  bell->addOption("Action requested", static_cast<int>(RemoteCommandNotify::ACTION_REQUESTED));
  bell->addOption("Jobs added", static_cast<int>(RemoteCommandNotify::JOBS_ADDED));
  bell->addOption("All commands", static_cast<int>(RemoteCommandNotify::ALL_COMMANDS));
  bell->setOption(static_cast<int>(rch->getNotify()));
  mso.addStringField(y++, x, "preparedraceexpirytime", "Prepared spread job expiration time:", std::to_string(global->getEngine()->getPreparedRaceExpiryTime()), false, 5);
  std::shared_ptr<MenuSelectOptionTextArrow> racestarterexpiry = mso.addTextArrow(y++, x, "racestarterexpiry", "Next prepared spread job starter timeout:");
  racestarterexpiry->addOption("30s", 30);
  racestarterexpiry->addOption("1m", 60);
  racestarterexpiry->addOption("3m", 180);
  racestarterexpiry->addOption("5m", 300);
  racestarterexpiry->addOption("10m", 600);
  racestarterexpiry->addOption("30m", 1800);
  racestarterexpiry->addOption("1h", 3600);
  racestarterexpiry->addOption("Unlimited", 0);
  racestarterexpiry->setOption(global->getEngine()->getNextPreparedRaceStarterTimeout());
  y++;
  std::shared_ptr<MenuSelectOptionTextArrow> legendmode = mso.addTextArrow(y++, x, "legendmode", "Legend bar:");
  legendmode->addOption("Disabled", LEGEND_DISABLED);
  legendmode->addOption("Scrolling", LEGEND_SCROLLING);
  legendmode->addOption("Static", LEGEND_STATIC);
  legendmode->setOption(ui->legendMode());
  y++;
  mso.addStringField(y++, x, "defuser", "Default site username:", sm->getDefaultUserName(), false);
  mso.addStringField(y++, x, "defpass", "Default site password:", sm->getDefaultPassword(), true);
  mso.addIntArrow(y++, x, "deflogins", "Default site login slots:", sm->getDefaultMaxLogins(), 0, 99);
  mso.addIntArrow(y++, x, "defmaxup", "Default site upload slots:", sm->getDefaultMaxUp(), 0, 99);
  mso.addIntArrow(y++, x, "defmaxdn", "Default site download slots:", sm->getDefaultMaxDown(), 0, 99);
  std::shared_ptr<MenuSelectOptionTextArrow> tlsmode = mso.addTextArrow(y++, x, "tlsmode", "Default TLS mode:");
  tlsmode->addOption("None", static_cast<int>(TLSMode::NONE));
  tlsmode->addOption("AUTH TLS", static_cast<int>(TLSMode::AUTH_TLS));
  tlsmode->addOption("Implicit", static_cast<int>(TLSMode::IMPLICIT));
  tlsmode->setOption(static_cast<int>(sm->getDefaultTLSMode()));
  std::shared_ptr<MenuSelectOptionTextArrow> sslfxp = mso.addTextArrow(y++, x, "tlsfxp", "Default TLS transfers:");
  sslfxp->addOption("Always off", SITE_SSL_ALWAYS_OFF);
  sslfxp->addOption("Prefer off", SITE_SSL_PREFER_OFF);
  sslfxp->addOption("Prefer on", SITE_SSL_PREFER_ON);
  sslfxp->addOption("Always on", SITE_SSL_ALWAYS_ON);
  sslfxp->setOption(sm->getDefaultSSLTransferPolicy());
  mso.addStringField(y++, x, "defidletime", "Default site max idle time (s):", std::to_string(sm->getDefaultMaxIdleTime()), false);
  y++;
  mso.addStringField(y++, x, "dlpath", "Download path:", ls->getDownloadPath().toString(), false, 128, 128);
  y++;
  mso.addTextButtonNoContent(y++, x, "skiplist", "Configure skiplist...");
  mso.addTextButtonNoContent(y++, x, "proxy", "Configure proxy settings...");
  mso.addTextButtonNoContent(y++, x, "fileviewer", "Configure file viewing...");
  mso.addTextButtonNoContent(y++, x, "changekey", "Change encryption key...");
  init(row, col);
}

void GlobalOptionsScreen::redraw() {
  ui->erase();
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

void GlobalOptionsScreen::update() {
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

bool GlobalOptionsScreen::keyPressed(unsigned int ch) {
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
  std::shared_ptr<MenuSelectOptionElement> msoe;
  switch(ch) {
    case KEY_UP:
      mso.goUp();
      ui->update();
      return true;
    case KEY_DOWN:
      mso.goDown();
      ui->update();
      return true;
    case 10:
      msoe = mso.getElement(mso.getSelectionPointer());
      if (msoe->getIdentifier() == "skiplist") {
        ui->goSkiplist();
        return true;
      }
      if (msoe->getIdentifier() == "changekey") {
        ui->goChangeKey();
        return true;
      }
      if (msoe->getIdentifier() == "proxy") {
        ui->goProxy();
        return true;
      }
      if (msoe->getIdentifier() == "fileviewer") {
        ui->goFileViewerSettings();
        return true;
      }
      activation = msoe->activate();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = msoe;
      currentlegendtext = activeelement->getLegendText();
      ui->setLegend();
      ui->update();
      return true;
    case 27: // esc
    case 'c':
      global->getSettingsLoaderSaver()->saveSettings();
      ui->returnToLast();
      return true;
    case 'S':
      ui->goSkiplist();
      return true;
    case 'd':
      bool udpenable = false;
      for(unsigned int i = 0; i < mso.size(); i++) {
        std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "defaultinterface") {
          std::string interface = interfacemap[std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()];
          global->getIOManager()->setDefaultInterface(interface);
        }
        else if (identifier == "activeportrange") {
          std::string portrange = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
          size_t splitpos = portrange.find(":");
          if (splitpos != std::string::npos) {
            int portfirst = std::stoi(portrange.substr(0, splitpos));
            int portlast = std::stoi(portrange.substr(splitpos + 1));
            ls->setActivePortFirst(portfirst);
            ls->setActivePortLast(portlast);
          }
        }
        else if (identifier == "transferprotocol") {
          ls->setTransferProtocol(static_cast<TransferProtocol>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "useactiveaddress") {
          ls->setUseActiveModeAddress(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "activeaddress4") {
          ls->setActiveModeAddress4(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "activeaddress6") {
          ls->setActiveModeAddress6(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "tcpenable") {
          global->getHTTPServer()->setEnabled(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "tcpport") {
          global->getHTTPServer()->setPort(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "udpenable") {
          udpenable = std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData();
          if (rch->isEnabled() && !udpenable) {
            rch->setEnabled(false);
          }
        }
        else if (identifier == "udpport") {
          rch->setPort(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "apipass") {
          rch->setPassword(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "udpbell") {
          rch->setNotify(static_cast<RemoteCommandNotify>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "defuser") {
          sm->setDefaultUserName(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "defpass") {
          sm->setDefaultPassword(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "deflogins") {
          sm->setDefaultMaxLogins(std::static_pointer_cast<MenuSelectOptionNumArrow>(msoe)->getData());
        }
        else if (identifier == "defmaxup") {
          sm->setDefaultMaxUp(std::static_pointer_cast<MenuSelectOptionNumArrow>(msoe)->getData());
        }
        else if (identifier == "defmaxdn") {
          sm->setDefaultMaxDown(std::static_pointer_cast<MenuSelectOptionNumArrow>(msoe)->getData());
        }
        else if (identifier == "tlsmode") {
          sm->setDefaultTLSMode(static_cast<TLSMode>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "tlsfxp") {
          sm->setDefaultSSLTransferPolicy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "defidletime") {
          sm->setDefaultMaxIdleTime(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "legend") { // legacy
          ui->setLegendMode(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData() ? LEGEND_SCROLLING : LEGEND_DISABLED);
        }
        else if (identifier == "legendmode") {
          ui->setLegendMode((LegendMode)std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "dlpath") {
          ls->setDownloadPath(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "preparedraceexpirytime") {
          global->getEngine()->setPreparedRaceExpiryTime(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "racestarterexpiry") {
          global->getEngine()->setNextPreparedRaceStarterTimeout(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
      }
      rch->setEnabled(udpenable);
      global->getSettingsLoaderSaver()->saveSettings();
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string GlobalOptionsScreen::getLegendText() const {
  return currentlegendtext;
}

std::string GlobalOptionsScreen::getInfoLabel() const {
  return "GLOBAL SETTINGS";
}
