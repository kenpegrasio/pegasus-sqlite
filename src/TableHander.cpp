#include "TableHander.h"

TableHandler::TableHandler(DatabaseHandler& db, PageSize rootpage,
                           std::string sql_command)
    : db_handler(db), rootpage_number(rootpage), sql_create(sql_command) {}

std::vector<std::string> TableHandler::get_columns(std::string& sql_command) {
  std::vector<std::string> res;
  bool read = true;
  std::string cur = "";
  for (int i = sql_command.find('(') + 1; i < sql_command.find(')'); i++) {
    if (read) {
      if (sql_command[i] == '\n' || sql_command[i] == '\t') continue;
      if (sql_command[i] != ' ')
        cur += sql_command[i];
      else if (sql_command[i] == ' ') {
        if (cur.empty()) continue;
        read = false;
        if (!cur.empty()) res.push_back(cur);
        cur.clear();
      }
    }
    if (sql_command[i] == ',') {
      read = true;
    }
  }
  return res;
}

void TableHandler::traverse_btree(std::vector<PointerSize>& pointer_records,
                                  unsigned int cur_page) {
  auto page_size = db_handler.get_page_size();
  auto page_type = db_handler.get_btree_page_type(cur_page);
  if (page_type == 0x05) {
    PointerSize start = (cur_page - 1) * page_size;
    PointerSize ptr = (cur_page - 1) * page_size;
    ptr += 3;
    unsigned short number_of_cells = db_handler.reader.read_2_bytes(ptr);
    ptr += 3;
    PointerSize right_most_pointer = db_handler.reader.read_4_bytes(ptr);
    std::vector<PointerSize> cell_locations;
    for (int i = 0; i < number_of_cells; i++) {
      auto cell_location = db_handler.reader.read_2_bytes(ptr);
      cell_locations.push_back(cell_location + start);
    }
    for (auto cell_location : cell_locations) {
      PointerSize loc = cell_location;
      auto left_child_pointer = db_handler.reader.read_4_bytes(loc);
      traverse_btree(pointer_records, left_child_pointer);
    }
    traverse_btree(pointer_records, right_most_pointer);
  } else if (page_type == 0x0d) {
    PointerSize ptr = (cur_page - 1) * page_size;
    PointerSize start = (cur_page - 1) * page_size;
    ptr += 3;
    unsigned short number_of_rows = db_handler.reader.read_2_bytes(ptr);
    ptr += 3;
    for (int i = 0; i < number_of_rows; i++) {
      unsigned short record = db_handler.reader.read_2_bytes(ptr);
      pointer_records.push_back(record + start);
    }
  } else {
    throw std::string("Index B-Tree is not supported");
  }
}

void TableHandler::get_record(
    std::vector<std::map<std::string, std::string>>& records,
    std::vector<std::string>& columns, int record_location) {
  unsigned long long ptr = record_location;
  Varint payload_size = db_handler.reader.read_varint(ptr);
  Varint rowid = db_handler.reader.read_varint(ptr);
  Varint header_size = db_handler.reader.read_varint(ptr);
  int tot = header_size.value - header_size.bytes;
  int idx = 0;
  std::map<std::string, int> records_size;
  while (tot > 0) {
    Varint record = db_handler.reader.read_varint(ptr);
    records_size[columns[idx]] = get_size_from_serial_type(record.value);
    idx++;
    tot -= record.bytes;
  }
  std::map<std::string, std::string> entry;
  for (auto column : columns) {
    if (column == "id") {
      entry[column] = std::to_string(rowid.value);
      continue;
    }
    std::string cur = "";
    for (int i = 0; i < records_size[column]; i++) {
      cur += db_handler.reader.read_1_byte(ptr);
    }
    entry[column] = cur;
  }
  records.push_back(entry);
}

unsigned short TableHandler::get_number_of_rows() {
  auto page_size = db_handler.get_page_size();
  PointerSize ptr = (rootpage_number - 1) * page_size + 3;
  return db_handler.reader.read_2_bytes(ptr);
}

std::vector<PointerSize> TableHandler::get_all_pointer_records() {
  std::vector<PointerSize> pointer_records;
  traverse_btree(pointer_records, rootpage_number);
  return pointer_records;
}

std::vector<std::map<std::string, std::string>>
TableHandler::get_all_records() {
  auto pointer_records = get_all_pointer_records();
  auto columns = get_columns(sql_create);
  std::vector<std::map<std::string, std::string>> records;
  for (auto pointer_record : pointer_records) {
    get_record(records, columns, pointer_record);
  }
  return records;
}