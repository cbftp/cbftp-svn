#pragma once

#include <ncurses.h>
#include <unistd.h>
#include <string>

class UserInterface;

class UIWindow {
protected:
  WINDOW * window;
  int row;
  int col;
public:
  void init(WINDOW *, int, int);
  virtual ~UIWindow();
  virtual void redraw() = 0;
  void resize(int, int);
  virtual void update();
  virtual std::string getLegendText();
  virtual void keyPressed(int);
};
