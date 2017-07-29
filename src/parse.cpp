#include "parse.hpp"

#include <cctype>
#include <iostream>

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

// Returns a pair of a function's name and the commands in it, or nullopt
// if there is a parsing error
std::optional<std::pair<std::string, CommandList>> get_func(std::ifstream &file) {
    auto func_name = get_name(file);
    if (not func_name) {
        return std::nullopt;
    }

    CommandList func_cmds;
    char c;
    while (file.get(c) and c != ']') {
        // Parse commands here later
    }

    if (c != ']') {
        std::cerr << "Error! Unexpected end of file during function definition!\n";
        return std::nullopt;
    }

    return {{*func_name, func_cmds}};
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
