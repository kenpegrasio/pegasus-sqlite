#ifndef VARINT_H
#define VARINT_H

struct Varint {
  int bytes;
  unsigned long long value;

  Varint();
  Varint(int new_bytes, unsigned long long new_value);
};

#endif