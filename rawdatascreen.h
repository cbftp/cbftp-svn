#pragma once

#include "uiwindow.h"
#include "ftpthread.h"
#include "rawbuffer.h"
#include "uicommunicator.h"
#include "sitethread.h"
#include "sitethreadmanager.h"
#include "globalcontext.h"
#include "termint.h"

extern GlobalContext * global;

class RawDataScreen : public UIWindow {
public:
  RawDataScreen(WINDOW *, UICommunicator *, int, int);
  void redraw();
  void update();
  void keyPressed(int);
  std::string getLegendText();
private:
  int threads;
  std::string sitename;
  int threadid;
  UICommunicator * uicommunicator;
  RawBuffer * rawbuf;
};
