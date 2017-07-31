#include "builtins.hpp"
#include "class.hpp"
#include "variable.hpp"

#include <cctype>
#include <cmath>
#include <iostream>
#include <optional>

std::map<std::string, Class> get_builtins() {
    Class input;
    input.functions["l"] = {Builtin::InputLine};
    input.functions["c"] = {Builtin::InputChar};
    input.functions["e"] = {Builtin::InputEof};

    Class output;
    output.functions["o"] = {Builtin::OutputStr};
    output.functions["on"] = {Builtin::OutputNumber};

    Class math;
    math.functions["a"] = {Builtin::MathAdd};
    math.functions["s"] = {Builtin::MathSub};
    math.functions["m"] = {Builtin::MathMult};
    math.functions["d"] = {Builtin::MathDiv};
    math.functions["mod"] = {Builtin::MathMod};
    math.functions["f"] = {Builtin::MathFloor};
    math.functions["e"] = {Builtin::MathEqual};
    math.functions["ne"] = {Builtin::MathNotEqual};
    math.functions["lt"] = {Builtin::MathLessThan};
    math.functions["le"] = {Builtin::MathLessOrEqual};
    math.functions["gt"] = {Builtin::MathGreaterThan};
    math.functions["ge"] = {Builtin::MathGreaterOrEqual};

    Class string;
    string.functions["l"] = {Builtin::StrLength};
    string.functions["i"] = {Builtin::StrIndex};
    string.functions["si"] = {Builtin::StrReplace};
    string.functions["a"] = {Builtin::StrConcatenate};
    string.functions["d"] = {Builtin::StrSplit};
    string.functions["e"] = {Builtin::StrEqual};
    string.functions["ns"] = {Builtin::StrNumtoChar};
    string.functions["sn"] = {Builtin::StrChartoNum};

    Class vars;
    vars.functions["n"] = {Builtin::VarNew};
    vars.functions["d"] = {Builtin::VarDelete};

    return {{"A", math}, {"I", input}, {"O", output}, {"S", string}, {"V", vars}};
}

