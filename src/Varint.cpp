#include "Varint.h"

Varint::Varint() {}

Varint::Varint(int new_bytes, unsigned long long new_value)
    : bytes(new_bytes), value(new_value) {}
