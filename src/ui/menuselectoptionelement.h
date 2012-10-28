#pragma once

#include <string>

class MenuSelectOptionElement {
  private:
    std::string identifier;
    std::string label;
    int col;
    int row;
  protected:
    bool active;
  public:
    void init(std::string, int, int, std::string);
    virtual ~MenuSelectOptionElement();
    std::string getLabelText();
    std::string getIdentifier();
    virtual std::string getContentText() = 0;
    virtual bool activate();
    virtual void deactivate();
    virtual int cursorPosition();
    virtual void inputChar(int);
    virtual std::string getLegendText();
    int getCol();
    int getRow();
};
