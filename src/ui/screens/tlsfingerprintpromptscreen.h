#pragma once

#include <memory>
#include <string>

#include "../uiwindow.h"

class Site;

class TLSFingerprintPromptScreen : public UIWindow {
public:
  TLSFingerprintPromptScreen(Ui *);
  void initialize(unsigned int row, unsigned int col, int connid, const std::string& sitename,
                  const std::string& oldfp, const std::string& newfp);
  void redraw() override;
  bool keyPressed(unsigned int ch) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  int connid;
  std::string sitename;
  std::string oldfingerprint;
  std::string newfingerprint;
};