#pragma once

class DataBlock {
private:
  char * datap;
  int datalen;
public:
  DataBlock(char *, int);
  char * data();
  int dataLength();
};
