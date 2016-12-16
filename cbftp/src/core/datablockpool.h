#pragma once

#include <list>

#include "lock.h"

class DataBlockPool {
private:
  std::list<char *> blocks;
  int totalblocks;
  Lock blocklock;
  void allocateNewBlocks();
public:
  DataBlockPool();
  char * getBlock();
  const int blockSize() const;
  void returnBlock(char *);
};
