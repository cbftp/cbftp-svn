#pragma once

#include "types.h"

namespace encoding {

enum Encoding {
  ENCODING_CP437,
  ENCODING_ISO88591
};

unsigned int cp437toUnicode(unsigned char);
Encoding guessEncoding(const binary_data &);
}