// Handles a builtin function, returning true if there was an error
bool handle_builtin(Builtin type, std::vector<Variable> &stack) {
    switch (type) {
        case Builtin::InputLine: {
            std::string str;
            std::getline(std::cin, str);
            stack.emplace_back(VarType::String, str);
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
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't add non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num1 + *num2);
            break;
        }

        case Builtin::MathSub: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't subtract non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num2 - *num1);
            break;
        }

        case Builtin::MathMult: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't multiply non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num1 * *num2);
            break;
        }

        case Builtin::MathDiv: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't divide non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num2 / *num1);
            break;
        }

        case Builtin::MathMod: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't perform modulo on non-numbers!\n";
                return true;
            }
            stack.emplace_back(std::fmod(*num2, *num1));
            break;
        }

        case Builtin::MathFloor:  {
            auto top = pop_stack(stack);
            if (not top) {
                return true;
            }
            auto num = top->get_number();
            if (not num) {
                std::cerr << "Error! Can't floor non-number!\n";
                return true;
            }
            stack.emplace_back(std::floor(*num));
            break;
        }

        case Builtin::MathEqual: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't check equality of non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num1 == *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathNotEqual:  {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't check nonequality of non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num1 != *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathLessThan: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't compare non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num1 >= *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathLessOrEqual: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't compare non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num1 > *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathGreaterThan: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't compare non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num1 <= *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::MathGreaterOrEqual: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num1 = stack1->get_number(), num2 = stack2->get_number();
            if (not num1 or not num2) {
                std::cerr << "Error! Can't compare non-numbers!\n";
                return true;
            }
            stack.emplace_back(*num1 < *num2 ? 1.0: 0.0);
            break;
        }

        case Builtin::OutputStr: {
            auto top = pop_stack(stack);
            if (not top) {
                return true;
            }
            if (auto str = top->get_string()) {
                std::cout << *str;
            } else if (auto name = top->get_name()) {
                std::cout << *name;
            } else {
                std::cerr << "Error! Attempted to output non-string as string!\n";
                return true;
            }
            break;
        }

        case Builtin::OutputNumber: {
            auto top = pop_stack(stack);
            if (not top) {
                return true;
            }
            if (auto num = top->get_number()) {
                std::cout << *num;
            } else {
                std::cerr << "Error! Attempted to output non-number as number!\n";
                return true;
            }
            break;
        }

        case Builtin::StrLength: {
            auto top = pop_stack(stack);
            if (not top) {
                return true;
            }
            if (auto str = top->get_string()) {
                stack.emplace_back(static_cast<double>(str->size()));
            } else {
                std::cerr << "Error! Cannot get length of non-string!\n";
                return true;
            }
            break;
        }

        case Builtin::StrIndex: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto num = stack1->get_number();
            auto str = stack2->get_string();
            if (not num) {
                std::cerr << "Cannot get index of non-number!\n";
                return true;
            } else if (not str) {
                std::cerr << "Cannot index a non-string!\n";
                return true;
            }
            auto index = static_cast<int>(*num);
            if (index < 0 or (unsigned) index >= str->size()) {
                stack.emplace_back(VarType::String, "");
            } else {
                stack.emplace_back(VarType::String, std::string{(*str)[index]});
            }
            break;
        }

        case Builtin::StrReplace: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack),
                 stack3 = pop_stack(stack);
            if (not stack3) {
                return true;
            }
            auto string = stack3->get_string(), chr = stack1->get_string();
            auto num = stack2->get_number();
            if (not string) {
                std::cerr << "Error! Cannot replace character in non-string!\n";
                return true;
            } else if (not num) {
                std::cerr << "Error! Cannot use non-number as an index!\n";
                return true;
            } else if (not chr) {
                std::cerr << "Error! Cannot replace character with non-string!\n";
                return true;
            } else if (chr->size() < 1) {
                std::cerr << "Error! Need non-empty string to replace character!\n";
                return true;
            } else if (chr->size() > 1) {
                std::cerr << "Error! Cannot replace character with multi-character string!\n";
                return true;
            }
            auto index = static_cast<int>(*num);
            if (index < 0 or (unsigned) index >= string->size()) {
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
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto str1 = stack1->get_string(), str2 = stack2->get_string();
            if (not str1 or not str2) {
                std::cerr << "Error! Cannot concatenate non-string!\n";
                return true;
            }
            stack.emplace_back(VarType::String, *str2 + *str1);
            break;
        }

        case Builtin::StrSplit: {
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto string = stack2->get_string();
            auto pos = stack1->get_number();
            if (not string) {
                std::cerr << "Error! Cannot split non-string!\n";
                return true;
            } else if (not pos) {
                std::cerr << "Error! Cannot use non-number as index to split string!\n";
                return true;
            }
            auto index = static_cast<int>(*pos);
            if (index <= 0) {
                stack.emplace_back(VarType::String, "");
                stack.emplace_back(VarType::String, *string);
            } else if ((unsigned) index >= string->size()) {
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
            auto stack1 = pop_stack(stack), stack2 = pop_stack(stack);
            if (not stack2) {
                return true;
            }
            auto str1 = stack1->get_string(), str2 = stack2->get_string();
            if (not str1 or not str2) {
                std::cerr << "Error! Cannot check equality of non-strings!\n";
                return true;
            }
            stack.emplace_back(*str1 == *str2 ? 1.0: 0.0);
            break;
        }

        case Builtin::StrNumtoChar: {
            auto top = pop_stack(stack);
            if (not top) {
                return true;
            }
            auto num = top->get_number();
            if (not num) {
                std::cerr << "Error! Cannot convert non-number from number to character!\n";
                return true;
            }
            stack.emplace_back(VarType::String, std::string{(char) *num});
            break;
        }

        case Builtin::StrChartoNum: {
            auto top = pop_stack(stack);
            if (not top) {
                return true;
            }
            auto chr = top->get_string();
            if (not chr) {
                std::cerr << "Error! Cannot convert non-string from character to number!\n";
                return true;
            } else if (chr->size() == 0) {
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
            // Not exactly sure what this method is supposed to do, and it does
            // nothing in reference implementation??? So I just pop the stack and
            // check that it's an auto-generated name, and call that good
            auto top = pop_stack(stack);
            if (not top) {
                return true;
            }
            auto name = top->get_name();
            if (not name) {
                std::cerr << "Error! Cannot delete non-name!\n";
                return true;
            }
            for (auto c: *name) {
                if (not std::isdigit(c)) {
                    std::cerr << "Error! Cannot delete non-generated name!\n";
                    return true;
                }
            }
            break;
        }
    }

    return false;
}
