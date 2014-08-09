#pragma once

#include <list>
#include <semaphore.h>
#include <pthread.h>


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
  const int blockSize() const;
  void returnBlock(char *);
};
