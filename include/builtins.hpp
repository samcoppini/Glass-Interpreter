#ifndef BUILTINS_HPP
#define BUILTINS_HPP

#include "class.hpp"
#include "variable.hpp"

#include <map>
#include <string>

// An enumeration of all of the built-in functions
enum class Builtin: int {
    // Functions in the built-in I class
    InputLine, // I.l
    InputChar, // I.c
    InputEof,  // I.e

    // Functions in the built-in A class
    MathAdd,            // A.a
    MathSub,            // A.s
    MathMult,           // A.m
    MathDiv,            // A.d
    MathMod,            // A.mod
    MathFloor,          // A.f
    MathEqual,          // A.e
    MathNotEqual,       // A.ne
    MathLessThan,       // A.lt
    MathLessOrEqual,    // A.le
    MathGreaterThan,    // A.gt
    MathGreaterOrEqual, // A.ge

    // Functions in the built-in O class
    OutputStr,    // O.o
    OutputNumber, // O.on

    // Functions in the built-in S class
    StrLength,      // S.l
    StrIndex,       // S.i
    StrReplace,     // S.si
    StrConcatenate, // S.a
    StrSplit,       // S.d
    StrEqual,       // S.e
    StrNumtoChar,   // S.ns
    StrChartoNum,   // S.sn

    // Functions in the built-in V class
    VarNew,   // V.n
    VarDelete // V.d
};

ClassMap get_builtins();
void remove_builtins(ClassMap &classes);
bool handle_builtin(Builtin type, std::vector<Variable> &stack, VarMap &globals);
std::string builtin_text(Builtin type, const std::string &temp_name);

#endif
