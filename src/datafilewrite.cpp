#include "datafilecat.h"

int main(int argc, char ** argv) {
  if (argc < 4) {
    std::cout << "datafilewrite: encrypts the content of a cbftp plain text data file.\n\nUsage: datafilewrite <plaintextdatafile> <crypto key> <outdatafile>" << std::endl;
    return 0;
  }
  char * path = argv[1];
  std::string key = std::string(argv[2]);
  char * outpath = argv[3];
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
  if (strstr((const char *)rawdata, std::string("DataFileHandler.readable").data()) == NULL) {
    std::cout << "Error: The input file is not a valid v data file, or is missing the DataFileHandler.readable entry." << std::endl;
    delete[] rawdata;
    return -1;
  }
  unsigned char ciphertext[rawdatalen + Crypto::blocksize()];
  int ciphertextlen;
  unsigned char keyhash[32];
  Crypto::sha256(key, keyhash);
  Crypto::encrypt(rawdata, rawdatalen, keyhash, ciphertext, &ciphertextlen);
  delete[] rawdata;
  std::ofstream outfile;
  outfile.open(outpath, std::ios::trunc);
  outfile.write((const char *)ciphertext, ciphertextlen);
  outfile.close();
  return 0;
}
