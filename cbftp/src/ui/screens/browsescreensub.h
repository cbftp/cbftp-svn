#pragma once

#include "../../core/eventreceiver.h"

class BrowseScreenAction;
class UIFileList;

enum BrowseScreenType {
  BROWSESCREEN_SITE,
  BROWSESCREEN_LOCAL,
  BROWSESCREEN_SELECTOR
};

class BrowseScreenSub : public EventReceiver {
public:
  virtual void redraw(unsigned int, unsigned int, unsigned int) = 0;
  virtual void update() = 0;
  virtual BrowseScreenType type() const = 0;
  virtual std::string getLegendText() const = 0;
  virtual std::string getInfoLabel() const = 0;
  virtual std::string getInfoText() const = 0;
  virtual void command(const std::string &, const std::string &) { }
  virtual BrowseScreenAction keyPressed(unsigned int) = 0;
  virtual void setFocus(bool) = 0;
  virtual const UIFileList * getUIFileList() const { return NULL; }
};
