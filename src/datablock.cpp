#include "datablock.h"

DataBlock::DataBlock(char * datap, int datalen) {
  this->datap = datap;
  this->datalen = datalen;
}

char * DataBlock::data() {
  return datap;
}

int DataBlock::dataLength() {
  return datalen;
}
