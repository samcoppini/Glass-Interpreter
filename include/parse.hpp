#ifndef PARSE_HPP
#define PARSE_HPP

#include "class.hpp"

#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <utility>

std::optional<std::pair<std::map<std::string, Class>, std::vector<std::string>>>
get_classes(const std::string &filename, bool pedantic, bool add_builtins = true);

#endif
