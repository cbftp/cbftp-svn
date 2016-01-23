#include "scoreboardelement.h"

#include "race.h"

ScoreBoardElement::ScoreBoardElement(const std::string & filename, unsigned short score, bool prio, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld, Pointer<Race> & race) {
  reset(filename, score, prio, src, fls, dst, fld, race);
}

void ScoreBoardElement::reset(const std::string & filename, unsigned short score, bool prio, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld, Pointer<Race> & race) {
  this->filename = filename;
  this->src = src;
  this->fls = fls;
  this->dst = dst;
  this->fld = fld;
  this->race = race;
  this->score = score;
  this->prio = prio;
}

const std::string & ScoreBoardElement::fileName() const {
  return filename;
}

SiteLogic * ScoreBoardElement::getSource() const {
  return src;
}

SiteLogic * ScoreBoardElement::getDestination() const {
  return dst;
}

FileList * ScoreBoardElement::getSourceFileList() const {
  return fls;
}

FileList * ScoreBoardElement::getDestinationFileList() const {
  return fld;
}

const Pointer<Race> & ScoreBoardElement::getRace() const {
  return race;
}

unsigned short ScoreBoardElement::getScore() const {
  return score;
}

bool ScoreBoardElement::isPrioritized() const {
  return prio;
}

std::ostream & operator<<(std::ostream & out, const ScoreBoardElement & sbe) {
  return out << sbe.fileName() << " - " << sbe.getScore();
}
