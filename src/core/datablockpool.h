#pragma once

#include <list>

#include "lock.h"

class DataBlockPool {
private:
  std::list<char *> blocks;
  std::list<char *> availableblocks;
  int totalblocks;
  Lock blocklock;
  void allocateNewBlocks();
public:
  DataBlockPool();
  ~DataBlockPool();
  char * getBlock();
  const int blockSize() const;
  void returnBlock(char *);
};
