#pragma once

#include <list>
#include <semaphore.h>
#include <cstdlib>

#define BLOCKSIZE 2048
#define MAXBLOCKS 2048

class DataBlockPool {
private:
  std::list<char *> blocks;
  int totalblocks;
  sem_t blocksem;
public:
  DataBlockPool();
  char * getBlock();
  int blockSize();
  void returnBlock(char *);
};
