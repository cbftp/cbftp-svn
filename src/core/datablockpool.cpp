#include "datablockpool.h"

#include <cstdlib>

#include "scopelock.h"

#define BLOCKSIZE 16384

DataBlockPool::DataBlockPool() : totalblocks(0) {
  ScopeLock lock(blocklock);
  allocateNewBlocks();
}

char * DataBlockPool::getBlock() {
  char * block;
  ScopeLock lock(blocklock);
  if (blocks.empty()) {
    allocateNewBlocks();
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
}

void DataBlockPool::allocateNewBlocks() {
  for (int i = 0; i < 10; i++) {
    char * block = (char *) malloc(BLOCKSIZE);
    blocks.push_back(block);
    totalblocks++;
  }
}
