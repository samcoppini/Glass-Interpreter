#include "parse.hpp"

#include <cctype>
#include <iostream>

// Tries to get a number from the file that is ended by end_char, and returns
// the number as a double, or returns nullopt if there's a parsing error
std::optional<double> get_number(std::ifstream &file, char end_char) {
    std::string str;
    char c;

    while (file.get(c) and c != end_char) {
        str += c;
    }

    if (c != end_char) {
        std::cerr << "Error! End of file reached while parsing number!\n";
        return std::nullopt;
    }

    try {
        return std::stod(str);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Error! Was expecting a number, but \"" << str
                  << "\" is not a number.\n";
        return std::nullopt;
    } catch (const std::out_of_range &e) {
        std::cerr << "Error! " << str
                  << " is too large to be represented as a number!\n";
        return std::nullopt;
    }
}

// Tries to get a name from the file, returning it as a string if there is
// one, or nullopt if there's some sort of parsing error
std::optional<std::string> get_name(std::ifstream &file) {
    std::string name;
    char c;

    if (not file.get(c)) {
        std::cerr << "Error! End of file encountered while reading name!\n";
        return std::nullopt;
    } else if (c == '(') {
        while (file.get(c) and c != ')') {
            if (std::isalnum(c) or c == '_') {
                if (name.size() == 0 and std::isdigit(c)) {
                    std::cerr << "Error! \"" << c << "\" may not be used to "
                              << "start a name!\n";
                    return std::nullopt;
                }
                name += c;
            } else {
                std::cerr << "Error! Unexpected \"" << c
                          << "\" encountered when parsing name!\n";
                return std::nullopt;
            }
        }
        if (c != ')') {
            std::cerr << "Error! End of file encountered while reading name!\n";
            return std::nullopt;
        }
    } else if (std::isalpha(c)) {
        name = c;
    } else {
        std::cerr << "Error! \"" << c << "\" is not a valid name!\n";
        return std::nullopt;
    }

    if (name.size() == 0) {
        std::cerr << "Error! Name cannot be zero-length!\n";
        return std::nullopt;
    }

    return name;
}

// Gets a "" string from the file and returns its contents, or nullopt if there
// is some sort of parsing error
std::optional<std::string> get_string(std::ifstream &file) {
    std::string str;
    char c;

    while (file.get(c) and c != '"') {
        str += c;
    }

    if (c != '"') {
        std::cerr << "Error! End of file encountered when parsing string!\n";
        return std::nullopt;
    }

    return str;
}

// Returns a list of commands from the file, ended by a given character, or
// returns nullopt if there's some sort of parsing error
std::optional<CommandList> get_commands(std::ifstream &file, char end_char) {
    CommandList commands;
    char c;

    while (file.get(c) and c != end_char) {
        switch (c) {
            case ',':
                commands.emplace_back(CommandType::PopStack);
                break;

            case '^':
                commands.emplace_back(CommandType::Return);
                break;

            case '=':
                commands.emplace_back(CommandType::AssignValue);
                break;

            case '!':
                commands.emplace_back(CommandType::AssignClass);
                break;

            case '.':
                commands.emplace_back(CommandType::GetFunction);
                break;

            case '?':
                commands.emplace_back(CommandType::ExecuteFunc);
                break;

            case '*':
                commands.emplace_back(CommandType::GetValue);
                break;

            case '$':
                commands.emplace_back(CommandType::AssignSelf);
                break;

            case '"': {
                auto str_val = get_string(file);
                if (not str_val) {
                    return std::nullopt;
                }
                commands.emplace_back(CommandType::PushString, *str_val);
                break;
            }

            case '/': {
                auto name = get_name(file);
                if (not name) {
                    return std::nullopt;
                }
                auto loop_body = get_commands(file, '\\');
                if (not loop_body) {
                    return std::nullopt;
                }
                commands.emplace_back(CommandType::WhileLoop, *name, *loop_body);
                break;
            }

            case '(':
                if (not file.get(c)) {
                    continue;
                } else if (std::isdigit(c)) {
                    file.unget();
                    auto num = get_number(file, ')');
                    if (not num) {
                        return std::nullopt;
                    }
                    commands.emplace_back(CommandType::DupElement, *num);
                } else {
                    file.unget();
                    file.unget();
                    auto name = get_name(file);
                    if (not name) {
                        return std::nullopt;
                    }
                    commands.emplace_back(CommandType::PushName, *name);
                }
                break;

            case '<': {
                auto num_val = get_number(file, '>');
                if (not num_val) {
                    return std::nullopt;
                }
                commands.emplace_back(CommandType::PushNumber, *num_val);
                break;
            }

            default:
                if (std::isalpha(c)) {
                    commands.emplace_back(CommandType::PushName, std::string{c});
                } else if (std::isdigit(c)) {
                    commands.emplace_back(CommandType::DupElement, c - '0');
                } else if (not std::isspace(c)) {
                    std::cerr << "Error! Unexpected \"" << c
                              << "\" encountered when parsing function!\n";
                    return std::nullopt;
                }
                break;
        }
    }

    if (c != end_char) {
        std::cerr << "Error! End of file encountered when parsing function!\n";
        return std::nullopt;
    }

    return commands;
}

// Returns a pair of a function's name and the commands in it, or nullopt
// if there is a parsing error
std::optional<std::pair<std::string, CommandList>> get_func(std::ifstream &file) {
    auto func_name = get_name(file);
    if (not func_name) {
        return std::nullopt;
    }

    auto func_cmds = get_commands(file, ']');
    if (not func_cmds) {
        return std::nullopt;
    }

    return {{*func_name, *func_cmds}};
}

// Returns a pair of class's name, and the actual class, or nullopt if there is
// a parsing error
std::optional<std::pair<std::string, Class>> get_class(std::ifstream &file) {
    auto class_name = get_name(file);
    if (not class_name) {
        return std::nullopt;
    }

    Class new_class;
    char c;
    while (file.get(c) and c != '}') {
        if (std::isspace(c)) {
            continue;
        } else if (c == '[') {
            auto func = get_func(file);
            if (not func) {
                return std::nullopt;
            }
            auto func_name = func->first;
            auto commands = func->second;
            if (new_class.functions.count(func_name)) {
                std::cerr << "Error! \"" << *class_name
                          << "\" has multiple definitions of \""
                          << func_name << "\".\n";
                return std::nullopt;
            }
            new_class.functions[func_name] = commands;
        } else {
            std::cerr << "Error! Unexpected \"" << c
                      << "\" character encountered when parsing \""
                      << *class_name << "\".\n";
            return std::nullopt;
        }
    }

    if (c != '}') {
        std::cerr << "Error! Unexpected end of file during class definition!\n";
        return std::nullopt;
    }

    return {{*class_name, new_class}};
}

// Returns all of the classes in the file, or nullopt if there's some sort of
// parsing error
std::optional<std::map<std::string, Class>> get_classes(std::ifstream &file) {
    std::map<std::string, Class> classes;
    char c;

    while (file.get(c)) {
        if (std::isspace(c)) {
            continue;
        } else if (c == '{') {
            auto class_pair = get_class(file);
            if (not class_pair) {
                return std::nullopt;
            }
            auto class_name = class_pair->first;
            auto new_class = class_pair->second;
            if (classes.count(class_name)) {
                std::cerr << "Error! Class \"" << class_name
                          << "\" defined multiple times!\n";
                return std::nullopt;
            }
            classes[class_name] = new_class;
        } else {
            std::cerr << "Error! Was not expecting a \"" << c
                      << "\" character outside a class definition!\n";
            return std::nullopt;
        }
    }

    return classes;
}
