#pragma once

#include <string>

#include "core/types.h"

namespace encoding {

enum Encoding {
  ENCODING_CP437,
  ENCODING_CP437_DOUBLE,
  ENCODING_ISO88591,
  ENCODING_UTF8
};

char32_t cp437toUnicode(unsigned char);
std::basic_string<char32_t> cp437toUnicode(const std::string& in);
std::basic_string<char32_t> doublecp437toUnicode(const std::string& in);
std::basic_string<char32_t> toUnicode(const std::string& in);
std::basic_string<char32_t> utf8toUnicode(const std::string& in);
Encoding guessEncoding(const Core::BinaryData& data);

}
