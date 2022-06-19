#pragma once

#include "fmtstring.h"

struct _win_st;
typedef struct _win_st WINDOW;

enum UICommandType {
  UI_COMMAND_REFRESH,
  UI_COMMAND_HIGHLIGHT_OFF,
  UI_COMMAND_HIGHLIGHT_ON,
  UI_COMMAND_CURSOR_SHOW,
  UI_COMMAND_CURSOR_HIDE,
  UI_COMMAND_CURSOR_MOVE,
  UI_COMMAND_ERASE,
  UI_COMMAND_PRINT_STR,
  UI_COMMAND_PRINT_CHAR,
  UI_COMMAND_INIT,
  UI_COMMAND_KILL,
  UI_COMMAND_RESIZE,
  UI_COMMAND_ADJUST,
  UI_COMMAND_BELL,
  UI_COMMAND_COLOR_ON,
  UI_COMMAND_COLOR_OFF,
  UI_COMMAND_BOLD_ON,
  UI_COMMAND_BOLD_OFF,
  UI_COMMAND_DEFAULT_COLORS
};

class UICommand {
public:
  UICommand(int command);
  UICommand(int command, int fgcolor, int bgcolor);
  UICommand(int command, bool show);
  UICommand(int command, bool infoenabled, bool legendenabled);
  UICommand(int command, WINDOW *, int);
  UICommand(int command, WINDOW *, unsigned int, unsigned int);
  UICommand(int command, WINDOW *);
  UICommand(int command, WINDOW *, unsigned int, unsigned int, const FmtString& str, int, bool);
  UICommand(int command, WINDOW *, unsigned int, unsigned int, unsigned int);
  int getCommand() const;
  WINDOW * getWindow() const;
  unsigned int getRow() const;
  unsigned int getCol() const;
  FmtString getText() const;
  int getMaxlen() const;
  bool getRightAlign() const;
  unsigned int getChar() const;
  int getColor() const;
  int getBgColor() const;
  bool getShow() const;
  bool getInfoEnabled() const;
  bool getLegendEnabled() const;
private:
  int command;
  WINDOW * window;
  unsigned int row;
  unsigned int col;
  FmtString text;
  int maxlen;
  bool rightalign;
  unsigned int c;
  bool show;
  bool infoenabled;
  bool legendenabled;
  int color;
  int bgcolor;
};
