#include "DatabaseHandler.h"

DatabaseHandler::DatabaseHandler(std::string& database_file_path)
    : database_file(database_file_path, std::ios::binary),
      reader(database_file),
      sqlite_parser(database_file, reader) {
  if (!database_file.is_open()) {
    throw std::string("Failed to open the database file");
  }
}

int DatabaseHandler::get_page_size() {  // unsigned short cannot store 65536, so
                                        // use int
  unsigned long long ptr = 16;
  auto sz = reader.read_2_bytes(ptr);
  if (sz == 1) return 65536;
  return sz;
}

unsigned short DatabaseHandler::get_number_of_tables() {
  unsigned long long ptr = 103;
  return reader.read_2_bytes(ptr);
}

unsigned char DatabaseHandler::get_btree_page_type(PageSize page_number) {
  auto page_size = get_page_size();
  if (page_number == 1) {
    database_file.seekg(100);  // pass the database header
  } else {
    database_file.seekg((page_number - 1) * page_size);
  }
  char btree_flag[1];
  database_file.read(btree_flag, 1);  // get the b-tree page type
  return static_cast<unsigned char>(btree_flag[0]);
}