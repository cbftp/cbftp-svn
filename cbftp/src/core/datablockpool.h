#pragma once

#include <list>

#include "semaphore.h"
#include "lock.h"

#define BLOCKSIZE 16384
#define MAXBLOCKS 2048

class DataBlockPool {
private:
  std::list<char *> blocks;
  int totalblocks;
  Semaphore blocksem;
  Lock blocklock;
  bool waitingforblocks;
  void allocateNewBlocks();
  unsigned int currentMaxNumBlocks() const;
public:
  DataBlockPool();
  char * getBlock();
  const int blockSize() const;
  void returnBlock(char *);
  void awaitFreeBlocks();
};
