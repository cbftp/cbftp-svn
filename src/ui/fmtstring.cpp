#include "fmtstring.h"

FmtString::FmtString() : formattedlength(0) {
}

FmtString::FmtString(const std::string& str) : rawlegacy(str) {
  raw.resize(str.length());
  for (size_t i = 0; i < str.length(); ++i) {
    raw[i] = str[i];
  }
  calculateFormattedLength();
}

FmtString::FmtString(const char* str) : rawlegacy(str) {
  raw.resize(rawlegacy.length());
  for (size_t i = 0; i < rawlegacy.length(); ++i) {
    raw[i] = rawlegacy[i];
  }
  calculateFormattedLength();
}

FmtString::FmtString(const std::basic_string<unsigned int>& str) : raw(str) {
  rawlegacy.resize(str.length());
  for (size_t i = 0; i < str.length(); ++i) {
    rawlegacy[i] = str[i];
  }
  calculateFormattedLength();
}

FmtString& FmtString::operator=(const char* str) {
  rawlegacy = str;
  raw.resize(rawlegacy.length());
  for (size_t i = 0; i < rawlegacy.length(); ++i) {
    raw[i] = rawlegacy[i];
  }
  calculateFormattedLength();
  return *this;
}

FmtString& FmtString::operator=(const std::string& str) {
  rawlegacy = str;
  raw.resize(str.length());
  for (size_t i = 0; i < str.length(); ++i) {
    raw[i] = str[i];
  }
  calculateFormattedLength();
  return *this;
}

FmtString& FmtString::operator=(const std::basic_string<unsigned int>& str) {
  raw = str;
  calculateFormattedLength();
  return *this;
}

bool FmtString::operator==(const char* other) const {
  return rawlegacy == other;
}

bool FmtString::operator==(const std::string& other) const {
  return rawlegacy == other;
}

bool FmtString::operator==(const std::basic_string<unsigned int>& other) const {
  return raw == other;
}

FmtString FmtString::operator+(const char* rhs) const {
  return rawlegacy + rhs;
}

FmtString FmtString::operator+(const std::string& rhs) const {
  return rawlegacy + rhs;
}

FmtString FmtString::operator+(const std::basic_string<unsigned int>& rhs) const {
  return raw + rhs;
}

FmtString FmtString::operator+(const FmtString& rhs) const {
  return raw + std::basic_string<unsigned int>(rhs);
}

FmtString FmtString::operator+(unsigned int rhs) const {
  return raw + rhs;
}

FmtString& FmtString::operator+=(const char* rhs) {
  rawlegacy += rhs;
  raw.resize(rawlegacy.length());
  for (size_t i = 0; i < rawlegacy.length(); ++i) {
    raw[i] = rawlegacy[i];
  }
  calculateFormattedLength();
  return *this;
}

FmtString& FmtString::operator+=(const std::string& rhs) {
  rawlegacy += rhs;
  raw.resize(rawlegacy.length());
  for (size_t i = 0; i < rawlegacy.length(); ++i) {
    raw[i] = rawlegacy[i];
  }
  calculateFormattedLength();
  return *this;
}

FmtString& FmtString::operator+=(const std::basic_string<unsigned int>& rhs) {
  raw += rhs;
  rawlegacy.resize(raw.length());
  for (size_t i = 0; i < raw.length(); ++i) {
    rawlegacy[i] = raw[i];
  }
  calculateFormattedLength();
  return *this;
}

FmtString& FmtString::operator+=(unsigned int rhs) {
  raw+= rhs;
  rawlegacy += static_cast<char>(rhs);
  if (rhs == ')') {
    calculateFormattedLength();
  }
  else {
    ++formattedlength;
  }
  return *this;
}

FmtString::operator std::string() const {
  return rawlegacy;
}

FmtString::operator std::basic_string<unsigned int>() const {
  return raw;
}

unsigned int FmtString::operator[](std::size_t idx) const {
  return raw[idx];
}

size_t FmtString::size() const {
  return formattedlength;
}

size_t FmtString::length() const {
  return formattedlength;
}

size_t FmtString::rawLength() const {
  return raw.length();
}

void FmtString::calculateFormattedLength() {
  size_t len = raw.length();
  size_t formatsegmentslen = 0;
  for (size_t i = 0; i < len; ++i) {
    if (len - i > 3 && raw[i] == '%') {
      if (raw[i+1] == 'C' && raw[i+2] == '(') {
        if (raw[i+3] == ')') {
          formatsegmentslen += 4;
          i += 3;
          continue;
        }
        else if (len - i > 4 && raw[i+4] == ')') {
          formatsegmentslen += 5;
          i += 4;
          continue;
        }
        else if (len - i > 6 && raw[i+4] == ',' && raw[i+6] == ')') {
          formatsegmentslen += 7;
          i += 6;
          continue;
        }
      }
      else if (raw[i+1] == 'B' && raw[i+2] == '(' && raw[i+3] == ')') {
        formatsegmentslen += 4;
        i += 3;
        continue;
      }
    }
  }
  formattedlength = len - formatsegmentslen;
}

