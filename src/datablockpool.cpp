#include "datablockpool.h"

#include <cstdlib>

#include "scopelock.h"

DataBlockPool::DataBlockPool() {
  char * block;
  totalblocks = 0;
  ScopeLock lock(blocklock);
  for (int i = 0; i < 10; i++) {
    block = (char *) malloc(BLOCKSIZE);
    blocks.push_back(block);
    totalblocks++;
  }
}

char * DataBlockPool::getBlock() {
  char * block;
  ScopeLock lock(blocklock);
  if (blocks.size() == 0) {
    for (int i = 0; i < 10 && totalblocks < MAXBLOCKS; i++) {
      block = (char *) malloc(BLOCKSIZE);
      blocks.push_back(block);
      totalblocks++;
    }
  }
  while (blocks.size() == 0) {
    lock.unlock();
    blocksem.wait();
    lock.lock();
  }
  block = blocks.back();
  blocks.pop_back();
  return block;
}

const int DataBlockPool::blockSize() const {
  return BLOCKSIZE;
}

void DataBlockPool::returnBlock(char * block) {
  ScopeLock lock(blocklock);
  blocks.push_back(block);
  if (blocks.size() == 1) {
    blocksem.post();
  }
}
