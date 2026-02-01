#include "menuselectoptionalttextbutton.h"

MenuSelectOptionAltTextButton::MenuSelectOptionAltTextButton(const std::string& identifier, int row, int col, const std::string& longtext, const std::string& shorttext) :
  longtext(longtext), shorttext(shorttext)
{
  init(identifier, row, col, longtext);
}

FmtString MenuSelectOptionAltTextButton::getContentText() const {
  return "<NOT SUPPORTED>";
}

FmtString MenuSelectOptionAltTextButton::getLabelText() const {
  if (longtext.length() <= maxwidth) {
    std::string alignextra;
    if (rightaligned) {
      while (maxwidth > longtext.length() + alignextra.length()) {
        alignextra += " ";
      }
    }
    return FmtString(alignextra) + longtext;
  }
  if (shorttext.length() <= maxwidth) {
    std::string alignextra;
    if (rightaligned) {
      while (maxwidth > shorttext.length() + alignextra.length()) {
        alignextra += " ";
      }
    }
    return FmtString(alignextra) + shorttext;
  }
  switch (resizemethod) {
    case RESIZE_CUTEND:
      return shorttext.substr(0, maxwidth);
    case RESIZE_WITHDOTS:
    {
      std::string dots = "...";
      if (maxwidth > 3) {
        return shorttext.substr(0, maxwidth - dots.length()) + dots;
      }
      else {
        return dots.substr(0, maxwidth);
      }
    }
    case RESIZE_WITHLAST3:
      if (maxwidth > 3) {
        return shorttext.substr(0, maxwidth - 4) + "~" + shorttext.substr(shorttext.length() - 3);
      }
      else {
        return shorttext.substr(shorttext.length() - maxwidth);
      }
  }
  return label;
}

bool MenuSelectOptionAltTextButton::activate() {
  active = true;
  return false;
}

unsigned int MenuSelectOptionAltTextButton::wantedWidth() const {
  return longtext.length();
}

unsigned int MenuSelectOptionAltTextButton::alternateWantedWidth() const {
  return shorttext.length();
}

unsigned int MenuSelectOptionAltTextButton::getTotalWidth() const {
  return wantedWidth();
}