bool FmtString::empty() const {
  return !raw.length();
}

const unsigned int* FmtString::data() const {
  return raw.data();
}

FmtString FmtString::substr(size_t pos, size_t count) const {
  std::basic_string<unsigned int> out;
  size_t len = raw.length();
  std::basic_string<unsigned int> color;
  bool bold = false;
  size_t i = 0;
  for (size_t formattedpos = 0; i < len && formattedpos < pos; ++i) {
    if (len - i > 3 && raw[i] == '%') {
      if (raw[i+1] == 'C' && raw[i+2] == '(') {
        if (raw[i+3] == ')') {
          color.clear();
          i += 3;
          continue;
        }
        else if (len - i > 4 && raw[i+4] == ')') {
          color = raw.substr(i, 5);
          i += 4;
          continue;
        }
        else if (len - i > 6 && raw[i+4] == ',' && raw[i+6] == ')') {
          color = raw.substr(i, 7);
          i += 6;
          continue;
        }
      }
      else if (raw[i+1] == 'B' && raw[i+2] == '(' && raw[i+3] == ')') {
        bold ^= bold;
        i += 3;
        continue;
      }
    }
    ++formattedpos;
  }
  if (i == len) {
    return out;
  }
  if (!color.empty()) {
    out += color;
  }
  if (bold) {
    out += {'%', 'B', '(', ')'};
  }
  size_t consumestart = i;
  for (size_t consumed = 0; i < len && consumed < count; ++i) {
    if (len - i > 3 && raw[i] == '%') {
      if (raw[i+1] == 'C' && raw[i+2] == '(') {
        if (raw[i+3] == ')') {
          color.clear();
          i += 3;
          continue;
        }
        else if (len - i > 4 && raw[i+4] == ')') {
          color = raw.substr(i, 5);
          i += 4;
          continue;
        }
        else if (len - i > 6 && raw[i+4] == ',' && raw[i+6] == ')') {
          color = raw.substr(i, 7);
          i += 6;
          continue;
        }
      }
      else if (raw[i+1] == 'B' && raw[i+2] == '(' && raw[i+3] == ')') {
        bold ^= bold;
        i += 3;
        continue;
      }
    }
    ++consumed;
  }
  out += raw.substr(consumestart, i - consumestart);
  if (!color.empty()) {
    out += {'%', 'C', '(', ')'};
  }
  if (bold) {
    out += {'%', 'B', '(', ')'};
  }
  return out;
}

FmtString FmtString::rawSubstr(size_t pos, size_t count) const {
  return raw.substr(pos, count);
}

unsigned int FmtString::positionInRaw(unsigned int pos, bool post) const {
  size_t i = 0;
  size_t len = raw.length();
  for (size_t formattedpos = 0; i < len && (post || formattedpos < pos); ++i) {
    if (len - i > 3 && raw[i] == '%') {
      if (raw[i+1] == 'C' && raw[i+2] == '(') {
        if (raw[i+3] == ')') {
          i += 3;
          continue;
        }
        else if (len - i > 4 && raw[i+4] == ')') {
          i += 4;
          continue;
        }
        else if (len - i > 6 && raw[i+4] == ',' && raw[i+6] == ')') {
          i += 6;
          continue;
        }
      }
      else if (raw[i+1] == 'B' && raw[i+2] == '(' && raw[i+3] == ')') {
        i += 3;
        continue;
      }
    }
    if (formattedpos == pos) {
      break;
    }
    ++formattedpos;
  }
  return i;
}

unsigned int FmtString::positionInFormatted(unsigned int rawpos) const {
  size_t len = raw.length();
  size_t formattedpos = 0;
  for (size_t i = 0; i < rawpos; ++i) {
    if (len - i > 3 && raw[i] == '%') {
      if (raw[i+1] == 'C' && raw[i+2] == '(') {
        if (raw[i+3] == ')') {
          i += 3;
          continue;
        }
        else if (len - i > 4 && raw[i+4] == ')') {
          i += 4;
          continue;
        }
        else if (len - i > 6 && raw[i+4] == ',' && raw[i+6] == ')') {
          i += 6;
          continue;
        }
      }
      else if (raw[i+1] == 'B' && raw[i+2] == '(' && raw[i+3] == ')') {
        i += 3;
        continue;
      }
    }
    ++formattedpos;
  }
  return formattedpos;
}

FmtString operator+(const char* lhs, const FmtString& rhs) {
  return FmtString(lhs) + rhs;
}

FmtString operator+(const std::string& lhs, const FmtString& rhs) {
  return FmtString(lhs) + rhs;
}

FmtString operator+(const std::basic_string<unsigned int>& lhs, const FmtString& rhs) {
  return FmtString(lhs) + rhs;
}

FmtString operator+(unsigned int lhs, const FmtString& rhs) {
  return FmtString() + lhs + rhs;
}

