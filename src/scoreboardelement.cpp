#include "scoreboardelement.h"

ScoreBoardElement::ScoreBoardElement(std::string filename, int score, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld) {
  reset(filename, score, src, fls, dst, fld);
}

void ScoreBoardElement::reset(std::string filename, int score, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld) {
  this->filename = filename;
  this->src = src;
  this->fls = fls;
  this->dst = dst;
  this->fld = fld;
  this->score = score;
}

std::string ScoreBoardElement::fileName() {
  return filename;
}

SiteLogic * ScoreBoardElement::getSource() {
  return src;
}

SiteLogic * ScoreBoardElement::getDestination() {
  return dst;
}

FileList * ScoreBoardElement::getSourceFileList() {
  return fls;
}

FileList * ScoreBoardElement::getDestinationFileList() {
  return fld;
}

int ScoreBoardElement::getScore() {
  return score;
}
