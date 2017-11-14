#ifndef PARSE_HPP
#define PARSE_HPP

#include "class.hpp"

#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <utility>

std::optional<std::map<std::string, Class>>
get_classes(const std::string &filename, bool pedantic);

#endif
