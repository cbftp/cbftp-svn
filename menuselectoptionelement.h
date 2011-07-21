#pragma once

#include <string>

class MenuSelectOptionElement {
  private:
    bool str;
    bool checkbox;
    std::string identifier;
    std::string label;
    std::string strcontent;
    int intcontent;
    int col;
    int row;
  public:
    MenuSelectOptionElement(int, int, std::string, std::string, std::string);
    MenuSelectOptionElement(int, int, std::string, std::string, int, bool);
    std::string getIdentifier();
    std::string getLabel();
    std::string getContent();
    void setContent(std::string);
    void setIntContent(int);
    int getIntContent();
    bool hasStrValue();
    bool isCheckBox();
    int getCol();
    int getRow();
};
