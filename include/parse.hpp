#ifndef PARSE_HPP
#define PARSE_HPP

#include "class.hpp"

#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <tuple>

bool get_comment(std::ifstream &file);
std::optional<double> get_number(std::ifstream &file, char end_char);
std::optional<std::string> get_name(std::ifstream &file, bool paren_started = false);
std::optional<std::string> get_string(std::ifstream &file);
std::optional<CommandList> get_commands(std::ifstream &file);
std::optional<std::pair<std::string, CommandList>> get_func(std::ifstream &file);
std::optional<std::pair<std::string, Class>> get_class(std::ifstream &file);
std::optional<std::map<std::string, Class>> get_classes(std::ifstream &file);

#endif
