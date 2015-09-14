#include "datafilehandler.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>

#include "crypto.h"

DataFileHandler::DataFileHandler(std::string path) {
  this->path = path;
  fileexists = false;
  initialized = false;
  if (access(path.c_str(), F_OK) < 0) {
    return;
  }
  if (access(path.c_str(), R_OK | W_OK) < 0) {
    perror(std::string("There was an error accessing " + path).c_str());
    exit(1);
  }
  fileexists = true;
  std::fstream infile;
  infile.open(path.c_str());
  int gcount = 0;
  while (!infile.eof() && infile.good()) {
    unsigned char * rawdatablock = new unsigned char[READBLOCKSIZE];
    rawdatablocks.push_back(rawdatablock);
    infile.read((char *)rawdatablock, READBLOCKSIZE);
    gcount = infile.gcount();
  }
  infile.close();
  rawdatalen = ((rawdatablocks.size() - 1) * READBLOCKSIZE) + gcount;
  int rawdatasize = rawdatablocks.size() * READBLOCKSIZE;
  rawdata = new unsigned char[rawdatasize];
  std::vector<unsigned char *>::iterator it;
  int count = 0;
  for (it = rawdatablocks.begin(); it != rawdatablocks.end(); it++) {
    memcpy(rawdata + (count++ * READBLOCKSIZE), *it, READBLOCKSIZE);
    delete *it;
  }
}

bool DataFileHandler::tryDecrypt(std::string key) {
  if (!fileexists || initialized) {
    return false;
  }
  this->key = key;
  unsigned char keyhash[32];
  Crypto::sha256(key, keyhash);
  unsigned char decryptedtext[rawdatalen + Crypto::blocksize()];
  int decryptedlen;
  Crypto::decrypt(rawdata, rawdatalen, keyhash, decryptedtext, &decryptedlen);
  decryptedtext[decryptedlen] = '\0';
  if (strstr((const char *)decryptedtext, std::string("DataFileHandler.readable").data()) == NULL) {
    // backwards compatibility begin
    while (key.length() < 32) {
      key.append("0");
    }
    Crypto::decrypt(rawdata, rawdatalen, (unsigned char *)key.data(), decryptedtext, &decryptedlen);
    decryptedtext[decryptedlen] = '\0';
    if (strstr((const char *)decryptedtext, std::string("DataFileHandler.readable").data()) == NULL)
    // backwards compatibility end
    return false;
  }
  delete rawdata;
  int lastbreakpos = 0;
  for (int currentpos = 0; currentpos <= decryptedlen; currentpos++) {
    if (decryptedtext[currentpos] == '\n' || currentpos == decryptedlen) {
      decryptedlines.push_back(std::string((const char *)(decryptedtext + lastbreakpos), currentpos - lastbreakpos));
      lastbreakpos = currentpos + 1;
    }
  }
  initialized = true;
  return true;
}

void DataFileHandler::newDataFile(std::string key) {
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
  fileoutput.append("DataFileHandler.readable");
  int plaintextlen = fileoutput.length();
  unsigned char plaintext[plaintextlen];
  unsigned char ciphertext[plaintextlen + Crypto::blocksize()];
  memcpy(plaintext, fileoutput.data(), plaintextlen);
  int ciphertextlen;
  unsigned char keyhash[32];
  Crypto::sha256(key, keyhash);
  Crypto::encrypt(plaintext, plaintextlen, keyhash, ciphertext, &ciphertextlen);
  std::ofstream outfile;
  outfile.open(path.c_str(), std::ios::trunc);
  outfile.write((const char *)ciphertext, ciphertextlen);
  outfile.close();
}

bool DataFileHandler::changeKey(std::string key, std::string newkey) {
  if (this->key != key) {
    return false;
  }
  this->key = newkey;
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

void DataFileHandler::addOutputLine(std::string owner, std::string line) {
  outputlines.push_back(owner.append(".").append(line));
}

void DataFileHandler::getDataFor(std::string owner, std::vector<std::string> * matches) {
  std::vector<std::string>::iterator it;
  owner.append(".");
  int len = owner.length();
  for (it = decryptedlines.begin(); it != decryptedlines.end(); it++) {
    if (it->compare(0, len, owner) == 0) {
      matches->push_back(it->substr(len));
    }
  }
}
