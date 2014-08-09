#include "datablock.h"

DataBlock::DataBlock(char * datap, int datalen) :
  datap(datap),
  datalen(datalen) {
}

char * DataBlock::data() const {
  return datap;
}

int DataBlock::dataLength() const {
  return datalen;
}
