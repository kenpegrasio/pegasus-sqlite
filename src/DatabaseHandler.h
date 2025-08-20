#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include <fstream>
#include "Reader.h"
#include "SQLiteSchemaParser.h"

class DatabaseHandler {
 private:
  std::ifstream database_file;

 public:
  Reader reader;
  SQLiteSchemaParser sqlite_parser;
  DatabaseHandler(std::string& database_file_path);

  int get_page_size();
  unsigned short get_number_of_tables();
  unsigned char get_btree_page_type(PageSize page_number);
};

#endif