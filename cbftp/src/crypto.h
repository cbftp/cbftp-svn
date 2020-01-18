#pragma once

#include "core/types.h"

class Crypto {
public:
  static void encrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata);
  static void decrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata);
  static void sha256(const Core::BinaryData& indata, Core::BinaryData& outdata);
  static void base64Encode(const Core::BinaryData& indata, Core::BinaryData& outdata);
  static void base64Decode(const Core::BinaryData& indata, Core::BinaryData& outdata);
  static bool isMostlyASCII(const Core::BinaryData& data);
};
