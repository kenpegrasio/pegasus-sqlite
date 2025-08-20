#ifndef TYPES_H
#define TYPES_H

// unsigned short -> 16 bits -> 2 bytes -> 65536 - 1
// unsigned int -> 32 bits -> 4 bytes -> 2^32
// unsigned long long -> 64 bits -> 8 bytes -> 2^64

typedef unsigned int PageSize;
typedef unsigned long long PointerSize;

struct SchemaRecord {
  std::string type, name, tbl_name, sql;
  PageSize rootpage;
};

#endif