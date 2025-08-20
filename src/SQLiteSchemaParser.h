#ifndef SQLITE_PARSER_H
#define SQLITE_PARSER_H

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "Reader.h"
#include "utils.h"

class SQLiteSchemaParser {
 private:
  std::ifstream& database_file;
  Reader& reader;

  std::map<std::string, unsigned long long> get_record_size(PointerSize& ptr);
  std::vector<unsigned short> get_table_pointers();
  SchemaRecord get_single_record_data(PointerSize ptr);

 public:
  SQLiteSchemaParser(std::ifstream& file, Reader& new_reader);

  unsigned short get_number_of_tables();
  std::vector<SchemaRecord> get_all_record_data();
};

#endif