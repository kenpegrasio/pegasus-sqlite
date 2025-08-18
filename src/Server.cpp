#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

struct Varint {
  int bytes;
  unsigned long long value;
  Varint(int new_bytes, unsigned long long new_value)
      : bytes(new_bytes), value(new_value) {}
};

Varint read_varint(std::ifstream& database_file, unsigned short ptr) {
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
  } while (static_cast<unsigned char>(byte[0]) & 0x80);
  return Varint(cnt, ans);
}

unsigned short get_page_size(std::ifstream& database_file) {
  database_file.seekg(16);  // moves the pointer to byte 16
  char buffer[2];
  database_file.read(buffer, 2);  // get byte 17 and 18
  unsigned short page_size = (static_cast<unsigned char>(buffer[1]) |
                              (static_cast<unsigned char>(buffer[0]) << 8));
  return page_size;
}

unsigned short get_number_of_tables(std::ifstream& database_file) {
  database_file.seekg(103);  // moves the pointer to byte 103
  char buffer[2];
  database_file.read(buffer, 2);  // get byte 104 and 105
  unsigned short number_of_tables =
      (static_cast<unsigned char>(buffer[1]) |
       static_cast<unsigned char>(buffer[0]) << 8);
  return number_of_tables;
}

unsigned char get_btree_page_type(std::ifstream& database_file,
                                  int page_number) {
  if (page_number == 1) {
    database_file.seekg(100);  // pass the database header
  } else {
    database_file.seekg((page_number - 1) * 4096);
  }
  char btree_flag[1];
  database_file.read(btree_flag, 1);  // get the b-tree page type
  return static_cast<unsigned char>(btree_flag[0]);
}

int get_size_from_serial_type(unsigned long long serial_type) {
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

std::string get_schema_type(std::ifstream& database_file,
                            std::map<std::string, int>& sqlite_schema,
                            std::string identifier, int cell_location) {
  std::vector<std::string> types = {"type", "name", "tbl_name", "rootpage",
                                    "sql"};
  int ptr = cell_location;
  for (auto type : types) {
    if (type == identifier) break;
    ptr += sqlite_schema[type];
  }
  database_file.seekg(ptr);
  std::string res = "";
  for (int i = 0; i < sqlite_schema[identifier]; i++) {
    char buffer[1];
    database_file.read(buffer, 1);
    res += buffer[0];
  }
  return res;
}

int main(int argc, char* argv[]) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cerr << "Logs from your program will appear here" << std::endl;

  if (argc != 3) {
    std::cerr << "Expected two arguments" << std::endl;
    return 1;
  }

  std::string database_file_path = argv[1];
  std::string command = argv[2];

  if (command == ".dbinfo") {
    std::ifstream database_file(database_file_path, std::ios::binary);
    if (!database_file) {
      std::cerr << "Failed to open the database file" << std::endl;
      return 1;
    }

    unsigned short page_size = get_page_size(database_file);
    unsigned short number_of_tables = get_number_of_tables(database_file);

    std::cout << "database page size: " << page_size << std::endl;
    std::cout << "number of tables: " << number_of_tables << std::endl;
  } else if (command == ".tables") {
    std::ifstream database_file(database_file_path, std::ios::binary);
    if (!database_file) {
      std::cerr << "Failed to open the database file" << std::endl;
      return 1;
    }

    if (get_btree_page_type(database_file, 1) != 0x0d) {
      throw std::string(
          "B-tree page type other than leaf table b-tree page is not "
          "supported!");
    }

    unsigned short number_of_tables = get_number_of_tables(database_file);

    std::vector<unsigned short> cell_locations;
    database_file.seekg(108);
    for (int i = 0; i < number_of_tables; i++) {
      char buffer[2];
      database_file.read(buffer, 2);
      unsigned short cell_ptr = (static_cast<unsigned char>(buffer[1]) |
                                 static_cast<unsigned char>(buffer[0]) << 8);
      cell_locations.push_back(cell_ptr);
    }
    std::vector<std::string> table_names;
    for (auto cell_location : cell_locations) {
      unsigned short ptr = cell_location;
      Varint record_size = read_varint(database_file, ptr);
      ptr += record_size.bytes;
      Varint rowid = read_varint(database_file, ptr);
      ptr += record_size.bytes;
      Varint header_size = read_varint(database_file, ptr);
      ptr += header_size.bytes;
      int tot = header_size.value - header_size.bytes;
      std::vector<std::string> schema_type = {"type", "name", "tbl_name",
                                              "rootpage", "sql"};
      std::map<std::string, int> sqlite_schema;
      int idx = 0;
      while (tot > 0) {
        Varint read = read_varint(database_file, ptr);
        sqlite_schema[schema_type[idx]] = get_size_from_serial_type(read.value);
        ptr += read.bytes;
        tot -= read.bytes;
        idx++;
      }
      std::string table_name = get_schema_type(database_file, sqlite_schema, "tbl_name", ptr);
      table_names.push_back(table_name);
    }
    for (auto table_name : table_names) {
      std::cout << table_name << " ";
    }
    std::cout << std::endl;
  }

  return 0;
}
