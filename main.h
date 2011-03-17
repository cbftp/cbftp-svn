#include <iostream>
#include <list>
#include <signal.h>
#include "sitethreadmanager.h"
#include "transfermanager.h"
#include "sitemanager.h"
#include "globalcontext.h"
#include "engine.h"

GlobalContext * global;

bool forever;
int main(int, char **);

void sighandler(int);
