#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "class.hpp"

bool compile_classes(const std::map<std::string, Class> &classes,
                     const std::string &file_name);

#endif
