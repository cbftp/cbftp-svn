#pragma once

#include "core/types.h"
#include "crypto.h"

#define DATAFILE "data"
#define DATAPATH ".cbftp"

namespace {

namespace DataFileHandlerMethod {

bool isMostlyASCII(const BinaryData & data) {
  unsigned int asciicount = 0;
  for (unsigned int i = 0; i < data.size(); ++i) {
    if (data[i] < 128) {
      ++asciicount;
    }
  }
  return asciicount > data.size() * 0.9;
}

bool encrypt(const BinaryData & indata, const BinaryData & pass, BinaryData & outdata) {
  if (!isMostlyASCII(indata)) {
    return false;
  }
  Crypto::encrypt(indata, pass, outdata);
  return true;
}

bool decrypt(const BinaryData & indata, const BinaryData & pass, BinaryData & outdata) {
  Crypto::decrypt(indata, pass, outdata);
  return isMostlyASCII(outdata);
}

}

}
