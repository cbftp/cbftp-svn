#include "scoreboardelement.h"

ScoreBoardElement::ScoreBoardElement(std::string filename, int score, SiteThread * src, FileList * fls, SiteThread * dst, FileList * fld) {
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

SiteThread * ScoreBoardElement::getSource() {
  return src;
}

SiteThread * ScoreBoardElement::getDestination() {
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
