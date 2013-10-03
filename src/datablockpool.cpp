#include "datablockpool.h"

#include <cstdlib>

DataBlockPool::DataBlockPool() {
  char * block;
  totalblocks = 0;
  sem_init(&blocksem, 0, 0);
  pthread_mutex_init(&blockmutex, NULL);
  pthread_mutex_lock(&blockmutex);
  for (int i = 0; i < 10; i++) {
    block = (char *) malloc(BLOCKSIZE);
    blocks.push_back(block);
    totalblocks++;
  }
  pthread_mutex_unlock(&blockmutex);
}

char * DataBlockPool::getBlock() {
  char * block;
  pthread_mutex_lock(&blockmutex);
  if (blocks.size() == 0) {
    for (int i = 0; i < 10 && totalblocks < MAXBLOCKS; i++) {
      block = (char *) malloc(BLOCKSIZE);
      blocks.push_back(block);
      totalblocks++;
    }
  }
  while (blocks.size() == 0) {
    pthread_mutex_unlock(&blockmutex);
    sem_wait(&blocksem);
    pthread_mutex_lock(&blockmutex);
  }
  block = blocks.back();
  blocks.pop_back();
  pthread_mutex_unlock(&blockmutex);
  return block;
}

int DataBlockPool::blockSize() {
  return BLOCKSIZE;
}

void DataBlockPool::returnBlock(char * block) {
  pthread_mutex_lock(&blockmutex);
  blocks.push_back(block);
  if (blocks.size() == 1) {
    sem_post(&blocksem);
  }
  pthread_mutex_unlock(&blockmutex);
}
