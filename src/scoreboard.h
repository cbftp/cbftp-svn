#pragma once

#include <vector>
#include <string>

class ScoreBoardElement;
class SiteLogic;
class FileList;

bool comparator(ScoreBoardElement *, ScoreBoardElement *);

class ScoreBoard {
  private:
    std::vector<ScoreBoardElement *> elements;
    unsigned int showsize;
  public:
    ScoreBoard();
    void add(std::string, int, SiteLogic *, FileList *, SiteLogic *, FileList *);
    unsigned int size();
    std::vector<ScoreBoardElement *>::iterator begin();
    std::vector<ScoreBoardElement *>::iterator end();
    void sort();
    std::vector<ScoreBoardElement *> * getElementVector();
    void wipe();
};
