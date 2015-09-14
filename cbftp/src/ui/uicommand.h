#pragma once

#include <string>

struct _win_st;
typedef struct _win_st WINDOW;

#define UI_COMMAND_REFRESH 83230
#define UI_COMMAND_HIGHLIGHT_OFF 83231
#define UI_COMMAND_HIGHLIGHT_ON 83232
#define UI_COMMAND_CURSOR_SHOW 83233
#define UI_COMMAND_CURSOR_HIDE 83234
#define UI_COMMAND_CURSOR_MOVE 83235
#define UI_COMMAND_ERASE 83236
#define UI_COMMAND_PRINT_STR 83237
#define UI_COMMAND_PRINT_CHAR 83238
#define UI_COMMAND_INIT 83239
#define UI_COMMAND_KILL 83240
#define UI_COMMAND_RESIZE 83241
#define UI_COMMAND_ADJUST_LEGEND 83242
#define UI_COMMAND_ADJUST_INFO 83243

class UICommand {
public:
  UICommand(int);
  UICommand(int, bool);
  UICommand(int, WINDOW *, unsigned int, unsigned int);
  UICommand(int, WINDOW *);
  UICommand(int, WINDOW *, unsigned int, unsigned int, std::string, int, bool);
  UICommand(int, WINDOW *, unsigned int, unsigned int, unsigned int);
  int getCommand() const;
  WINDOW * getWindow() const;
  unsigned int getRow() const;
  unsigned int getCol() const;
  std::string getText() const;
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
  int maxlen;
  bool rightalign;
  unsigned int c;
  bool show;
};
