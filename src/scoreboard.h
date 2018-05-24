#pragma once

#include <vector>
#include <string>

#include "core/pointer.h"

class ScoreBoardElement;
class SiteLogic;
class FileList;
class Race;
class SiteRace;

#define RESIZE_CHUNK 1000
#define USHORT_MAX 0x10000

class ScoreBoard {
  private:
    std::vector<ScoreBoardElement *> elements;
    std::vector<ScoreBoardElement *> elementstmp;
    unsigned int showsize;
    unsigned int * count;
    unsigned int * bucketpositions;
    unsigned int countarraybytesize;
    void shuffle(unsigned int firstpos, unsigned int lastpos);
  public:
    ScoreBoard();
    ~ScoreBoard();
    void add(const std::string &, unsigned short, bool, const Pointer<SiteLogic> &,
        FileList *, SiteRace *, const Pointer<SiteLogic> &, FileList *, SiteRace *, Pointer<Race> &, const std::string &);
    unsigned int size() const;
    std::vector<ScoreBoardElement *>::const_iterator begin() const;
    std::vector<ScoreBoardElement *>::const_iterator end() const;
    void sort();
    void shuffleEquals();
    const std::vector<ScoreBoardElement *> & getElementVector() const;
    void wipe();
};
