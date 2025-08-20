#include "SQLCommandParser.h"

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

std::vector<std::string> parse_multiple_columns(std::string columns) {
  std::vector<std::string> res;
  std::string cur = "";
  for (int i = 0; i < (int)columns.size(); i++) {
    if (columns[i] == ',') {
      res.push_back(cur);
      cur.clear();
      continue;
    }
    if (columns[i] != ' ') cur += columns[i];
  }
  if (!cur.empty()) res.push_back(cur);
  return res;
}

std::pair<std::string, std::string> parse_single_where(std::string condition) {
  if (condition.find('=') == condition.npos)
    throw std::string("Invalid WHERE condition");
  std::string key = "";
  for (int i = 0; i < condition.find('='); i++) {
    if (condition[i] == ' ') continue;
    key += condition[i];
  }
  std::string value = "";
  bool quote = false;
  for (int i = condition.find('=') + 1; i < (int)condition.size(); i++) {
    if (condition[i] == '\'') {
      quote = !quote;
      continue;
    }
    if (!quote && condition[i] == ' ') continue;
    value += condition[i];
  }
  return {key, value};
}