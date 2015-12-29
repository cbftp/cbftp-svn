#include "datablockpool.h"

#include <cstdlib>

#include "scopelock.h"

DataBlockPool::DataBlockPool() :
  totalblocks(0),
  waitingforblocks(false) {
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
  if (waitingforblocks) {
    if (blocks.size() > currentMaxNumBlocks() / 2) {
      waitingforblocks = false;
      blocksem.post();
    }
  }
}

void DataBlockPool::awaitFreeBlocks() {
  blocklock.lock();
  if (blocks.size() < currentMaxNumBlocks() / 2) {
    waitingforblocks = true;
    blocklock.unlock();
    blocksem.wait();
  }
  else {
    blocklock.unlock();
  }
}

void DataBlockPool::allocateNewBlocks() {
  for (int i = 0; i < 10; i++) {
    char * block = (char *) malloc(BLOCKSIZE);
    blocks.push_back(block);
    totalblocks++;
  }
}

unsigned int DataBlockPool::currentMaxNumBlocks() const {
  return totalblocks < MAXBLOCKS ? totalblocks : MAXBLOCKS;
}
