#pragma once

#include <list>
#include <semaphore.h>
#include <pthread.h>
#include <cstdlib>

#define BLOCKSIZE 2048
#define MAXBLOCKS 2048

class DataBlockPool {
private:
  std::list<char *> blocks;
  int totalblocks;
  sem_t blocksem;
  pthread_mutex_t blockmutex;
public:
  DataBlockPool();
  char * getBlock();
  int blockSize();
  void returnBlock(char *);
};
