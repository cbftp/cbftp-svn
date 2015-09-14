#pragma once

class DataBlock {
private:
  char * datap;
  int datalen;
  int offset;
public:
  DataBlock(char *, int);
  char * rawData() const;
  int rawDataLength() const;
  char * data() const;
  int dataLength() const;
  void consume(int);
};
