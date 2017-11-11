#include "builtins.hpp"
#include "class.hpp"
#include "variable.hpp"

#include <cctype>
#include <cmath>
#include <iostream>
#include <optional>

const std::map<Builtin, std::pair<std::string, std::string>> BUILTIN_INFO = {
    {Builtin::InputLine,          {"I", "l"}},
    {Builtin::InputChar,          {"I", "c"}},
    {Builtin::InputEof,           {"I", "e"}},
    {Builtin::MathAdd,            {"A", "a"}},
    {Builtin::MathSub,            {"A", "s"}},
    {Builtin::MathMult,           {"A", "m"}},
    {Builtin::MathDiv,            {"A", "d"}},
    {Builtin::MathMod,            {"A", "mod"}},
    {Builtin::MathFloor,          {"A", "f"}},
    {Builtin::MathEqual,          {"A", "e"}},
    {Builtin::MathNotEqual,       {"A", "ne"}},
    {Builtin::MathLessThan,       {"A", "lt"}},
    {Builtin::MathLessOrEqual,    {"A", "le"}},
    {Builtin::MathGreaterThan,    {"A", "gt"}},
    {Builtin::MathGreaterOrEqual, {"A", "ge"}},
    {Builtin::OutputStr,          {"O", "o"}},
    {Builtin::OutputNumber,       {"O", "on"}},
    {Builtin::StrLength,          {"S", "l"}},
    {Builtin::StrIndex,           {"S", "i"}},
    {Builtin::StrReplace,         {"S", "si"}},
    {Builtin::StrConcatenate,     {"S", "a"}},
    {Builtin::StrSplit,           {"S", "d"}},
    {Builtin::StrEqual,           {"S", "e"}},
    {Builtin::StrNumtoChar,       {"S", "ns"}},
    {Builtin::StrChartoNum,       {"S", "sn"}},
    {Builtin::VarNew,             {"V", "n"}},
    {Builtin::VarDelete,          {"V", "d"}},
};

std::map<std::string, Class> get_builtins() {
    std::map<std::string, Class> builtins = {
        {"A", {"A"}}, {"I", {"I"}}, {"O", {"O"}}, {"S", {"S"}}, {"V", {"V"}}
    };

    for (auto &[builtin, builtin_info]: BUILTIN_INFO) {
        auto &[builtin_class, builtin_name] = builtin_info;
        builtins.at(builtin_class).add_function(builtin_name, {builtin});
    }

    return builtins;
}

// Remove the builtin classes from the given map of classes
void remove_builtins(std::map<std::string, Class> &classes) {
    classes.erase("A");
    classes.erase("I");
    classes.erase("O");
    classes.erase("S");
    classes.erase("V");
}

// Makes sure that the values on the top of the stack match the types required
// by a built-in function
bool types_match(const std::vector<Variable> &stack, const std::string &name,
                 const std::vector<VarType> &types)
{
    // If there aren't enough arguments on the stack for the function, give
    // an error message
    if (stack.size() < types.size()) {
        std::cerr << "Error! Built-in " << name << " method requires ";
        if (types.size() == 1) {
            std::cerr << "a single argument, but the stack is empty.\n";
        } else {
            std::cerr << types.size() << " arguments, but the stack ";
            if (stack.size() == 0) {
                std::cerr << "is empty.\n";
            } else {
                std::cerr << "only has " << stack.size() << " element"
                          << (stack.size() > 1 ? "s": "") << "\n";
            }
        }
        return false;
    }

    // Check to see whether the types of the values on the top of the stack
    // loop the required types for the function
    bool type_mismatch = false;
    for (size_t i = 1; i <= types.size(); i++) {
        if (stack[stack.size() - i].get_type() != types[types.size() - i]) {
            type_mismatch = true;
            break;
        }
    }

    // If there is a value on the top of the stack that doesn't match its
    // required type, give an error message telling the types needed for the
    // function and the types that it got
    if (type_mismatch) {
        std::cerr << "Error! Built-in " << name << " method requires ";
        if (types.size() == 1) {
            std::cerr << "an argument of the type " << get_type_name(types[0])
                      << "\nReceived an argument of the type "
                      << get_type_name(stack.back().get_type()) << "\n";
        } else {
            std::cerr << "arguments of the following types:\n ";
            for (auto &type: types) {
                std::cerr << " " << get_type_name(type);
            }
            std::cerr << "\nReceived arguments of the following types:\n ";
            for (size_t i = stack.size() - types.size(); i < stack.size(); i++) {
                std::cerr << " " << get_type_name(stack[i].get_type());
            }
            std::cerr << "\n";
        }
        return false;
    }

    return true;
}

