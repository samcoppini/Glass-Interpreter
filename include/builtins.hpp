#ifndef BUILTINS_HPP
#define BUILTINS_HPP

#include "class.hpp"

#include <map>
#include <string>

class Variable;

enum class Builtin: int {
    OutputStr,
    OutputNumber
};

std::map<std::string, Class> get_builtins();
bool handle_builtin(Builtin type, std::vector<Variable> &stack);

#endif
