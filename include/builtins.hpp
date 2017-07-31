#ifndef BUILTINS_HPP
#define BUILTINS_HPP

#include "class.hpp"

#include <map>
#include <string>

class Variable;

enum class Builtin: int {
    InputLine,
    InputChar,
    InputEof,
    OutputStr,
    OutputNumber,
    StrLength,
    StrIndex,
    StrReplace,
    StrConcatenate,
    StrSplit,
    StrEqual,
    StrNumtoChar,
    StrChartoNum,
    VarNew,
    VarDelete
};

std::map<std::string, Class> get_builtins();
bool handle_builtin(Builtin type, std::vector<Variable> &stack);

#endif
