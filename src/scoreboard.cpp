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

void ScoreBoard::add(const std::string & name, unsigned short score, bool prio, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld, Pointer<Race> & race) {
  if (showsize == elements.size()) {
    elements.resize(elements.size() + RESIZE_CHUNK);
    elementstmp.resize(elements.size());
    for (int i = 0; i < RESIZE_CHUNK; i++) {
      ScoreBoardElement * sbe = new ScoreBoardElement(name, score, prio, src, fls, dst, fld, race);
      elements[showsize + i] = sbe;
    }
  }
  else {
    elements[showsize]->reset(name, score, prio, src, fls, dst, fld, race);
  }
  ++showsize;
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

std::vector<ScoreBoardElement *>::const_iterator ScoreBoard::begin() const {
  return elements.begin();
}

std::vector<ScoreBoardElement *>::const_iterator ScoreBoard::end() const {
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
}
