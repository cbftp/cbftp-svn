#include "scoreboard.h"

#include <cstring>
#include <algorithm>

#include "scoreboardelement.h"
#include "race.h"

ScoreBoard::ScoreBoard() :
  currelements(&elements),
  currelementstmp(&elementstmp),
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

void ScoreBoard::add(std::string name, unsigned short score, bool prio, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld, Pointer<Race> race) {
  if (showsize == currelements->size()) {
    currelements->resize(currelements->size() + RESIZE_CHUNK);
    currelementstmp->resize(currelements->size());
    for (int i = 0; i < RESIZE_CHUNK; i++) {
      ScoreBoardElement * sbe = new ScoreBoardElement(name, score, prio, src, fls, dst, fld, race);
      (*currelements)[showsize + i] = sbe;
      (*currelementstmp)[showsize + i] = sbe;
    }
  }
  else {
    (*currelements)[showsize]->reset(name, score, prio, src, fls, dst, fld, race);
  }
  ++showsize;
}

void ScoreBoard::sort() {
  // base2^16 single-pass radix sort
  memset(count, 0, countarraybytesize);
  for (unsigned int i = 0; i < showsize; i++) {
    ++count[(*currelements)[i]->getScore()];
  }
  unsigned int currentpos = showsize - 1;
  for (unsigned int i = 0; i < USHORT_MAX; currentpos -= count[i++]) {
    bucketpositions[i] = currentpos;
  }
  for (unsigned int i = showsize - 1; i < showsize; i--) {
    ScoreBoardElement * currentelem = (*currelements)[i];
    (*currelementstmp)[bucketpositions[currentelem->getScore()]--] = currentelem;
  }
  std::vector<ScoreBoardElement *> * swap = currelementstmp;
  currelementstmp = currelements;
  currelements = swap;
}

/*void ScoreBoard::sort() {
  // base2^8 LSD radix sort
  for (unsigned char shift = 0; shift < 16; shift += 8) {
    memset(count, 0, 0x100 * sizeof(unsigned int);
    for (unsigned int i = 0; i < showsize; i++) {
      count[((*currelements)[i]->getScore() >> shift) & 0xFF]++;
    }
    unsigned int currentpos = showsize - 1;
    for (unsigned short i = 0; i < 0x100; currentpos -= count[i++]) {
      bucketpositions[i] = currentpos;
    }
    for (unsigned int i = showsize - 1; i < showsize; i--) {
      ScoreBoardElement * currentelem = (*currelements)[i];
      (*currelementstmp)[bucketpositions[(currentelem->getScore() >> shift) & 0xFF]--] = currentelem;
    }
    std::vector<ScoreBoardElement *> * swap = currelementstmp;
    currelementstmp = currelements;
    currelements = swap;
  }
}*/

std::vector<ScoreBoardElement *>::const_iterator ScoreBoard::begin() const {
  return currelements->begin();
}

std::vector<ScoreBoardElement *>::const_iterator ScoreBoard::end() const {
  return currelements->begin() + showsize;
}

unsigned int ScoreBoard::size() const {
  return showsize;
}

const std::vector<ScoreBoardElement *> * ScoreBoard::getElementVector() const{
  return currelements;
}

void ScoreBoard::wipe() {
  showsize = 0;
}
