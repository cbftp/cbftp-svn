#pragma once

#include "fmtstring.h"

#include "menuselectoptionelement.h"
#include "resizableelement.h"

class MenuSelectOptionAltTextButton : public ResizableElement {
private:
  FmtString longtext;
  FmtString shorttext;
  std::string legendtext;
public:
  MenuSelectOptionAltTextButton(const std::string& identifier, int row, int col, const std::string& longtext, const std::string& shorttext);
  FmtString getContentText() const override;
  FmtString getLabelText() const override;
  bool activate() override;
  unsigned int wantedWidth() const override;
  unsigned int alternateWantedWidth() const override;
  virtual unsigned int getTotalWidth() const override;
};
