#ifndef PARSE_HPP
#define PARSE_HPP

#include "class.hpp"

#include <fstream>
#include <map>
#include <optional>
#include <string>

std::optional<std::string> get_name(std::ifstream &file);
std::optional<std::map<std::string, Class>> get_classes(std::ifstream &file);

#endif
