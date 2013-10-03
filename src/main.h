#pragma once

#define DATAFILE "data"
#define DATAPATH ".clusterbomb"

class GlobalContext;

GlobalContext * global;

class Main {
private:
public:
  Main();
};

int main(int, char **);

bool forever;

void sighandler(int);
void sighandler_winch(int);
void sighandler_ignore(int);


