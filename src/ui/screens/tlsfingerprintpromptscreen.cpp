#include "tlsfingerprintpromptscreen.h"

#include "../ui.h"

#include "../../globalcontext.h"
#include "../../sitemanager.h"
#include "../../site.h"

TLSFingerprintPromptScreen::TLSFingerprintPromptScreen(Ui* ui) : UIWindow(ui, "TLSFingerprintPromptScreen") {
  allowimplicitgokeybinds = false;
}

void TLSFingerprintPromptScreen::initialize(unsigned int row, unsigned int col,
                                               int connid,
                                               const std::string& sitename,
                                               const std::string& oldfp,
                                               const std::string& newfp) {
  this->connid = connid;
  this->sitename = sitename;
  this->oldfingerprint = oldfp;
  this->newfingerprint = newfp;
  init(row, col);
}

void TLSFingerprintPromptScreen::redraw() {
  vv->clear();
  vv->putStr(1, 1, "WARNING: TLS certificate fingerprint has changed for site: " + sitename);
  vv->putStr(3, 1, "Old fingerprint: " + oldfingerprint);
  vv->putStr(4, 1, "New fingerprint: " + newfingerprint);
  vv->putStr(6, 1, "This could indicate:");
  vv->putStr(7, 1, "  - Certificate renewal (normal)");
  vv->putStr(8, 1, "  - Man-in-the-middle attack (DANGER)");
  vv->putStr(10, 1, "What do you want to do?");
  vv->putStr(12, 1, "[a] Accept new fingerprint and continue");
  vv->putStr(13, 1, "[d] Disable fingerprint verification and continue");
  vv->putStr(14, 1, "[c] Cancel connection");
}

bool TLSFingerprintPromptScreen::keyPressed(unsigned int ch) {
  if (ch == 'a' || ch == 'A') {
    ui->fingerprintPromptAccept(connid);
    return true;
  }
  else if (ch == 'd' || ch == 'D') {
    ui->fingerprintPromptDisable(connid);
    return true;
  }
  else if (ch == 'c' || ch == 'C') {
    ui->fingerprintPromptCancel(connid);
    return true;
  }
  return false;
}

std::string TLSFingerprintPromptScreen::getLegendText() const {
  return "[a]ccept - [d]isable verification - [c]ancel";
}

std::string TLSFingerprintPromptScreen::getInfoLabel() const {
  return "TLS FINGERPRINT CHANGED";
}