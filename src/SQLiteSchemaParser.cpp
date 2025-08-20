#include "SQLiteSchemaParser.h"

SQLiteSchemaParser::SQLiteSchemaParser(std::ifstream& file, Reader& new_reader)
    : database_file(file), reader(new_reader) {}

std::map<std::string, unsigned long long> SQLiteSchemaParser::get_record_size(
    PointerSize& ptr) {
  std::vector<std::string> schema_type = {"type", "name", "tbl_name",
                                          "rootpage", "sql"};
  Varint record_size = reader.read_varint(ptr);
  Varint rowid = reader.read_varint(ptr);
  Varint header_size = reader.read_varint(ptr);
  int idx = 0;
  int tot = header_size.value - header_size.bytes;
  std::map<std::string, unsigned long long> schema_size;
  while (tot > 0) {
    Varint read = reader.read_varint(ptr);
    schema_size[schema_type[idx]] = get_size_from_serial_type(read.value);
    tot -= read.bytes;
    idx++;
  }
  return schema_size;
}

unsigned short SQLiteSchemaParser::get_number_of_tables() {
  unsigned long long ptr = 103;
  return reader.read_2_bytes(ptr);
}

std::vector<unsigned short> SQLiteSchemaParser::get_table_pointers() {
  unsigned long long ptr = 103;
  auto number_of_tables = reader.read_2_bytes(ptr);
  ptr += 3;
  std::vector<unsigned short> table_pointers;
  for (int i = 0; i < number_of_tables; i++) {
    table_pointers.push_back(reader.read_2_bytes(ptr));
  }
  return table_pointers;
}

SchemaRecord SQLiteSchemaParser::get_single_record_data(PointerSize ptr) {
  SchemaRecord ans;
  auto record_size = get_record_size(ptr);
  std::vector<std::string> types = {"type", "name", "tbl_name", "rootpage",
                                    "sql"};
  for (auto type : types) {
    if (type != "rootpage") {
      std::string res = "";
      for (int i = 0; i < record_size[type]; i++) {
        char buffer[1];
        database_file.read(buffer, 1);
        res += buffer[0];
      }
      if (type == "type")
        ans.type = res;
      else if (type == "name")
        ans.name = res;
      else if (type == "tbl_name")
        ans.tbl_name = res;
      else if (type == "sql")
        ans.sql = res;
    } else {
      PageSize res = 0;
      for (int i = 0; i < record_size[type]; i++) {
        char buffer[1];
        database_file.read(buffer, 1);
        res += static_cast<int>(buffer[0]);
      }
      ans.rootpage = res;
    }
  }
  return ans;
}

std::vector<SchemaRecord> SQLiteSchemaParser::get_all_record_data() {
  std::vector<SchemaRecord> res;
  auto table_pointers = get_table_pointers();
  for (auto table_pointer : table_pointers) {
    res.push_back(get_single_record_data(table_pointer));
  }
  return res;
}