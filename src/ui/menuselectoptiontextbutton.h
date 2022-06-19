#pragma once

#include "fmtstring.h"

#include "menuselectoptionelement.h"
#include "resizableelement.h"

class MenuSelectOptionTextButton : public ResizableElement {
private:
  FmtString text;
  std::string legendtext;
public:
  MenuSelectOptionTextButton(std::string, int, int, std::string);
  MenuSelectOptionTextButton(std::string, int, int, std::string, bool);
  FmtString getContentText() const override;
  FmtString getLabelText() const override;
  bool isActivated() const;
  bool activate() override;
  unsigned int wantedWidth() const override;
  std::string getLegendText() const override;
  void setLegendText(const std::string& text);
  virtual unsigned int getTotalWidth() const override;
};
