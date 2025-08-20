#ifndef TABLE_HANDER_H
#define TABLE_HANDLER_H

#include <string>

#include "DatabaseHandler.h"
#include "Types.h"

class TableHandler {
 private:
  DatabaseHandler& db_handler;
  PageSize rootpage_number;
  std::string sql_create;

  std::vector<std::string> get_columns(std::string& sql_command);
  void traverse_btree(std::vector<PointerSize>& pointer_records,
                      unsigned int cur_page);
  void get_record(std::vector<std::map<std::string, std::string>>& records,
                  std::vector<std::string>& columns, int record_location);

 public:
  TableHandler(DatabaseHandler& db, PageSize rootpage, std::string sql_command);

  unsigned short get_number_of_rows();
  std::vector<PointerSize> get_all_pointer_records();
  std::vector<std::map<std::string, std::string>> get_all_records();
};

#endif