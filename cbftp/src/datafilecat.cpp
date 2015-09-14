#include "datafilecat.h"

int main(int argc, char ** argv) {
  if (argc < 3) {
    std::cout << "datafilecat: decrypts and prints the decrypted content of a cbftp data file.\n\nUsage: datafilecat <file> <crypto key>" << std::endl;
    return 0;
  }
  char * path = argv[1];
  std::string key = std::string(argv[2]);
  if (access(path, F_OK) < 0) {
    std::cout << "Error: The input file does not exist." << std::endl;
    return -1;
  }
  if (access(path, R_OK) < 0) {
    std::cout << "Error: Could not read the input file." << std::endl;
    return -1;
  }
  std::fstream infile;
  std::vector<unsigned char *> rawdatablocks;
  infile.open(path);
  int total = 0;
  int gcount;
  while (!infile.eof() && infile.good()) {
    unsigned char * rawdatablock = new unsigned char[READBLOCKSIZE];
    rawdatablocks.push_back(rawdatablock);
    infile.read((char *)rawdatablock, READBLOCKSIZE);
    gcount = infile.gcount();
  }
  infile.close();
  int rawdatalen = ((rawdatablocks.size() - 1) * READBLOCKSIZE) + gcount;
  int rawdatasize = rawdatablocks.size() * READBLOCKSIZE;
  unsigned char * rawdata = new unsigned char[rawdatasize];
  std::vector<unsigned char *>::iterator it;
  int count = 0;
  for (it = rawdatablocks.begin(); it != rawdatablocks.end(); it++) {
    memcpy(rawdata + (count++ * READBLOCKSIZE), *it, READBLOCKSIZE);
    delete *it;
  }
  unsigned char decryptedtext[rawdatalen + Crypto::blocksize()];
  int decryptedlen;
  unsigned char keyhash[32];
  Crypto::sha256(key, keyhash);
  Crypto::decrypt(rawdata, rawdatalen, keyhash, decryptedtext, &decryptedlen);
  decryptedtext[decryptedlen] = '\0';
  delete[] rawdata;
  if (strstr((const char *)decryptedtext, std::string("DataFileHandler.readable").data()) == NULL) {
    std::cout << "Error: Either the key is wrong, or the indata file is not a valid cbftp data file." << std::endl;
    return -1;
  }
  std::cout << decryptedtext << std::endl;
  return 0;
}
