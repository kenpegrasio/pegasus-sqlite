#include "Reader.h"

Reader::Reader(std::ifstream& file) : database_file(file) {}

Varint Reader::read_varint(PointerSize& ptr) {
  database_file.seekg(ptr);
  unsigned long long ans = 0;
  int cnt = 0;
  char byte[1];
  do {
    database_file.read(byte, 1);
    cnt++;
    if (cnt == 9) {
      ans <<= 8;
      ans += static_cast<unsigned int>(byte[0]);
      break;
    } else {
      ans <<= 7;
      ans += static_cast<unsigned int>(byte[0]) & 0x7F;
    }
  } while (static_cast<unsigned int>(byte[0]) & 0x80);
  ptr += cnt;
  return Varint(cnt, ans);
}

unsigned char Reader::read_1_byte(PointerSize& ptr) {
  database_file.seekg(ptr);
  char buffer[1];
  database_file.read(buffer, 1);
  ptr++;
  return static_cast<unsigned char>(buffer[0]);
}

unsigned short Reader::read_2_bytes(PointerSize& ptr) {
  database_file.seekg(ptr);
  char buffer[2];
  database_file.read(buffer, 2);
  unsigned short value = (static_cast<unsigned char>(buffer[1]) |
                          static_cast<unsigned char>(buffer[0]) << 8);
  ptr += 2;
  return value;
}

unsigned int Reader::read_4_bytes(PointerSize& ptr) {
  database_file.seekg(ptr);
  char buffer[4];
  database_file.read(buffer, 4);
  unsigned int value = (static_cast<unsigned char>(buffer[3]) |
                        static_cast<unsigned char>(buffer[2]) << 8 |
                        static_cast<unsigned char>(buffer[1]) << 16 |
                        static_cast<unsigned char>(buffer[0]) << 24);
  ptr += 4;
  return value;
}
