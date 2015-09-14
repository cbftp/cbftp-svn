#pragma once

#include <string>
#include <vector>

#define READBLOCKSIZE 256

class DataFileHandler {
private:
  std::vector<unsigned char *> rawdatablocks;
  unsigned char * rawdata;
  int rawdatalen;
  std::vector<std::string> decryptedlines;
  std::vector<std::string> outputlines;
  std::string path;
  std::string key;
  bool fileexists;
  bool initialized;
public:
  DataFileHandler(std::string);
  bool tryDecrypt(std::string);
  void newDataFile(std::string);
  void writeFile();
  bool changeKey(std::string, std::string);
  bool fileExists() const;
  bool isInitialized() const;
  void clearOutputData();
  void addOutputLine(std::string, std::string);
  void getDataFor(std::string, std::vector<std::string> *);
};
