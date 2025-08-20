#ifndef READER_H
#define READER_H

#include <fstream>

#include "Types.h"
#include "Varint.h"

class Reader {
 private:
  std::ifstream& database_file;

 public:
  Reader(std::ifstream& file);

  Varint read_varint(PointerSize& ptr);
  unsigned char read_1_byte(PointerSize& ptr);
  unsigned short read_2_bytes(PointerSize& ptr);
  unsigned int read_4_bytes(PointerSize& ptr);
};

#endif