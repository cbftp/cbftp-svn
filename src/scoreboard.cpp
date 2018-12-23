#include "scoreboard.h"

#include <cstring>
#include <algorithm>

#include "scoreboardelement.h"
#include "race.h"

ScoreBoard::ScoreBoard() :
  showsize(0),
  count(new unsigned int[USHORT_MAX]),
  bucketpositions(new unsigned int[USHORT_MAX]),
  countarraybytesize(USHORT_MAX * sizeof(unsigned int))
{
}

ScoreBoard::~ScoreBoard() {
  delete[] count;
  delete[] bucketpositions;
}

void ScoreBoard::update(
    const std::string & name, unsigned short score, unsigned long long int filesize,
    PrioType priotype,
    const std::shared_ptr<SiteLogic> & src, FileList * fls, SiteRace * srs,
    const std::shared_ptr<SiteLogic> & dst, FileList * fld, SiteRace * srd,
    const std::shared_ptr<Race> & race, const std::string & subdir)
{
  auto flsit = elementlocator.find(fls);
  if (flsit != elementlocator.end()) {
    std::unordered_map<FileList *, std::unordered_map<std::string, ScoreBoardElement *>> & fldmap = flsit->second;
    auto fldit = fldmap.find(fld);
    if (fldit != fldmap.end()) {
      std::unordered_map<std::string, ScoreBoardElement *> & filemap = fldit->second;
      auto fileit = filemap.find(name);
      if (fileit != filemap.end()) {
        fileit->second->update(score);
        return;
      }
    }
  }
  if (showsize == elements.size()) {
    elements.resize(elements.size() + RESIZE_CHUNK);
    elementstmp.resize(elements.size());
    for (int i = 0; i < RESIZE_CHUNK; i++) {
      ScoreBoardElement * sbe = new ScoreBoardElement(name, score, filesize, priotype, src, fls, srs, dst, fld, srd, race, subdir);
      elements[showsize + i] = sbe;
    }
  }
  else {
    elements[showsize]->reset(name, score, filesize, priotype, src, fls, srs, dst, fld, srd, race, subdir);
  }
  elementlocator[fls][fld][name] = elements[showsize];
  ++showsize;
}

void ScoreBoard::remove(const std::string & name, FileList * fls, FileList * fld) {
  auto flsit = elementlocator.find(fls);
  if (flsit == elementlocator.end()) {
    return;
  }
  std::unordered_map<FileList *, std::unordered_map<std::string, ScoreBoardElement *>> & fldmap = flsit->second;
  auto fldit = fldmap.find(fld);
  if (fldit == fldmap.end()) {
    return;
  }
  std::unordered_map<std::string, ScoreBoardElement *> & filemap = fldit->second;
  auto fileit = filemap.find(name);
  if (fileit == filemap.end()) {
    return;
  }
  ScoreBoardElement * sbe = fileit->second;
  filemap.erase(fileit);
  if (showsize && sbe != elements[showsize - 1]) {
    sbe->reset(*elements[showsize - 1]);
    elementlocator[sbe->getSourceFileList()][sbe->getDestinationFileList()][sbe->fileName()] = sbe;
  }
  --showsize;
}

void ScoreBoard::sort() {
  // base2^16 single-pass radix sort
  memset(count, 0, countarraybytesize);
  for (unsigned int i = 0; i < showsize; i++) {
    ++count[elements[i]->getScore()];
  }
  unsigned int currentpos = showsize - 1;
  for (unsigned int i = 0; i < USHORT_MAX; currentpos -= count[i++]) {
    bucketpositions[i] = currentpos;
  }
  for (unsigned int i = 0; i < showsize; i++) {
    ScoreBoardElement * currentelem = elements[i];
    elementstmp[bucketpositions[currentelem->getScore()]--] = currentelem;
  }
  for (unsigned int i = 0; i < showsize; i++) {
    elements[i] = elementstmp[i];
  }
}

/*void ScoreBoard::sort() {
  // base2^8 LSD radix sort
  for (unsigned char shift = 0; shift < 16; shift += 8) {
    memset(count, 0, 0x100 * sizeof(unsigned int);
    for (unsigned int i = 0; i < showsize; i++) {
      ++count[(elements[i]->getScore() >> shift) & 0xFF];
    }
    unsigned int currentpos = showsize - 1;
    for (unsigned short i = 0; i < 0x100; currentpos -= count[i++]) {
      bucketpositions[i] = currentpos;
    }
    for (unsigned int i = 0; i < showsize; i++) {
      ScoreBoardElement * currentelem = elements[i];
      elementstmp[bucketpositions[(currentelem->getScore() >> shift) & 0xFF]--] = currentelem;
    }
    for (unsigned int i = 0; i < showsize; i++) {
      elements[i] = elementstmp[i];
    }
  }
}*/

void ScoreBoard::shuffleEquals() {
  if (!showsize) {
    return;
  }
  unsigned short lastscore = 0;
  unsigned int equalstart = 0;
  for (unsigned int i = 0; i < showsize; i++) {
    unsigned short score = elements[i]->getScore();
    if (score != lastscore) {
      if (i > 0) {
        shuffle(equalstart, i - 1);
      }
      equalstart = i;
      lastscore = score;
    }
  }
  shuffle(equalstart, showsize - 1);
}

void ScoreBoard::shuffle(unsigned int firstpos, unsigned int lastpos) {
  ScoreBoardElement * tmp;
  for (unsigned int i = firstpos; i < lastpos; i++) {
    unsigned int swappos = i + rand() % (lastpos - i + 1);
    tmp = elements[i];
    elements[i] = elements[swappos];
    elements[swappos] = tmp;
  }
}

std::vector<ScoreBoardElement *>::const_iterator ScoreBoard::begin() const {
  return elements.begin();
}

std::vector<ScoreBoardElement *>::const_iterator ScoreBoard::end() const {
  return elements.begin() + showsize;
}

std::vector<ScoreBoardElement *>::iterator ScoreBoard::begin() {
  return elements.begin();
}

std::vector<ScoreBoardElement *>::iterator ScoreBoard::end() {
  return elements.begin() + showsize;
}

unsigned int ScoreBoard::size() const {
  return showsize;
}

const std::vector<ScoreBoardElement *> & ScoreBoard::getElementVector() const{
  return elements;
}

void ScoreBoard::wipe() {
  showsize = 0;
  elementlocator.clear();
}

void ScoreBoard::wipe(FileList * fl) {
  auto flsit = elementlocator.find(fl);
  if (flsit == elementlocator.end()) {
    return;
  }
  elementlocator.erase(flsit);
  for (int i = showsize - 1; i >= 0; i--) {
    ScoreBoardElement * sbe = elements[i];
    if (sbe->getSourceFileList() == fl) {
      if ((unsigned int)i < showsize - 1) {
        sbe->reset(*elements[showsize - 1]);
        elementlocator[sbe->getSourceFileList()][sbe->getDestinationFileList()][sbe->fileName()] = sbe;
      }
      --showsize;
    }
  }
}
