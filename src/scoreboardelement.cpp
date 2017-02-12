#include "scoreboardelement.h"

#include "race.h"

ScoreBoardElement::ScoreBoardElement(const std::string & filename, unsigned short score, bool prio, const Pointer<SiteLogic> & src, FileList * fls, const Pointer<SiteLogic> & dst, FileList * fld, Pointer<Race> & race) {
  reset(filename, score, prio, src, fls, dst, fld, race);
}

void ScoreBoardElement::reset(const std::string & filename, unsigned short score, bool prio, const Pointer<SiteLogic> & src, FileList * fls, const Pointer<SiteLogic> & dst, FileList * fld, Pointer<Race> & race) {
  this->filename = filename;
  this->src = src;
  this->fls = fls;
  this->dst = dst;
  this->fld = fld;
  this->race = race;
  this->score = score;
  this->prio = prio;
  attempted = false;
}

const std::string & ScoreBoardElement::fileName() const {
  return filename;
}

const Pointer<SiteLogic> & ScoreBoardElement::getSource() const {
  return src;
}

const Pointer<SiteLogic> & ScoreBoardElement::getDestination() const {
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

bool ScoreBoardElement::wasAttempted() const {
  return attempted;
}

void ScoreBoardElement::setAttempted() {
  attempted = true;
}

std::ostream & operator<<(std::ostream & out, const ScoreBoardElement & sbe) {
  return out << sbe.fileName() << " - " << sbe.getScore();
}
