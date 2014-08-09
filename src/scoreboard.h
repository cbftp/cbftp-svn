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
    void add(std::string, int, bool, SiteLogic *, FileList *, SiteLogic *, FileList *);
    unsigned int size() const;
    std::vector<ScoreBoardElement *>::const_iterator begin() const;
    std::vector<ScoreBoardElement *>::const_iterator end() const;
    void sort();
    const std::vector<ScoreBoardElement *> * getElementVector() const;
    void wipe();
};
