#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#if defined __APPLE__ || defined __FreeBSD__
#include <ncurses.h>
#else
#include <ncursesw/curses.h>
#endif
