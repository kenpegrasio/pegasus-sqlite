#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Varint {
  int bytes;
  unsigned long long value;
  Varint() {}
  Varint(int new_bytes, unsigned long long new_value)
      : bytes(new_bytes), value(new_value) {}
};

Varint read_varint(std::ifstream& database_file, unsigned short& ptr) {
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
  ptr += cnt;
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

void get_record_data(Varint& record_size, Varint& rowid, Varint& header_size,
                     std::map<std::string, int>& sqlite_schema,
                     std::ifstream& database_file, unsigned short& ptr) {
  std::vector<std::string> schema_type = {"type", "name", "tbl_name",
                                          "rootpage", "sql"};
  record_size = read_varint(database_file, ptr);
  rowid = read_varint(database_file, ptr);
  header_size = read_varint(database_file, ptr);
  int idx = 0;
  int tot = header_size.value - header_size.bytes;
  while (tot > 0) {
    Varint read = read_varint(database_file, ptr);
    sqlite_schema[schema_type[idx]] = get_size_from_serial_type(read.value);
    tot -= read.bytes;
    idx++;
  }
}

std::string get_table_name(std::ifstream& database_file,
                           std::map<std::string, int>& sqlite_schema,
                           int cell_location) {
  std::vector<std::string> types = {"type", "name", "tbl_name", "rootpage",
                                    "sql"};
  int ptr = cell_location;
  for (auto type : types) {
    if (type == "tbl_name") break;
    ptr += sqlite_schema[type];
  }
  database_file.seekg(ptr);
  std::string res = "";
  for (int i = 0; i < sqlite_schema["tbl_name"]; i++) {
    char buffer[1];
    database_file.read(buffer, 1);
    res += buffer[0];
  }
  return res;
}

std::string get_sql(std::ifstream& database_file,
                    std::map<std::string, int>& sqlite_schema,
                    int cell_location) {
  std::vector<std::string> types = {"type", "name", "tbl_name", "rootpage",
                                    "sql"};
  int ptr = cell_location;
  for (auto type : types) {
    if (type == "sql") break;
    ptr += sqlite_schema[type];
  }
  database_file.seekg(ptr);
  std::string res = "";
  for (int i = 0; i < sqlite_schema["sql"]; i++) {
    char buffer[1];
    database_file.read(buffer, 1);
    res += buffer[0];
  }
  return res;
}

int get_rootpage(std::ifstream& database_file,
                 std::map<std::string, int>& sqlite_schema, int cell_location) {
  std::vector<std::string> types = {"type", "name", "tbl_name", "rootpage",
                                    "sql"};
  int ptr = cell_location;
  for (auto type : types) {
    if (type == "rootpage") break;
    ptr += sqlite_schema[type];
  }
  database_file.seekg(ptr);
  int res = 0;
  for (int i = 0; i < sqlite_schema["rootpage"]; i++) {
    char buffer[1];
    database_file.read(buffer, 1);
    res += static_cast<int>(buffer[0]);
  }
  return res;
}

char lower_char(char x) {
  if (x >= 'A' && x <= 'Z') {
    return x - 'A' + 'a';
  }
  return x;
}

bool is_variation_of(std::string cur, std::string target) {
  if ((int)cur.length() != (int)target.length()) return false;
  for (int i = 0; i < (int)target.length(); i++) {
    if (lower_char(target[i]) != lower_char(cur[i])) {
      return false;
    }
  }
  return true;
}

void lower_keywords(std::string& command, int l, int r) {
  for (int i = l; i <= r; i++) {
    command[i] = lower_char(command[i]);
  }
}

void transform_keywords_to_lower(std::string& command) {
  for (int i = 0; i < (int)command.length(); i++) {
    if (i - 4 >= 0 && is_variation_of(command.substr(i - 4, 5), "where")) {
      lower_keywords(command, i - 4, i);
    }
    if (i - 3 >= 0 && is_variation_of(command.substr(i - 3, 4), "from")) {
      lower_keywords(command, i - 3, i);
    }
    if (i - 5 >= 0 && is_variation_of(command.substr(i - 5, 6), "select")) {
      lower_keywords(command, i - 5, i);
    }
  }
}

std::map<std::string, std::string> parse_sql(std::string& command) {
  transform_keywords_to_lower(command);
  if (command.find("where") == command.npos) {
    std::map<std::string, std::string> res;
    auto select_pos = command.find("select");
    auto from_pos = command.find("from");
    std::string select_arg = "";
    for (int i = select_pos + 7; i < from_pos - 1; i++) {
      select_arg += command[i];
    }
    std::string from_arg = "";
    for (int i = from_pos + 5; i < (int)command.length(); i++) {
      from_arg += command[i];
    }
    res["select"] = select_arg;
    res["from"] = from_arg;
    return res;
  } else {
    std::map<std::string, std::string> res;
    auto select_pos = command.find("select");
    auto from_pos = command.find("from");
    auto where_pos = command.find("where");
    std::string select_arg = "";
    for (int i = select_pos + 7; i < from_pos - 1; i++) {
      select_arg += command[i];
    }
    std::string from_arg = "";
    for (int i = from_pos + 5; i < where_pos - 1; i++) {
      from_arg += command[i];
    }
    std::string where_arg = "";
    for (int i = where_pos + 6; i < (int)command.length(); i++) {
      where_arg += command[i];
    }
    res["select"] = select_arg;
    res["from"] = from_arg;
    res["where"] = where_arg;
    return res;
  }
}

std::vector<std::string> get_columns(std::string& sql_command) {
  std::vector<std::string> res;
  bool read = true;
  std::string cur = "";
  for (int i = sql_command.find('(') + 1; i < sql_command.find(')'); i++) {
    if (read) {
      if (sql_command[i] != ' ' && sql_command[i] >= 'a' &&
          sql_command[i] <= 'z')
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

int find_number_of_rows(std::ifstream& database_file,
                        std::vector<unsigned short>& cell_locations,
                        std::string& target_table) {
  int page_size = get_page_size(database_file);
  for (auto cell_location : cell_locations) {
    unsigned short ptr = cell_location;
    Varint record_size, rowid, header_size;
    std::map<std::string, int> sqlite_schema;
    get_record_data(record_size, rowid, header_size, sqlite_schema,
                    database_file, ptr);
    std::string table_name = get_table_name(database_file, sqlite_schema, ptr);
    std::string sql_command = get_sql(database_file, sqlite_schema, ptr);
    int table_root_page = get_rootpage(database_file, sqlite_schema, ptr);

    if (table_name == target_table) {
      int ptr = (table_root_page - 1) * page_size;
      database_file.seekg(ptr + 3);
      char buffer[2];
      database_file.read(buffer, 2);
      unsigned short number_of_rows =
          (static_cast<unsigned char>(buffer[1]) |
           static_cast<unsigned char>(buffer[0]) << 8);
      return number_of_rows;
    }
  }
  return -1;
}

void get_record(std::map<std::string, std::vector<std::string>>& records,
                std::ifstream& database_file, std::vector<std::string>& columns, unsigned short cell_location) {
  unsigned short ptr = cell_location;
  database_file.seekg(ptr);
  Varint payload_size = read_varint(database_file, ptr);
  Varint rowid = read_varint(database_file, ptr);
  Varint header_size = read_varint(database_file, ptr);
  int tot = header_size.value - header_size.bytes;
  int idx = 0;
  std::map <std::string, int> records_size;
  while (tot > 0) {
    Varint record = read_varint(database_file, ptr);
    records_size[columns[idx]] = get_size_from_serial_type(record.value);
    idx++;
    tot -= record.bytes;
  }
  for (auto column : columns) {
    std::string cur = "";
    for (int i = 0; i < records_size[column]; i++) {
      char buffer[1];
      database_file.read(buffer, 1);
      cur += buffer[0];
    }
    records[column].push_back(cur);
  }
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
      Varint record_size, rowid, header_size;
      std::map<std::string, int> sqlite_schema;
      get_record_data(record_size, rowid, header_size, sqlite_schema,
                      database_file, ptr);
      std::string table_name =
          get_table_name(database_file, sqlite_schema, ptr);
      table_names.push_back(table_name);
    }
    for (auto table_name : table_names) {
      std::cout << table_name << " ";
    }
    std::cout << std::endl;
  } else {
    auto parsed_commands = parse_sql(command);
    if (parsed_commands.empty()) {
      throw std::string("No command found");
    }
    std::string target_table = parsed_commands["from"];
    std::ifstream database_file(database_file_path, std::ios::binary);
    if (get_btree_page_type(database_file, 1) != 0x0d) {
      throw std::string(
          "B-tree page type other than leaf table b-tree page is not "
          "supported!");
    }

    unsigned short page_size = get_page_size(database_file);
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

    if (is_variation_of(parsed_commands["select"], "COUNT(*)")) {
      std::cout << find_number_of_rows(database_file, cell_locations,
                                       target_table)
                << std::endl;
      return 0;
    } else if (parsed_commands.find("where") == parsed_commands.end()) {
      for (auto cell_location : cell_locations) {
        unsigned short ptr = cell_location;
        Varint record_size, rowid, header_size;
        std::map<std::string, int> sqlite_schema;
        get_record_data(record_size, rowid, header_size, sqlite_schema,
                        database_file, ptr);
        std::string table_name =
            get_table_name(database_file, sqlite_schema, ptr);
        std::string sql_command = get_sql(database_file, sqlite_schema, ptr);
        int table_root_page = get_rootpage(database_file, sqlite_schema, ptr);

        std::vector<std::string> columns = get_columns(sql_command);

        if (table_name == target_table) {
          int start = (table_root_page - 1) * page_size;
          int ptr = (table_root_page - 1) * page_size;
          database_file.seekg(ptr + 3);
          char buffer[2];
          database_file.read(buffer, 2);
          unsigned short number_of_rows =
              (static_cast<unsigned char>(buffer[1]) |
               static_cast<unsigned char>(buffer[0]) << 8);

          database_file.seekg(ptr + 8);
          std::vector<unsigned short> pointer_records;
          for (int i = 0; i < number_of_rows; i++) {
            database_file.read(buffer, 2);
            unsigned short record =
                (static_cast<unsigned char>(buffer[1]) |
                 static_cast<unsigned char>(buffer[0]) << 8);
            pointer_records.push_back(record + start);
          }
          std::map<std::string, std::vector<std::string>> records;
          for (auto pointer_record : pointer_records) {
            get_record(records, database_file, columns, pointer_record);
          }
          std::string chosen_field = parsed_commands["select"];
          for (auto val : records[chosen_field]) {
            std::cout << val << std::endl;
          }
        }
      }
    }
  }

  return 0;
}
