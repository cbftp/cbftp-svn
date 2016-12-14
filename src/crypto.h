#pragma once

#include "core/types.h"

class Crypto {
public:
  static void encrypt(const BinaryData &, const BinaryData &, BinaryData &);
  static void decrypt(const BinaryData &, const BinaryData &, BinaryData &);
  static void decryptOld(const BinaryData &, const BinaryData &, BinaryData &);
  static void sha256(const BinaryData &, BinaryData &);
  static void base64Encode(const BinaryData &, BinaryData &);
  static void base64Decode(const BinaryData &, BinaryData &);
};
