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

    database_file.seekg(16);  // moves the pointer to byte 16

    char buffer[2];
    database_file.read(buffer, 2);  // get byte 17 and 18

    unsigned short page_size = (static_cast<unsigned char>(buffer[1]) |
                                (static_cast<unsigned char>(buffer[0]) << 8));

    database_file.seekg(103);       // moves the pointer to byte 103
    database_file.read(buffer, 2);  // get byte 104 and 105
    unsigned short table_numbers = (static_cast<unsigned char>(buffer[1]) |
                                    static_cast<unsigned char>(buffer[0]) << 8);

    std::cout << "database page size: " << page_size << std::endl;
    std::cout << "number of tables: " << table_numbers << std::endl;
  } else if (command == ".tables") {
    std::ifstream database_file(database_file_path, std::ios::binary);
    if (!database_file) {
      std::cerr << "Failed to open the database file" << std::endl;
      return 1;
    }
    database_file.seekg(100);  // pass the database header
    char btree_flag[1];
    database_file.read(btree_flag, 1);  // get the b-tree page type
    if (static_cast<unsigned char>(btree_flag[0]) != 0x0d) {
      throw std::string(
          "B-tree page type other than leaf table b-tree page is not "
          "supported!");
    }
    // std::cout << "The B-tree page type is leaf table b-tree page" << std::endl;

    char buffer[2];
    database_file.seekg(103);       // moves the pointer to byte 103
    database_file.read(buffer, 2);  // get byte 104 and 105
    unsigned short table_numbers = (static_cast<unsigned char>(buffer[1]) |
                                    static_cast<unsigned char>(buffer[0]) << 8);
    // std::cout << "number of tables detected: " << table_numbers << std::endl;

    std::vector<unsigned short> cell_locations;
    database_file.seekg(108);
    for (int i = 0; i < table_numbers; i++) {
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
        if (read.value >= 0 && read.value <= 4) {
          sqlite_schema[schema_type[idx]] = read.value;
        } else if (read.value == 5) {
          sqlite_schema[schema_type[idx]] = 6;
        } else if (read.value == 6 || read.value == 7) {
          sqlite_schema[schema_type[idx]] = 8;
        } else if (read.value == 8 || read.value == 9) {
          sqlite_schema[schema_type[idx]] = 0;
        } else if (read.value == 10 || read.value == 11) {
          throw std::string("Serial type 10 or 11 is reserved");
        } else if (read.value >= 12 && read.value % 2 == 0) {
          sqlite_schema[schema_type[idx]] = (read.value - 12) / 2;
        } else if (read.value >= 13 && read.value % 2 == 1) {
          sqlite_schema[schema_type[idx]] = (read.value - 13) / 2;
        } else {
          throw std::string("Invalid serial type");
        }
        ptr += read.bytes;
        tot -= read.bytes;
        idx++;
      }
    //   std::cout << "SQLite Schema" << std::endl;
    //   for (auto [key, value] : sqlite_schema) {
    //     std::cout << key << ": " << value << std::endl;
    //   }
      ptr += sqlite_schema["type"] + sqlite_schema["name"];
      database_file.seekg(ptr);
      std::string table_name = "";
      for (int i = 0; i < sqlite_schema["tbl_name"]; i++) {
        char buffer[1];
        database_file.read(buffer, 1);
        table_name += buffer[0];
      }
    //   std::cout << "Table name detected: " << table_name << std::endl;
      table_names.push_back(table_name);
    }
    for (auto table_name : table_names) {
        std::cout << table_name << " ";
    }
    std::cout << std::endl;
  }

  return 0;
}
