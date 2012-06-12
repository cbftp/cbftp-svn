#pragma once
#include "uiwindow.h"
#include "ftpthread.h"
#include "rawbuffer.h"
#include "uiwindowcommand.h"
#include "sitethread.h"
#include "sitethreadmanager.h"
#include "globalcontext.h"
#include <iostream>
#include <vector>

extern GlobalContext * global;

class RawDataScreen : public UIWindow {
public:
  RawDataScreen(WINDOW *, UIWindowCommand *, int, int);
  void redraw();
  void update();
  void keyPressed(int);
private:
  int threads;
  std::string sitename;
  int threadid;
  UIWindowCommand * windowcommand;
  RawBuffer * rawbuf;
};
