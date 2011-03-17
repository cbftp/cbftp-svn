#include "scoreboardelement.h"

ScoreBoardElement::ScoreBoardElement(std::string filename, int score, SiteThread * src, SiteRace * srs, SiteThread * dst, SiteRace * srd) {
  this->filename = filename;
  this->src = src;
  this->srs = srs;
  this->dst = dst;
  this->srd = srd;
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

SiteRace * ScoreBoardElement::getSourceSiteRace() {
  return srs;
}

SiteRace * ScoreBoardElement::getDestinationSiteRace() {
  return srd;
}

int ScoreBoardElement::getScore() {
  return score;
}
