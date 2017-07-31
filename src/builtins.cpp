#include "builtins.hpp"
#include "class.hpp"
#include "variable.hpp"

#include <iostream>
#include <optional>

std::map<std::string, Class> get_builtins() {
    Class output;
    output.functions["o"] = {Builtin::OutputStr};
    output.functions["on"] = {Builtin::OutputNumber};

    return {{"O", output}};
}

bool handle_builtin(Builtin type, std::vector<Variable> &stack) {
    auto pop_stack = [&] () -> std::optional<Variable> {
        if (stack.size() == 0) {
            return std::nullopt;
        } else {
            auto back_val = stack.back();
            stack.pop_back();
            return back_val;
        }
    };

    switch (type) {
        case Builtin::OutputStr: {
            auto top = pop_stack();
            if (not top) {
                std::cerr << "Error! Attempted to pop empty stack!\n";
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
            auto top = pop_stack();
            if (not top) {
                std::cerr << "Error! Attempted to pop empty stack!\n";
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
    }

    return false;
}