// Handles a builtin function, returning true if there was an error
bool handle_builtin(Builtin type, std::vector<Variable> &stack,
                    std::map<std::string, Variable> &globals)
{
    switch (type) {
        case Builtin::InputLine: {
            std::string str;
            std::getline(std::cin, str);
            stack.emplace_back(VarType::String, str + '\n');
            break;
        }

        case Builtin::InputChar: {
            auto c = std::cin.get();
            stack.emplace_back(VarType::String, std::string{(char)c});
            break;
        }

        case Builtin::InputEof:
            stack.emplace_back(std::cin.eof() ? 1.0: 0.0);
            break;

        case Builtin::MathAdd: {
            if (not types_match(stack, "A.a", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num1 + *num2);
            break;
        }

        case Builtin::MathSub: {
            if (not types_match(stack, "A.s", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num2 - *num1);
            break;
        }

        case Builtin::MathMult: {
            if (not types_match(stack, "A.m", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num1 * *num2);
            break;
        }

        case Builtin::MathDiv: {
            if (not types_match(stack, "A.d", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num2 / *num1);
            break;
        }

        case Builtin::MathMod: {
            if (not types_match(stack, "A.mod", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(std::fmod(*num2, *num1));
            break;
        }

        case Builtin::MathFloor:  {
            if (not types_match(stack, "A.f", {VarType::Number})) {
                return true;
            }
            auto num = pop_stack(stack)->get_number();
            stack.emplace_back(std::floor(*num));
            break;
        }

        case Builtin::MathEqual: {
            if (not types_match(stack, "A.e", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num1 == *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathNotEqual:  {
            if (not types_match(stack, "A.ne", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num1 != *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathLessThan: {
            if (not types_match(stack, "A.lt", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num1 > *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathLessOrEqual: {
            if (not types_match(stack, "A.le", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num1 >= *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathGreaterThan: {
            if (not types_match(stack, "A.gt", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num1 < *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathGreaterOrEqual: {
            if (not types_match(stack, "A.ge", {VarType::Number, VarType::Number})) {
                return true;
            }
            auto num1 = pop_stack(stack)->get_number();
            auto num2 = pop_stack(stack)->get_number();
            stack.emplace_back(*num1 <= *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::OutputStr: {
            if (not types_match(stack, "O.o", {VarType::String})) {
                return true;
            }
            auto str = pop_stack(stack)->get_string();
            std::cout << *str;
            break;
        }

        case Builtin::OutputNumber: {
            if (not types_match(stack, "O.on", {VarType::Number})) {
                return true;
            }
            auto num = pop_stack(stack)->get_number();
            std::cout << *num;
            break;
        }

        case Builtin::StrLength: {
            if (not types_match(stack, "S.l", {VarType::String})) {
                return true;
            }
            auto str = pop_stack(stack)->get_string();
            stack.emplace_back(static_cast<double>(str->size()));
            break;
        }

        case Builtin::StrIndex: {
            if (not types_match(stack, "S.i", {VarType::String, VarType::Number})) {
                return true;
            }
            auto num = pop_stack(stack)->get_number();
            auto str = pop_stack(stack)->get_string();
            if (*num < 0) {
                stack.emplace_back(VarType::String, "");
                break;
            }
            auto index = static_cast<std::size_t>(*num);
            if (index >= str->size()) {
                stack.emplace_back(VarType::String, "");
            } else {
                stack.emplace_back(VarType::String, std::string{(*str)[index]});
            }
            break;
        }

        case Builtin::StrReplace: {
            if (not types_match(stack, "S.si", {VarType::String, VarType::Number, VarType::String})) {
                return true;
            }
            auto chr = pop_stack(stack)->get_string();
            auto num = pop_stack(stack)->get_number();
            auto string = pop_stack(stack)->get_string();
            if (chr->size() < 1) {
                std::cerr << "Error! Need non-empty string to replace character!\n";
                return true;
            } else if (chr->size() > 1) {
                std::cerr << "Error! Cannot replace character with multi-character string!\n";
                return true;
            }
            if (*num < 0) {
                std::cerr << "Error! Cannot use negative index into string!\n";
                return true;
            }
            auto index = static_cast<std::size_t>(*num);
            if (index >= string->size()) {
                std::cerr << "Error! Index into string is out of range!\n";
                return true;
            } else {
                auto new_string = *string;
                new_string[index] = (*chr)[0];
                stack.emplace_back(VarType::String, new_string);
            }
            break;
        }

        case Builtin::StrConcatenate: {
            if (not types_match(stack, "S.a", {VarType::String, VarType::String})) {
                return true;
            }
            auto str1 = pop_stack(stack)->get_string();
            auto str2 = pop_stack(stack)->get_string();
            stack.emplace_back(VarType::String, *str2 + *str1);
            break;
        }

        case Builtin::StrSplit: {
            if (not types_match(stack, "S.d", {VarType::String, VarType::Number})) {
                return true;
            }
            auto pos = pop_stack(stack)->get_number();
            auto string = pop_stack(stack)->get_string();
            if (*pos < 0) {
                stack.emplace_back(VarType::String, "");
                stack.emplace_back(VarType::String, *string);
                break;
            }
            auto index = static_cast<std::size_t>(*pos);
            if (index >= string->size()) {
                stack.emplace_back(VarType::String, *string);
                stack.emplace_back(VarType::String, "");
            } else {
                auto split1 = string->substr(0, index);
                auto split2 = string->substr(index);
                stack.emplace_back(VarType::String, split1);
                stack.emplace_back(VarType::String, split2);
            }
            break;
        }

        case Builtin::StrEqual: {
            if (not types_match(stack, "S.e", {VarType::String, VarType::String})) {
                return true;
            }
            auto str1 = pop_stack(stack)->get_string();
            auto str2 = pop_stack(stack)->get_string();
            stack.emplace_back(*str1 == *str2 ? 1.0: 0.0);
            break;
        }

        case Builtin::StrNumtoChar: {
            if (not types_match(stack, "S.ns", {VarType::Number})) {
                return true;
            }
            auto num = pop_stack(stack)->get_number();
            stack.emplace_back(VarType::String, std::string{(char) *num});
            break;
        }

        case Builtin::StrChartoNum: {
            if (not types_match(stack, "S.sn", {VarType::String})) {
                return true;
            }
            auto chr = pop_stack(stack)->get_string();
            if (chr->size() == 0) {
                std::cerr << "Error! Cannot convert empty string to number!\n";
                return true;
            } else if (chr->size() > 1) {
                std::cerr << "Error! Cannot convert multi-character string to number!\n";
                return true;
            }
            stack.emplace_back(static_cast<double>(chr->front()));
            break;
        }

        case Builtin::VarNew: {
            // Variable names start with digits are interpreted as globals, and
            // may not be used by the programmer, so they won't conflict with
            // the player's names, making integral values perfect for V.n
            static int cur_var = 0;
            stack.emplace_back(VarType::Name, std::to_string(cur_var++));
            break;
        }

        case Builtin::VarDelete: {
            if (not types_match(stack, "V.d", {VarType::Name})) {
                return true;
            }
            auto name = pop_stack(stack)->get_name();
            for (auto c: *name) {
                if (not std::isdigit(c)) {
                    std::cerr << "Error! Cannot delete non-generated name!\n";
                    return true;
                }
            }
            globals.erase(*name);
            break;
        }
    }

    return false;
}

// Returns the text for executing a given builtin function
// temp_name is a name to be given that won't conflict with any other
// names in the source code
std::string builtin_text(Builtin type, const std::string &temp_name) {
    auto [class_name, method_name] = BUILTIN_INFO.at(type);
    if (method_name.size() > 1) {
        method_name = "(" + method_name + ")";
    }
    return temp_name + class_name + "!" + temp_name + method_name + ".?";
}
