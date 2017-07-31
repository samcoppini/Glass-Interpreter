#include "builtins.hpp"
#include "parse.hpp"

#include <cctype>
#include <cmath>
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
        if (c == '\\') {
            if (file.get(c)) {
                switch (c) {
                    case 'a': c = '\a'; break;
                    case 'b': c = '\b'; break;
                    case 'e': c = '\x1b'; break;
                    case 'f': c = '\f'; break;
                    case 'n': c = '\n'; break;
                    case 'r': c = '\r'; break;
                    case 't': c = '\t'; break;
                    case 'v': c = '\v'; break;
                }
            }
        }
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
    } else if (not std::islower(func_name->front())) {
        std::cerr << "Error! Function name \"" << *func_name
                  << "\" must start with a lowercase letter!\n";
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
    } else if (not std::isupper(class_name->front())) {
        std::cerr << "Error! Class name \"" << *class_name
                  << "\" must start with a capital letter!\n";
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
    auto classes = get_builtins();
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

// Prints out a formatted version of the list of commands, starting at a
// certain level of indentation
void print_commands(const CommandList &commands, int tab_level) {
    if (commands.size() == 0) {
        return;
    }

    std::cout << "\n";
    for (int i = 0; i < tab_level; i++) {
        std::cout << "  ";
    }

    for (const auto &command: commands) {
        switch (command.type) {
            case CommandType::AssignClass:
                std::cout << "!";
                break;
            case CommandType::AssignSelf:
                std::cout << "$";
                break;
            case CommandType::AssignValue:
                std::cout << "=";
                break;
            case CommandType::DupElement: {
                auto dval = std::get<double>(command.data);
                if (std::round(dval) == dval and dval >= 0 and dval < 10) {
                    std::cout << dval;
                } else {
                    std::cout << "(" << dval << ")";
                }
                break;
            }
            case CommandType::ExecuteFunc:
                std::cout << "?";
                break;
            case CommandType::GetFunction:
                std::cout << ".";
                break;
            case CommandType::GetValue:
                std::cout << "*";
                break;
            case CommandType::PopStack:
                std::cout << ",";
                break;
            case CommandType::PushName: {
                auto str = std::get<std::string>(command.data);
                if (str.size() == 1) {
                    std::cout << str;
                } else {
                    std::cout << "(" << str << ")";
                }
                break;
            }
            case CommandType::PushNumber:
                std::cout << "<" << std::get<double>(command.data) << ">";
                break;
            case CommandType::PushString:
                std::cout << "\"" << std::get<std::string>(command.data) << "\"";
                break;
            case CommandType::Return:
                std::cout << "^";
                break;
            case CommandType::WhileLoop: {
                auto str = std::get<std::string>(command.data);
                std::cout << "\n";
                for (int i = 0; i < tab_level; i++) {
                    std::cout << "  ";
                }
                std::cout << "/";
                if (str.size() == 1) {
                    std::cout << str;
                } else {
                    std::cout << "(" << str << ")";
                }
                print_commands(command.loop_body, tab_level + 1);
                if (command.loop_body.size() > 0) {
                    for (int i = 0; i < tab_level; i++) {
                        std::cout << "  ";
                    }
                }
                std::cout << "\\";
                break;
            }

            case CommandType::BuiltinFunction:
                break;
        }
    }
    std::cout << "\n";
}

// Prints out a formatted version of the given classes
void print_classes(const std::map<std::string, Class> &classes) {
    for (const auto &class_pair: classes) {
        auto name = class_pair.first;
        if (name == "A" or name == "S" or name == "V" or name == "O" or name == "I") {
            continue;
        }
        std::cout << "{";
        if (class_pair.first.size() == 1) {
            std::cout << class_pair.first;
        } else {
            std::cout << "(" << class_pair.first << ")";
        }
        if (class_pair.second.functions.size() == 0) {
            std::cout << "}\n";
            continue;
        }
        std::cout << "\n";
        for (const auto &func: class_pair.second.functions) {
            std::cout << "  [";
            if (func.first.size() == 1) {
                std::cout << func.first;
            } else {
                std::cout << "(" << func.first << ")";
            }
            print_commands(func.second, 2);
            if (func.second.size() > 0) {
                std::cout << "  ";
            }
            std::cout << "]\n";
        }
        std::cout << "}\n";
    }
}
