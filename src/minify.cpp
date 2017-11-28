#include "builtins.hpp"
#include "class.hpp"
#include "minify.hpp"
#include "string-things.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <unordered_set>

// Names that absolutely must stay the same, or else the program may not be
// able to call builtin functions or even run
const std::unordered_set<std::string> PREDEFINED_NAMES = {
    "A", "I", "M", "O", "S", "V", "a", "c", "d", "e", "f", "ge", "gt", "i",
    "l", "le", "lt", "m", "n", "ne", "ns", "mod", "o", "on", "s", "si", "sn",
    "c__", "d__"
};

// Given the list of classes, returns a list of names, ordered by how
// frequently they appear, with the most frequent names being first
std::vector<std::string> order_names(const ClassMap &classes) {
    std::unordered_map<std::string, int> name_freqs;
    for (const auto &[class_name, class_info]: classes) {
        name_freqs[class_name] += 1;

        for (const auto &parent_name: class_info.get_parents()) {
            name_freqs[parent_name] += 1;
        }

        for (const auto &[func_name, func_info]: class_info.get_functions()) {
            name_freqs[func_name] += 1;
            for (const auto &command: func_info) {
                if (command.get_type() == CommandType::LoopBegin) {
                    name_freqs[command.get_loop_var()] += 1;
                } else if (command.get_type() == CommandType::PushName)
                {
                    name_freqs[command.get_string()] += 1;
                }
            }
        }
    }
    std::vector<std::pair<int, std::string>> ordered_names;
    for (const auto &[name_freq, name]: name_freqs) {
        ordered_names.emplace_back(name, name_freq);
    }
    std::sort(ordered_names.rbegin(), ordered_names.rend());

    std::vector<std::string> names;
    for (const auto &name: ordered_names) {
        names.push_back(name.second);
    }
    return names;
}

// Returns an unordered_map the maps variable names to new, shorter names
std::unordered_map<std::string, std::string> reassign_names(const ClassMap &classes) {
    // The current name for each different type of name
    std::string upper_name = "a", lower_name = "a", under_name = "a";

    // "Increments" a name, moving it to the next possible name.
    // e.g. "a" becomes "b", "b" becomes "c", "z" becomes "aa", "az" becomes "ba"
    // The first letter must be a-z, as otherwise it could change the scope
    // (although, this is not true of local variables, as they are always
    // preceded by an underscore). All other letters can be a-z, A-Z or 0-9
    auto inc_name = [] (std::string &name, bool first_must_be_lower = true) {
        for (std::size_t i = 0; i < name.size(); i++) {
            std::size_t index = name.size() - i - 1;
            if (name[index] == 'Z') {
                name[index] = '0';
            } else if (name[index] == '9') {
                name[index] = 'a';
            } else if (name[index] == 'z') {
                if (index > 0 or not first_must_be_lower) {
                    name[index] = 'A';
                } else {
                    name[index] = 'a';
                }
            } else {
                name[index]++;
            }
            if (name[index] != 'a') {
                return;
            }
        }
        name = "a" + name;
    };

    std::unordered_map<std::string, std::string> reassigned_names;
    auto ordered_names = order_names(classes);

    // Go through each name, in order of their frequency, reassigning them to
    // have newer, shorter names. If the next name that would be assigned is
    // a built-in name, we skip over that name, and assign the next free name
    for (auto &name: ordered_names) {
        if (PREDEFINED_NAMES.count(name)) {
            reassigned_names[name] = name;
            continue;
        }
        std::string new_name;
        if (std::isupper(name[0])) {
            do {
                new_name = upper_name;
                new_name[0] = std::toupper(new_name[0]);
                inc_name(upper_name);
            } while (PREDEFINED_NAMES.count(new_name));
        } else if (std::islower(name[0])) {
            do {
                new_name = lower_name;
                inc_name(lower_name);
            } while (PREDEFINED_NAMES.count(new_name));
        } else {
            new_name = "_" + under_name;
            inc_name(under_name, true);
        }
        reassigned_names[name] = new_name;
    }

    return reassigned_names;
}

