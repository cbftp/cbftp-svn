#pragma once

#include <string>

class MenuSelectOptionElement {
  private:
    bool str;
    std::string identifier;
    std::string label;
    std::string strcontent;
    int intcontent;
    int col;
    int row;
  public:
    MenuSelectOptionElement(int, int, std::string, std::string, std::string);
    MenuSelectOptionElement(int, int, std::string, std::string, int);
    std::string getIdentifier();
    std::string getLabel();
    std::string getContent();
    void setContent(std::string);
    void setIntContent(int);
    int getIntContent();
    bool hasStrValue();
    int getCol();
    int getRow();
};
