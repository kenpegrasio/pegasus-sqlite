#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "DatabaseHandler.h"
#include "Reader.h"
#include "SQLCommandParser.h"
#include "SQLiteSchemaParser.h"
#include "TableHander.h"
#include "Types.h"
#include "Varint.h"

void display_entries(TableHandler& table_handler,
                     std::map<std::string, std::string>& parsed_commands) {
  auto entries = table_handler.get_all_records();
  auto chosen_field = parse_multiple_columns(parsed_commands["select"]);
  for (auto entry : entries) {
    for (int field = 0; field < (int)chosen_field.size(); field++) {
      std::cout << entry[chosen_field[field]];
      if (field == (int)chosen_field.size() - 1) {
        std::cout << std::endl;
      } else {
        std::cout << "|";
      }
    }
  }
}

void display_entries(TableHandler& table_handler,
                     std::map<std::string, std::string>& parsed_commands,
                     std::pair<std::string, std::string> condition) {
  auto entries = table_handler.get_all_records();
  auto chosen_field = parse_multiple_columns(parsed_commands["select"]);
  for (auto entry : entries) {
    for (int field = 0; field < (int)chosen_field.size(); field++) {
      if (entry[condition.first] != condition.second) continue;
      std::cout << entry[chosen_field[field]];
      if (field == (int)chosen_field.size() - 1) {
        std::cout << std::endl;
      } else {
        std::cout << "|";
      }
    }
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

  DatabaseHandler db_handler(database_file_path);

  if (command == ".dbinfo") {
    unsigned short page_size = db_handler.get_page_size();
    unsigned short number_of_tables = db_handler.get_number_of_tables();

    std::cout << "database page size: " << page_size << std::endl;
    std::cout << "number of tables: " << number_of_tables << std::endl;
  } else if (command == ".tables") {
    if (db_handler.get_btree_page_type(1) != 0x0d) {
      throw std::string(
          "B-tree page type other than leaf table b-tree page is not "
          "supported!");
    }

    auto records = db_handler.sqlite_parser.get_all_record_data();
    std::vector<std::string> table_names;
    for (auto record : records) {
      table_names.push_back(record.tbl_name);
    }
    for (auto table_name : table_names) {
      std::cout << table_name << " ";
    }
    std::cout << std::endl;
  } else {
    try {
      auto parsed_commands = parse_sql(command);
      if (parsed_commands.empty()) {
        throw std::string("No command found");
      }
      std::string target_table = parsed_commands["from"];
      if (db_handler.get_btree_page_type(1) != 0x0d) {
        throw std::string(
            "B-tree page type other than leaf table b-tree page is not "
            "supported!");
      }

      unsigned short page_size = db_handler.get_page_size();
      auto records = db_handler.sqlite_parser.get_all_record_data();

      if (is_variation_of(parsed_commands["select"], "COUNT(*)")) {
        for (auto record : records) {
          auto table_name = record.tbl_name;
          auto table_root_page = record.rootpage;
          auto sql_command = record.sql;
          TableHandler table_handler(db_handler, table_root_page, sql_command);
          if (table_name == target_table) {
            auto entries = table_handler.get_all_records();
            std::cout << entries.size() << std::endl;
          }
        }
      } else {
        for (auto record : records) {
          auto table_name = record.tbl_name;
          auto sql_command = record.sql;
          auto table_root_page = record.rootpage;

          if (table_name != target_table) continue;
          TableHandler table_handler(db_handler, table_root_page, sql_command);

          if (parsed_commands.find("where") == parsed_commands.end()) {
            display_entries(table_handler, parsed_commands);
          } else {
            auto condition = parse_single_where(parsed_commands["where"]);
            display_entries(table_handler, parsed_commands, condition);
          }
        }
      }
    } catch (std::string& error) {
      std::cout << error << std::endl;
    }
  }

  return 0;
}