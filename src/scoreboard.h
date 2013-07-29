#pragma once

#include <vector>
#include <algorithm>

#include "scoreboardelement.h"

bool comparator(ScoreBoardElement *, ScoreBoardElement *);

class ScoreBoard {
  private:
    std::vector<ScoreBoardElement *> elements;
  public:
    ScoreBoard();
    void add(std::string, int, SiteLogic *, FileList *, SiteLogic *, FileList *);
    int size();
    std::vector<ScoreBoardElement *>::iterator begin();
    std::vector<ScoreBoardElement *>::iterator end();
    void sort();
    std::vector<ScoreBoardElement *> getElementVector();
    void wipe();
};
