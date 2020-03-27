#pragma once

#include <string>
#include <list>

#include "keybinds.h"

class Ui;

class UIWindow {
protected:
  std::string name;
  unsigned int row;
  unsigned int col;
  bool autoupdate;
  bool expectbackendpush;
  Ui * ui;
  KeyBinds keybinds;
  bool allowimplicitgokeybinds;
public:
  void init(unsigned int, unsigned int);
  UIWindow(Ui* ui, const std::string& name);
  virtual ~UIWindow();
  virtual void redraw() = 0;
  void resize(unsigned int, unsigned int);
  virtual void update();
  virtual void command(const std::string &);
  virtual void command(const std::string &, const std::string &);
  virtual std::string getInfoLabel() const;
  virtual std::string getInfoText() const;
  virtual std::string getLegendText() const;
  bool keyPressedBase(unsigned int);
  virtual bool keyPressed(unsigned int);
  bool autoUpdate() const;
  bool expectBackendPush() const;
};
