#pragma once

#include "core/types.h"
#include "crypto.h"

#define DATAFILE "data"
#define DATAPATH ".cbftp"

namespace {

namespace DataFileHandlerMethod {

bool isMostlyASCII(const Core::BinaryData& data) {
  unsigned int asciicount = 0;
  for (unsigned int i = 0; i < data.size(); ++i) {
    if (data[i] < 128) {
      ++asciicount;
    }
  }
  return asciicount > data.size() * 0.9;
}

bool encrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata) {
  if (!isMostlyASCII(indata)) {
    return false;
  }
  Crypto::encrypt(indata, pass, outdata);
  return true;
}

bool decrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata) {
  Crypto::decrypt(indata, pass, outdata);
  return isMostlyASCII(outdata);
}

}

}
