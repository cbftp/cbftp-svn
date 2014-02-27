#include "scoreboard.h"

#include <algorithm>

#include "scoreboardelement.h"

bool comparator(ScoreBoardElement * e1, ScoreBoardElement * e2) {
  return (e1->getScore() > e2->getScore());
}

ScoreBoard::ScoreBoard() {
  showsize = 0;
}

void ScoreBoard::add(std::string name, int score, bool prio, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld) {
  if (showsize == elements.size()) {
    elements.push_back(new ScoreBoardElement(name, score, prio, src, fls, dst, fld));
  }
  else {
    elements[showsize]->reset(name, score, prio, src, fls, dst, fld);
  }
  showsize++;
}

void ScoreBoard::sort() {
  std::sort(begin(), end(), comparator);
}

std::vector<ScoreBoardElement *>::iterator ScoreBoard::begin() {
  return elements.begin();
}

std::vector<ScoreBoardElement *>::iterator ScoreBoard::end() {
  return elements.begin() + showsize;
}

unsigned int ScoreBoard::size() {
  return showsize;
}

std::vector<ScoreBoardElement *> * ScoreBoard::getElementVector() {
  return &elements;
}

void ScoreBoard::wipe() {
  showsize = 0;
}
