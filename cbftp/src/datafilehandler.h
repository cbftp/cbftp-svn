#pragma once

#include <string>
#include <vector>

#include "core/types.h"
#include "path.h"

class DataFileHandler {
private:
  BinaryData rawdata;
  std::vector<std::string> decryptedlines;
  std::vector<std::string> outputlines;
  Path path;
  std::string key;
  BinaryData filehash;
  bool fileexists;
  bool initialized;
public:
  DataFileHandler();
  bool readEncrypted(const std::string &);
  bool readPlain();
  void newDataFile(const std::string &);
  void writeFile();
  bool changeKey(const std::string &, const std::string &);
  bool fileExists() const;
  bool isInitialized() const;
  void clearOutputData();
  void addOutputLine(const std::string &, const std::string &);
  void getDataFor(const std::string &, std::vector<std::string> *);
};
