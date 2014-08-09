#pragma once

#include "../uiwindow.h"

class RawBuffer;

class EventLogScreen : public UIWindow {
public:
  EventLogScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  bool readfromcopy;
  unsigned int copyreadpos;
  unsigned int copysize;
  RawBuffer * rawbuf;
};
