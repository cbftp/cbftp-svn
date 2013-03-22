#pragma once

#include <iostream>
#include <list>
#include <sys/stat.h>
#include <unistd.h>
#include <error.h>

#include "datafilehandler.h"
#include "sitethreadmanager.h"
#include "transfermanager.h"
#include "sitemanager.h"
#include "globalcontext.h"
#include "engine.h"
#include "ui/ui.h"
#include "ui/uicommunicator.h"
#include "tickpoke.h"
#include "remotecommandhandler.h"

#define DATAFILE "data"
#define DATAPATH ".clusterbomb"

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


