#include "datafilehandler.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "filesystem.h"
#include "datafilehandlermethod.h"

DataFileHandler::DataFileHandler() {
  std::string datadirpath = std::string(getenv("HOME")) + "/" + DATAPATH;
  path = datadirpath + "/" + DATAFILE;
  char * specialdatapath = getenv("CBFTP_DATA_PATH");
  if (specialdatapath != NULL) {
    path = std::string(specialdatapath);
  }
  else if (FileSystem::directoryExistsReadable(datadirpath)) {
    if (!FileSystem::directoryExistsWritable(datadirpath)) {
      perror(std::string("Error: no write access to " + datadirpath).c_str());
      exit(1);
    }
  }
  else {
    if (!FileSystem::createDirectory(datadirpath, true)) {
      perror(std::string("Error: could not create " + datadirpath).c_str());
      exit(1);
    }
  }
  fileexists = false;
  initialized = false;
  if (!FileSystem::fileExists(path)) {
    return;
  }
  if (!FileSystem::fileExistsWritable(path)) {
    perror(std::string("There was an error accessing " + path).c_str());
    exit(1);
  }
  fileexists = true;
  FileSystem::readFile(path, rawdata);
}

bool DataFileHandler::readEncrypted(const std::string & key) {
  if (!fileexists || initialized) {
    return false;
  }
  this->key = key;
  BinaryData keydata(key.begin(), key.end());
  BinaryData decryptedtext;
  if (!DataFileHandlerMethod::decrypt(rawdata, keydata, decryptedtext)) {
    return false;
  }
  int lastbreakpos = 0;
  for (unsigned int currentpos = 0; currentpos <= decryptedtext.size(); currentpos++) {
    if (decryptedtext[currentpos] == '\n' || currentpos == decryptedtext.size()) {
      decryptedlines.push_back(std::string((const char *)&decryptedtext[lastbreakpos], currentpos - lastbreakpos));
      lastbreakpos = currentpos + 1;
    }
  }
  Crypto::sha256(decryptedtext, filehash);
  initialized = true;
  return true;
}

bool DataFileHandler::readPlain() {
  return false;
}

void DataFileHandler::newDataFile(const std::string & key) {
  if (!initialized) {
    this->key = key;
    initialized = true;
  }
}

void DataFileHandler::writeFile() {
  std::string fileoutput = "";
  std::vector<std::string>::iterator it;
  for (it = outputlines.begin(); it != outputlines.end(); it++) {
    fileoutput.append(*it + "\n");
  }
  BinaryData datahash;
  BinaryData fileoutputdata(fileoutput.begin(),
                            fileoutput.end() - (fileoutput.size() ? 1 : 0));
  Crypto::sha256(fileoutputdata, datahash);
  if (datahash == filehash) {
    return;
  }
  filehash = datahash;
  BinaryData ciphertext;
  BinaryData keydata(key.begin(), key.end());
  DataFileHandlerMethod::encrypt(fileoutputdata, keydata, ciphertext);
  FileSystem::writeFile(path, ciphertext);
}

bool DataFileHandler::changeKey(const std::string & key, const std::string & newkey) {
  if (this->key != key) {
    return false;
  }
  this->key = newkey;
  filehash.clear();
  writeFile();
  return true;
}

bool DataFileHandler::fileExists() const {
  return fileexists;
}

bool DataFileHandler::isInitialized() const {
  return initialized;
}

void DataFileHandler::clearOutputData() {
  outputlines.clear();
}

void DataFileHandler::addOutputLine(const std::string & owner, const std::string & line) {
  outputlines.push_back(owner + "." + line);
}

void DataFileHandler::getDataFor(const std::string & owner, std::vector<std::string> * matches) {
  matches->clear();
  std::vector<std::string>::iterator it;
  int len = owner.length() + 1;
  for (it = decryptedlines.begin(); it != decryptedlines.end(); it++) {
    if (it->compare(0, len, owner + ".") == 0) {
      matches->push_back(it->substr(len));
    }
  }
}
