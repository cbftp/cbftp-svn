#pragma once

#include <string>

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
  UI_COMMAND_PRINT_WIDE_STR,
  UI_COMMAND_PRINT_CHAR,
  UI_COMMAND_INIT,
  UI_COMMAND_KILL,
  UI_COMMAND_RESIZE,
  UI_COMMAND_ADJUST_LEGEND,
  UI_COMMAND_ADJUST_INFO
};

class UICommand {
public:
  UICommand(int);
  UICommand(int, bool);
  UICommand(int, WINDOW *, unsigned int, unsigned int);
  UICommand(int, WINDOW *);
  UICommand(int, WINDOW *, unsigned int, unsigned int, std::string, int, bool);
  UICommand(int, WINDOW *, unsigned int, unsigned int, std::basic_string<unsigned int>, int, bool);
  UICommand(int, WINDOW *, unsigned int, unsigned int, unsigned int);
  int getCommand() const;
  WINDOW * getWindow() const;
  unsigned int getRow() const;
  unsigned int getCol() const;
  std::string getText() const;
  std::basic_string<unsigned int> getWideText() const;
  int getMaxlen() const;
  bool getRightAlign() const;
  unsigned int getChar() const;
  bool getShow() const;
private:
  int command;
  WINDOW * window;
  unsigned int row;
  unsigned int col;
  std::string text;
  std::basic_string<unsigned int> wtext;
  int maxlen;
  bool rightalign;
  unsigned int c;
  bool show;
};
