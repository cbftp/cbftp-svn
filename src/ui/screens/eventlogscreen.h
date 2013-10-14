#pragma once

#include "../uiwindow.h"

class UICommunicator;
class RawBuffer;

class EventLogScreen : public UIWindow {
public:
  EventLogScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  bool readfromcopy;
  unsigned int copyreadpos;
  unsigned int copysize;
  UICommunicator * uicommunicator;
  RawBuffer * rawbuf;
};
