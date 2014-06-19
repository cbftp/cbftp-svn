#pragma once

#include <string>

class MenuSelectOptionElement {
  private:
    std::string identifier;
    int col;
    int row;
    bool shown;
    bool selectable;
  protected:
    std::string label;
    bool active;
  public:
    void init(std::string, int, int, std::string);
    virtual ~MenuSelectOptionElement();
    void setPosition(int, int);
    virtual std::string getLabelText();
    std::string getIdentifier();
    virtual std::string getContentText() = 0;
    virtual bool activate();
    virtual void deactivate();
    virtual bool isActive();
    virtual int cursorPosition();
    virtual void inputChar(int);
    virtual std::string getLegendText();
    unsigned int getCol();
    unsigned int getRow();
    void hide();
    void show();
    bool visible();
    bool isSelectable();
};
