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
    MathAdd,
    MathSub,
    MathMult,
    MathDiv,
    MathMod,
    MathFloor,
    MathEqual,
    MathNotEqual,
    MathLessThan,
    MathLessOrEqual,
    MathGreaterThan,
    MathGreaterOrEqual,
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
void remove_builtins(std::map<std::string, Class> &classes);
bool handle_builtin(Builtin type, std::vector<Variable> &stack,
                    std::map<std::string, Variable> &globals);

#endif
