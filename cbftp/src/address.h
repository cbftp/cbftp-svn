#pragma once

#include <string>

#include "core/types.h"

struct Address {
  Address();
  std::string toString() const;
  bool operator==(const Address& other) const;
  Core::AddressFamily addrfam;
  bool brackets;
  std::string host;
  int port;
};
