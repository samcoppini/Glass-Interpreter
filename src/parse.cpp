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

// Returns all of the classes in the file, or nullopt if there's some sort of
// parsing error
std::optional<std::map<std::string, Class>> get_classes(std::ifstream &file) {
    std::map<std::string, Class> classes;
    char c;

    while (file.get(c)) {
        if (std::isspace(c)) {
            continue;
        } else if (c == '{') {
            auto class_name = get_name(file);
            if (not class_name) {
                return std::nullopt;
            } else if (classes.count(*class_name)) {
                std::cerr << "Error! Class \"" << *class_name
                          << "\" defined multiple times!\n";
                return std::nullopt;
            } else {
                Class new_class;
                while (file.get(c) and c != '}') {
                    // Parse functions here later
                }
                if (c != '}') {
                    std::cerr << "Error! Unexpected end of file encountered in \""
                              << *class_name << "\" class definition.\n";
                    return std::nullopt;
                }
                classes[*class_name] = new_class;
            }
        } else {
            std::cerr << "Error! Was not expecting a \"" << c
                      << "\" character outside a class definition!\n";
            return std::nullopt;
        }
    }

    return classes;
}
