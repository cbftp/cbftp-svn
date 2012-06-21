#pragma once

#include <iostream>
#include <list>

#include "sitethreadmanager.h"
#include "transfermanager.h"
#include "sitemanager.h"
#include "globalcontext.h"
#include "engine.h"
#include "ui.h"

GlobalContext * global;

class Main {
  private:
    Engine * e;
    UserInterface * ui;
    SiteManager * sm;
    SiteThreadManager * stm;
    TransferManager * tm;
  public:
    Main();
};

int main(int, char **);

bool forever;

void sighandler(int);
void sighandler_ignore(int);


