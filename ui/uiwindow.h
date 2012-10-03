#pragma once

#include <ncurses.h>
#include <unistd.h>
#include <string>

class UserInterface;

class UIWindow {
protected:
  WINDOW * window;
  unsigned int row;
  unsigned int col;
public:
  void init(WINDOW *, unsigned int, unsigned int);
  virtual ~UIWindow();
  virtual void redraw() = 0;
  void resize(unsigned int, unsigned int);
  virtual void update();
  virtual std::string getLegendText();
  virtual void keyPressed(unsigned int);
};
