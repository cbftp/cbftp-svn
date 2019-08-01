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

bool directoryExistsReadable(const Path &);
bool directoryExistsWritable(const Path &);
bool fileExists(const Path &);
bool fileExistsReadable(const Path &);
bool fileExistsWritable(const Path &);
bool createDirectory(const Path &);
bool createDirectory(const Path &, bool);
bool createDirectoryRecursive(const Path &);
void readFile(const Path &, BinaryData &);
void writeFile(const Path &, const BinaryData &);
SpaceInfo getSpaceInfo(const Path & path);
}

