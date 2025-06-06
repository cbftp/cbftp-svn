#pragma once

#include <string>

class FmtString {
public:
  FmtString();
  FmtString(const char* str);
  FmtString(const std::string& str);
  FmtString(const std::basic_string<char32_t>& str);
  FmtString& operator=(const char* str);
  FmtString& operator=(const std::string& str);
  FmtString& operator=(const std::basic_string<char32_t>& str);
  bool operator==(const char* other) const;
  bool operator==(const std::string& other) const;
  bool operator==(const std::basic_string<char32_t>& other) const;
  bool operator==(const FmtString& other) const;
  bool operator!=(const char* other) const;
  bool operator!=(const std::string& other) const;
  bool operator!=(const std::basic_string<char32_t>& other) const;
  bool operator!=(const FmtString& other) const;
  FmtString operator+(const char* rhs) const;
  FmtString operator+(const std::string& rhs) const;
  FmtString operator+(const std::basic_string<char32_t>& rhs) const;
  FmtString operator+(const FmtString& rhs) const;
  FmtString operator+(char32_t rhs) const;
  FmtString& operator+=(const char* rhs);
  FmtString& operator+=(const std::string& rhs);
  FmtString& operator+=(const std::basic_string<char32_t>& rhs);
  FmtString& operator+=(char32_t rhs);
  operator std::string() const;
  operator std::basic_string<char32_t>() const;
  char32_t operator[](std::size_t idx) const;
  size_t size() const;
  size_t length() const;
  size_t rawLength() const;
  bool empty() const;
  const char32_t* data() const;
  FmtString substr(size_t pos, size_t count=1000) const;
  FmtString rawSubstr(size_t pos, size_t count=1000) const;
  size_t positionInRaw(size_t pos, bool post = true) const;
  size_t positionInFormatted(size_t rawpos) const;
private:
  void calculateFormattedLength();
  std::string rawlegacy;
  std::basic_string<char32_t> raw;
  size_t formattedlength;
};

FmtString operator+(const char* lhs, const FmtString& rhs);
FmtString operator+(const std::string& lhs, const FmtString& rhs);
FmtString operator+(const std::basic_string<char32_t>& lhs, const FmtString& rhs);
FmtString operator+(char32_t lhs, const FmtString& rhs);
