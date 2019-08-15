#pragma once

#include "core/types.h"

class Crypto {
public:
  static void encrypt(const Core::BinaryData &, const Core::BinaryData &, Core::BinaryData &);
  static void decrypt(const Core::BinaryData &, const Core::BinaryData &, Core::BinaryData &);
  static void sha256(const Core::BinaryData &, Core::BinaryData &);
  static void base64Encode(const Core::BinaryData &, Core::BinaryData &);
  static void base64Decode(const Core::BinaryData &, Core::BinaryData &);
};
