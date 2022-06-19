#pragma once

#include <string>

class FmtString {
public:
  FmtString();
  FmtString(const char* str);
  FmtString(const std::string& str);
  FmtString(const std::basic_string<unsigned int>& str);
  FmtString& operator=(const char* str);
  FmtString& operator=(const std::string& str);
  FmtString& operator=(const std::basic_string<unsigned int>& str);
  bool operator==(const char* other) const;
  bool operator==(const std::string& other) const;
  bool operator==(const std::basic_string<unsigned int>& other) const;
  FmtString operator+(const char* rhs) const;
  FmtString operator+(const std::string& rhs) const;
  FmtString operator+(const std::basic_string<unsigned int>& rhs) const;
  FmtString operator+(const FmtString& rhs) const;
  FmtString operator+(unsigned int rhs) const;
  FmtString& operator+=(const char* rhs);
  FmtString& operator+=(const std::string& rhs);
  FmtString& operator+=(const std::basic_string<unsigned int>& rhs);
  FmtString& operator+=(unsigned int rhs);
  operator std::string() const;
  operator std::basic_string<unsigned int>() const;
  unsigned int operator[](std::size_t idx) const;
  size_t size() const;
  size_t length() const;
  size_t rawLength() const;
  bool empty() const;
  const unsigned int* data() const;
  FmtString substr(size_t pos, size_t count=1000) const;
  FmtString rawSubstr(size_t pos, size_t count=1000) const;
  unsigned int positionInRaw(unsigned int pos, bool post = true) const;
  unsigned int positionInFormatted(unsigned int rawpos) const;
private:
  void calculateFormattedLength();
  std::string rawlegacy;
  std::basic_string<unsigned int> raw;
  size_t formattedlength;
};

FmtString operator+(const char* lhs, const FmtString& rhs);
FmtString operator+(const std::string& lhs, const FmtString& rhs);
FmtString operator+(const std::basic_string<unsigned int>& lhs, const FmtString& rhs);
FmtString operator+(unsigned int lhs, const FmtString& rhs);
