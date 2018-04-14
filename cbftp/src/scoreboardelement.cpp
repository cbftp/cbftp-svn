#include "scoreboardelement.h"

#include "race.h"

ScoreBoardElement::ScoreBoardElement(const std::string & filename, unsigned short score,
    bool prio, const Pointer<SiteLogic> & src, FileList * fls, SiteRace * srcsr,
    const Pointer<SiteLogic> & dst, FileList * fld, SiteRace * dstsr, Pointer<Race> & race,
    const std::string & subdir)
{
  reset(filename, score, prio, src, fls, srcsr, dst, fld, dstsr, race, subdir);
}

void ScoreBoardElement::reset(const std::string & filename, unsigned short score,
    bool prio, const Pointer<SiteLogic> & src, FileList * fls, SiteRace * srcsr,
    const Pointer<SiteLogic> & dst, FileList * fld, SiteRace * dstsr, Pointer<Race> & race,
    const std::string & subdir)
{
  this->filename = filename;
  this->src = src;
  this->fls = fls;
  this->srcsr = srcsr;
  this->dst = dst;
  this->fld = fld;
  this->dstsr = dstsr;
  this->race = race;
  this->score = score;
  this->prio = prio;
  attempted = false;
  this->subdir = subdir;
}

const std::string & ScoreBoardElement::fileName() const {
  return filename;
}

const std::string & ScoreBoardElement::subDir() const {
  return subdir;
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

SiteRace * ScoreBoardElement::getSourceSiteRace() const {
  return srcsr;
}

SiteRace * ScoreBoardElement::getDestinationSiteRace() const {
  return dstsr;
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
