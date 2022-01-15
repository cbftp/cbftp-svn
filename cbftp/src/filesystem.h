#pragma once

#include <string>

#include "core/types.h"
#include "path.h"

namespace FileSystem {

struct SpaceInfo {
  unsigned long long int size;
  unsigned long long int used;
  unsigned long long int avail;
};

struct Result {
  Result();
  Result(bool success, const std::string& error = "");
  bool success;
  std::string error;
};

bool directoryExists(const Path& path);
bool directoryExistsReadable(const Path& path);
bool directoryExistsWritable(const Path& path);
bool fileExists(const Path& path);
bool fileExistsReadable(const Path& path);
bool fileExistsWritable(const Path& path);
Result createDirectory(const Path& path, bool privatedir = false);
Result createDirectoryRecursive(const Path& path);
Result readFile(const Path& path, Core::BinaryData& rawdata);
Result writeFile(const Path& path, const Core::BinaryData& data);
Result move(const Path& src, const Path& dst);
SpaceInfo getSpaceInfo(const Path & path);
}

