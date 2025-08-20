#include "utils.h"

unsigned long long get_size_from_serial_type(unsigned long long serial_type) {
  if (serial_type >= 0 && serial_type <= 4) {
    return serial_type;
  } else if (serial_type == 5) {
    return 6;
  } else if (serial_type == 6 || serial_type == 7) {
    return 8;
  } else if (serial_type == 8 || serial_type == 9) {
    return 0;
  } else if (serial_type == 10 || serial_type == 11) {
    throw std::string("Serial type 10 or 11 is reserved");
  } else if (serial_type >= 12 && serial_type % 2 == 0) {
    return (serial_type - 12) / 2;
  } else if (serial_type >= 13 && serial_type % 2 == 1) {
    return (serial_type - 13) / 2;
  } else {
    throw std::string("Invalid serial type");
  }
}