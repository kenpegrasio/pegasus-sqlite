#ifndef SQL_COMMAND_PARSER_H
#define SQL_COMMAND_PARSER_H

#include <map>
#include <string>
#include <vector>

char lower_char(char x);
bool is_variation_of(std::string cur, std::string target);
void lower_keywords(std::string& command, int l, int r);
void transform_keywords_to_lower(std::string& command);
std::map<std::string, std::string> parse_sql(std::string& command);
std::vector<std::string> parse_multiple_columns(std::string columns);
std::pair<std::string, std::string> parse_single_where(std::string condition);

#endif