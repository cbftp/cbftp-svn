#include "datablockpool.h"

DataBlockPool::DataBlockPool() {
  char * block;
  totalblocks = 0;
  sem_init(&blocksem, 0, 0);
  for (int i = 0; i < 10; i++) {
    block = (char *) malloc(BLOCKSIZE);
    blocks.push_back(block);
    totalblocks++;
  }
}

char * DataBlockPool::getBlock() {
  char * block;
  if (blocks.size() == 0) {
    for (int i = 0; i < 10 && totalblocks < MAXBLOCKS; i++) {
      block = (char *) malloc(BLOCKSIZE);
      blocks.push_back(block);
      totalblocks++;
    }
  }
  while (blocks.size() == 0) {
    sem_wait(&blocksem);
  }
  block = blocks.back();
  blocks.pop_back();
  return block;
}

int DataBlockPool::blockSize() {
  return BLOCKSIZE;
}

void DataBlockPool::returnBlock(char * block) {
  blocks.push_back(block);
  if (blocks.size() == 1) {
    sem_post(&blocksem);
  }
}
