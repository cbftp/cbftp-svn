#pragma once

#include <string>

class Ui;

class UIWindow {
protected:
  unsigned int row;
  unsigned int col;
  bool autoupdate;
  bool expectbackendpush;
  Ui * ui;
public:
  void init(unsigned int, unsigned int);
  UIWindow();
  virtual ~UIWindow();
  virtual void redraw() = 0;
  void resize(unsigned int, unsigned int);
  virtual void update();
  virtual void command(std::string);
  virtual void command(std::string, std::string);
  virtual std::string getInfoLabel() const;
  virtual std::string getInfoText() const;
  virtual std::string getLegendText() const;
  virtual void keyPressed(unsigned int);
  bool autoUpdate() const;
  bool expectBackendPush() const;
};
