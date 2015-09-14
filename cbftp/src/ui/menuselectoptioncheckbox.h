#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "resizableelement.h"

class MenuSelectOptionCheckBox : public ResizableElement {
private:
  bool value;
public:
  MenuSelectOptionCheckBox(std::string, int, int, std::string, bool);
  std::string getContentText() const;
  bool activate();
  bool getData() const;
  unsigned int wantedWidth() const;
};
