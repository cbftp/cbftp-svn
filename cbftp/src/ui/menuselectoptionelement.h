#pragma once

#include <string>

class MenuSelectOptionElement {
  private:
    std::string identifier;
    int col;
    int row;
    bool shown;
    bool selectable;
    unsigned int id;
    void * origin;
  protected:
    std::string label;
    bool active;
  public:
    void init(std::string, int, int, std::string);
    virtual ~MenuSelectOptionElement();
    void setPosition(int, int);
    virtual std::string getLabelText() const;
    std::string getIdentifier() const;
    unsigned int getId() const;
    virtual std::string getContentText() const = 0;
    virtual bool activate();
    virtual void deactivate();
    virtual bool isActive() const;
    virtual int cursorPosition() const;
    virtual void inputChar(int);
    void setOrigin(void * origin);
    void * getOrigin() const;
    virtual std::string getLegendText() const;
    unsigned int getCol() const;
    unsigned int getRow() const;
    void hide();
    void show();
    bool visible() const;
    bool isSelectable() const;
    void setSelectable(bool);
    void setId(unsigned int);
    void setLabel(const std::string &);
};
