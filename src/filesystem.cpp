#include "filesystem.h"

#include <cstring>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <list>

#include "path.h"

#define READBLOCKSIZE 256

namespace FileSystem {

bool directoryExistsAccessible(const Path & path, bool write) {
  int how = write ? R_OK | W_OK : R_OK;
  if (access(path.toString().c_str(), how) == 0) {
    struct stat status;
    stat(path.toString().c_str(), &status);
    if (status.st_mode & S_IFDIR) {
      return true;
    }
  }
  return false;
}

bool directoryExistsWritable(const Path & path) {
  return directoryExistsAccessible(path, true);
}

bool directoryExistsReadable(const Path & path) {
  return directoryExistsAccessible(path, false);
}

bool fileExists(const Path & path) {
  return !access(path.toString().c_str(), F_OK);
}

bool fileExistsReadable(const Path & path) {
  return !access(path.toString().c_str(), R_OK);
}

bool fileExistsWritable(const Path & path) {
  return !access(path.toString().c_str(), R_OK | W_OK);
}

bool createDirectory(const Path & path) {
  return createDirectory(path, false);
}

bool createDirectory(const Path & path, bool privatedir) {
  mode_t mode = privatedir ? 0700 : 0755;
  return mkdir(path.toString().c_str(), mode) >= 0;
}

bool createDirectoryRecursive(const Path & path) {
  std::list<std::string> pathdirs = path.split();
  Path partialpath;
  while (pathdirs.size() > 0) {
    partialpath = partialpath / pathdirs.front();
    pathdirs.pop_front();
    if (!directoryExistsReadable(partialpath)) {
      if (!createDirectory(partialpath)) {
        return false;
      }
    }
  }
  return true;
}

void readFile(const Path & path, BinaryData & rawdata) {
  std::fstream infile;
  infile.open(path.toString().c_str(), std::ios::in | std::ios::binary);
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

void writeFile(const Path & path, const BinaryData & data) {
  std::ofstream outfile;
  outfile.open(path.toString().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  outfile.write((const char *)&data[0], data.size());
  outfile.close();
}

}
