#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "resizableelement.h"

class MenuSelectOptionTextButton : public ResizableElement {
private:
  std::string text;
  std::string legendtext;
public:
  MenuSelectOptionTextButton(std::string, int, int, std::string);
  MenuSelectOptionTextButton(std::string, int, int, std::string, bool);
  std::string getContentText() const;
  std::string getLabelText() const;
  bool isActivated() const;
  bool activate();
  unsigned int wantedWidth() const;
  std::string getLegendText() const override;
  void setLegendText(const std::string& text);
};
