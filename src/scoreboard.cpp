#include "scoreboard.h"

bool comparator(ScoreBoardElement * e1, ScoreBoardElement * e2) {
  return (e1->getScore() > e2->getScore());
}

ScoreBoard::ScoreBoard() {
}

void ScoreBoard::add(std::string name, int score, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld) {
  elements.push_back(new ScoreBoardElement(name, score, src, fls, dst, fld));
}

void ScoreBoard::sort() {
  std::sort(elements.begin(), elements.end(), comparator);
}

std::vector<ScoreBoardElement *>::iterator ScoreBoard::begin() {
  return elements.begin();
}

std::vector<ScoreBoardElement *>::iterator ScoreBoard::end() {
  return elements.end();
}

int ScoreBoard::size() {
  return elements.size();
}

std::vector<ScoreBoardElement *> ScoreBoard::getElementVector() {
  return elements;
}

void ScoreBoard::wipe() {
  elements.clear();
}
