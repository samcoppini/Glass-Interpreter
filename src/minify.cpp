#include "minify.hpp"
#include "builtins.hpp"
#include "class.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <set>

// Names that absolutely must stay the same, or else the program may not be
// able to call builtin functions or even run
const std::set<std::string> PREDEFINED_NAMES = {
    "A", "I", "M", "O", "S", "V", "a", "c", "d", "e", "f", "ge", "gt", "i",
    "l", "le", "lt", "m", "n", "ne", "ns", "mod", "o", "on", "s", "si", "sn",
    "c__", "d__"
};

// Reproduce a string, converting characters to the appropriate escape sequences
std::string escape_string(const std::string &str) {
    std::string new_str;
    for (const auto &c: str) {
        switch (c) {
            case '"':    new_str += "\\\""; break;
            case '\\':   new_str += "\\\\"; break;
            case '\a':   new_str += "\\a";  break;
            case '\b':   new_str += "\\b";  break;
            case '\x1b': new_str += "\\e";  break;
            case '\f':   new_str += "\\f";  break;
            case '\n':   new_str += "\\n";  break;
            case '\r':   new_str += "\\r";  break;
            case '\t':   new_str += "\\t";  break;
            case '\v':   new_str += "\\f";  break;
            default:     new_str += c;      break;
        }
    }
    return new_str;
}

// Given the list of classes, returns a list of names, ordered by how
// frequently they appear, with the most frequent names being first
std::vector<std::string> order_names(std::map<std::string, Class> &classes) {
    std::map<std::string, int> name_freqs;
    for (const auto &class_info: classes) {
        name_freqs[class_info.first] += 1;
        for (const auto &func: class_info.second.get_functions()) {
            name_freqs[func.first] += 1;
            for (const auto &command: func.second) {
                if (command.get_type() == CommandType::LoopBegin or
                    command.get_type() == CommandType::PushName)
                {
                        name_freqs[command.get_string()] += 1;
                }
            }
        }
    }
    std::vector<std::pair<int, std::string>> ordered_names;
    for (const auto &name: name_freqs) {
        ordered_names.emplace_back(name.second, name.first);
    }
    std::sort(ordered_names.rbegin(), ordered_names.rend());

    std::vector<std::string> names;
    for (const auto &name: ordered_names) {
        names.push_back(name.second);
    }
    return names;
}

// Returns a minified respresentation of the source code, based off of the
// already-parsed class definitions
std::string get_minified_source(std::map<std::string, Class> &classes) {
    std::map<std::string, std::string> reassigned_names;

    // Make sure we don't reassign the names of builtin functions
    for (const auto &name: PREDEFINED_NAMES) {
        reassigned_names[name] = name;
    }

    // The current names for Global, class and _local variables
    std::string upper_name = "`", lower_name = "`", under_name = "`";
    std::string new_code;

    remove_builtins(classes);

    // Increments a name. 'a' becomes 'b', 'aa' becomes 'ab', 'az' becomes 'ba'
    // 'zzz' becomes 'aaaa', etc...
    auto inc_name = [&] (std::string &name) {
        for (std::size_t i = 0; i < name.size(); i++) {
            std::size_t index = name.size() - i - 1;
            if (name[index] == 'z') {
                name[index] = 'a';
            } else {
                name[index]++;
                return;
            }
        }
        name += 'a';
    };

    // Returns a string representation of a number
    auto get_number = [] (const double &num, bool use_paren) -> std::string {
        auto num_str = std::to_string(num);

        // Remove unneeded trailing zeros, e.g. "7.00000" becomes "7"
        while (num_str.find('.') != std::string::npos) {
            if (num_str.back() == '0' or num_str.back() == '.') {
                num_str.pop_back();
            } else {
                break;
            }
        }
        if (use_paren and num_str.size() > 1) {
            return "(" + num_str + ")";
        } else {
            return num_str;
        }
    };

    // Reassign a name to a new, (hopefully) shorter name
    auto assign_name = [&] (const std::string &name) {
        if (PREDEFINED_NAMES.count(name)) {
            return;
        }
        if (std::isupper(name[0])) {
            inc_name(upper_name);
            auto new_name = upper_name;
            new_name[0] = std::toupper(new_name[0]);
            while (PREDEFINED_NAMES.count(new_name)) {
                inc_name(upper_name);
                new_name = upper_name;
                new_name[0] = std::toupper(new_name[0]);
            }
            reassigned_names[name] = new_name;
        } else if (std::islower(name[0])) {
            inc_name(lower_name);
            while (PREDEFINED_NAMES.count(lower_name)) {
                inc_name(lower_name);
            }
            reassigned_names[name] = lower_name;
        } else {
            inc_name(under_name);
            reassigned_names[name] = "_" + under_name;
        }
    };

    // Returns the representation of a name, with parentheses around it, if
    // it can't be written as a single character
    auto get_name = [&] (const std::string &name) -> std::string {
        auto new_name = reassigned_names[name];
        if (new_name.size() > 1) {
            return "(" + new_name + ")";
        } else {
            return new_name;
        }
    };

    auto names = order_names(classes);
    for (const auto &name: names) {
        assign_name(name);
    }

    for (const auto &class_info: classes) {
        new_code += "{";
        new_code += get_name(class_info.first);
        for (const auto &func_info: class_info.second.get_functions()) {
            new_code += "[";
            new_code += get_name(func_info.first);
            for (const auto &command: func_info.second) {
                switch (command.get_type()) {
                    case CommandType::AssignClass:
                        new_code += "!";
                        break;

                    case CommandType::AssignSelf:
                        new_code += "$";
                        break;

                    case CommandType::AssignValue:
                        new_code += "=";
                        break;

                    case CommandType::DupElement:
                        new_code += get_number(command.get_number(), true);
                        break;

                    case CommandType::ExecuteFunc:
                        new_code += "?";
                        break;

                    case CommandType::GetFunction:
                        new_code += ".";
                        break;

                    case CommandType::GetValue:
                        new_code += "*";
                        break;

                    case CommandType::LoopBegin:
                        new_code += "/" + get_name(command.get_string());
                        break;

                    case CommandType::LoopEnd:
                        new_code += "\\";
                        break;

                    case CommandType::PopStack:
                        new_code += ",";
                        break;

                    case CommandType::PushName:
                        new_code += get_name(command.get_string());
                        break;

                    case CommandType::PushNumber:
                        new_code += "<" + get_number(command.get_number(), false) + ">";
                        break;

                    case CommandType::PushString:
                        new_code += "\"" + escape_string(command.get_string()) + "\"";
                        break;

                    case CommandType::Return:
                        new_code += "^";
                        break;

                    case CommandType::BuiltinFunction:
                        assert(false);
                        break;
                }
            }
            new_code += "]";
        }
        new_code += "}";
    }

    return new_code;
}
