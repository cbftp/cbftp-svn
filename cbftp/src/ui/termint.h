#pragma once

#include <string>

struct _win_st;
typedef struct _win_st WINDOW;

#define BOX_HLINE 4194417
#define BOX_HLINE_TOP 4194422
#define BOX_HLINE_BOT 4194423
#define BOX_VLINE 4194424
#define BOX_VLINE_R 4194420
#define BOX_VLINE_L 4194421
#define BOX_CORNER_BR 4194410
#define BOX_CORNER_BL 4194413
#define BOX_CORNER_TL 4194412
#define BOX_CORNER_TR 4194411
#define BOX_CROSS 4194414
#define BOX_MIN BOX_CORNER_BR
#define BOX_MAX BOX_VLINE

class TermInt {
private:
  static unsigned int cursorrow;
  static unsigned int cursorcol;
  static WINDOW * cursorwindow;
public:
  static void printChar(WINDOW *, unsigned int, unsigned int, unsigned int);
  static void printStr(WINDOW *, unsigned int, unsigned int, std::string);
  static void printStr(WINDOW *, unsigned int, unsigned int, std::string, unsigned int);
  static void printStr(WINDOW *, unsigned int, unsigned int, std::string, unsigned int, bool);
  static void moveCursor(WINDOW *, unsigned int, unsigned int);
};

