#pragma once

#include <string>

#include "core/types.h"

namespace FileSystem {

bool directoryExistsReadable(const std::string &);
bool directoryExistsWritable(const std::string &);
bool fileExists(const std::string &);
bool fileExistsReadable(const std::string &);
bool fileExistsWritable(const std::string &);
bool createDirectory(const std::string &);
bool createDirectory(const std::string &, bool);
bool createDirectoryRecursive(std::string);
void readFile(const std::string &, BinaryData &);
void writeFile(const std::string &, const BinaryData &);

}

