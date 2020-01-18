#pragma once

#include "core/types.h"
#include "crypto.h"

#define DATAFILE "data"
#define DATAPATH ".cbftp"

namespace {

namespace DataFileHandlerMethod {

bool encrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata) {
  if (!Crypto::isMostlyASCII(indata)) {
    return false;
  }
  Crypto::encrypt(indata, pass, outdata);
  return true;
}

bool decrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata) {
  Crypto::decrypt(indata, pass, outdata);
  return Crypto::isMostlyASCII(outdata);
}

}

}
