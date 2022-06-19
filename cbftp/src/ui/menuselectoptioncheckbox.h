#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "resizableelement.h"

class MenuSelectOptionCheckBox : public ResizableElement {
private:
  bool value;
public:
  MenuSelectOptionCheckBox(std::string, int, int, std::string, bool);
  FmtString getContentText() const override;
  bool activate() override;
  void setValue(bool value);
  bool getData() const;
  unsigned int wantedWidth() const override;
};
