#include "filesystem.h"

#include <cstring>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <list>

#define READBLOCKSIZE 256

namespace FileSystem {

bool directoryExistsAccessible(const std::string & path, bool write) {
  int how = write ? R_OK | W_OK : R_OK;
  if (access(path.c_str(), how) == 0) {
    struct stat status;
    stat(path.c_str(), &status);
    if (status.st_mode & S_IFDIR) {
      return true;
    }
  }
  return false;
}

bool directoryExistsWritable(const std::string & path) {
  return directoryExistsAccessible(path, true);
}

bool directoryExistsReadable(const std::string & path) {
  return directoryExistsAccessible(path, false);
}

bool fileExists(const std::string & path) {
  return !access(path.c_str(), F_OK);
}

bool fileExistsReadable(const std::string & path) {
  return !access(path.c_str(), R_OK);
}

bool fileExistsWritable(const std::string & path) {
  return !access(path.c_str(), R_OK | W_OK);
}

bool createDirectory(const std::string & path) {
  return createDirectory(path, false);
}

bool createDirectory(const std::string & path, bool privatedir) {
  mode_t mode = privatedir ? 0700 : 0755;
  return mkdir(path.c_str(), mode) >= 0;
}

bool createDirectoryRecursive(std::string path) {
  std::list<std::string> pathdirs;
  if (path.length() == 0) {
    return false;
  }
  if (path[0] == '/') {
    path = path.substr(1);
  }
  size_t slashpos;
  while ((slashpos = path.find("/")) != std::string::npos) {
    pathdirs.push_back(path.substr(0, slashpos));
    path = path.substr(slashpos + 1);
  }
  if (path.length()) {
    pathdirs.push_back(path);
  }
  std::string partialpath;
  while (pathdirs.size() > 0) {
    partialpath += "/" + pathdirs.front();
    pathdirs.pop_front();
    if (!directoryExistsReadable(partialpath)) {
      if (!createDirectory(partialpath)) {
        return false;
      }
    }
  }
  return true;
}

void readFile(const std::string & path, BinaryData & rawdata) {
  std::fstream infile;
  infile.open(path.c_str(), std::ios::in | std::ios::binary);
  int gcount = 0;
  std::vector<unsigned char *> rawdatablocks;
  while (!infile.eof() && infile.good()) {
    unsigned char * rawdatablock = new unsigned char[READBLOCKSIZE];
    rawdatablocks.push_back(rawdatablock);
    infile.read((char *)rawdatablock, READBLOCKSIZE);
    gcount = infile.gcount();
  }
  infile.close();
  size_t rawdatalen = ((rawdatablocks.size() - 1) * READBLOCKSIZE) + gcount;
  size_t rawdatasize = rawdatablocks.size() * READBLOCKSIZE;
  rawdata.resize(rawdatasize);
  std::vector<unsigned char *>::iterator it;
  int count = 0;
  for (it = rawdatablocks.begin(); it != rawdatablocks.end(); it++) {
    memcpy(&rawdata[0] + (count++ * READBLOCKSIZE), *it, READBLOCKSIZE);
    delete[] *it;
  }
  rawdata.resize(rawdatalen);
}

void writeFile(const std::string & path, const BinaryData & data) {
  std::ofstream outfile;
  outfile.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  outfile.write((const char *)&data[0], data.size());
  outfile.close();
}

}