// Returns a minified respresentation of the source code, based off of the
// already-parsed class definitions
std::string get_minified_source(ClassMap &classes, std::size_t line_width,
                                bool minify_code, bool convert_code)
{
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

    std::unordered_map<std::string, std::string> reassigned_names;
    if (minify_code) {
        reassigned_names = reassign_names(classes);
    }
    remove_builtins(classes);

    std::string cur_line;
    std::string new_code;

    // Returns the representation of a name, with parentheses around it, if
    // it can't be written as a single character
    auto get_name = [&] (const std::string &name) -> std::string {
        std::string new_name;
        if (minify_code) {
            new_name = reassigned_names[name];
        } else {
            new_name = name;
        }
        if (new_name.size() > 1) {
            return "(" + new_name + ")";
        } else {
            return new_name;
        }
    };

    auto add_to_source = [&] (const std::string &new_src) {
        if (line_width == 0) {
            cur_line += new_src;
        } else if (cur_line.size() + new_src.size() > line_width) {
            new_code += cur_line + "\n";
            cur_line = new_src;
        } else {
            cur_line += new_src;
        }
    };

    for (const auto &[class_name, class_info]: classes) {
        add_to_source("{");
        add_to_source(get_name(class_name));

        if (not convert_code) {
            for (const auto &parent: class_info.get_parents()) {
                add_to_source(get_name(parent));
            }
        }
        for (const auto &[func_name, func_info]: class_info.get_functions()) {
            add_to_source("[");
            add_to_source(get_name(func_name));

            std::optional<Command> last_command;
            for (const auto &command: func_info) {
                switch (command.get_type()) {
                    case CommandType::AssignClass:
                        add_to_source("!");
                        break;

                    case CommandType::AssignSelf:
                        add_to_source("$");
                        break;

                    case CommandType::AssignValue:
                        add_to_source("=");
                        break;

                    case CommandType::DupElement:
                        add_to_source(get_number(command.get_number(), true));
                        break;

                    case CommandType::ExecuteFunc:
                        add_to_source("?");
                        break;

                    case CommandType::GetFunction:
                        add_to_source(".");
                        break;

                    case CommandType::GetValue:
                        add_to_source("*");
                        break;

                    case CommandType::LoopBegin:
                        add_to_source("/");
                        add_to_source(get_name(command.get_loop_var()));
                        break;

                    case CommandType::LoopEnd:
                        add_to_source("\\");
                        break;

                    case CommandType::PopStack:
                        add_to_source(",");
                        break;

                    case CommandType::PushName: {
                        auto name = command.get_string();
                        if (minify_code and last_command) {
                            if (last_command->get_type() == CommandType::PushName) {
                                if (last_command->get_string() == name) {
                                    add_to_source("0");
                                    break;
                                }
                            }
                        }
                        add_to_source(get_name(name));
                        break;
                    }

                    case CommandType::PushNumber:
                        add_to_source("<" + get_number(command.get_number(), false) + ">");
                        break;

                    case CommandType::PushString:
                        add_to_source("\"" + escape_str(command.get_string()) + "\"");
                        break;

                    case CommandType::Return:
                        add_to_source("^");
                        break;

                    case CommandType::BuiltinFunction:
                        add_to_source(builtin_text(command.get_builtin(), "(_t)"));
                        break;

                    // These commands are only generated by the optimizer, and
                    // since we don't run the optimizer before minification,
                    // these commands should never be encountered
                    case CommandType::AssignTo:
                    case CommandType::FuncCall:
                    case CommandType::NewInst:
                    case CommandType::Nop:
                        assert(false);
                        break;
                }
                last_command = command;
            }
            add_to_source("]");
        }
        add_to_source("}");
    }

    return new_code + cur_line;
}
