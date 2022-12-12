#pragma once

#include "../menuselectoptiontextfield.h"
#include "../uiwindow.h"

class RawBuffer;

class EventLogScreen : public UIWindow {
public:
  EventLogScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw() override;
  void update() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
private:
  void fixCopyReadPos();
  bool readfromcopy;
  unsigned int copyreadpos;
  RawBuffer * rawbuf;
  std::string filtertext;
  bool filtermodeinput;
  bool filtermodeinputregex;
  MenuSelectOptionTextField filterfield;
};
