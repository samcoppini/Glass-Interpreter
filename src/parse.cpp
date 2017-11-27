#include "builtins.hpp"
#include "file.hpp"
#include "parse.hpp"
#include "string-things.hpp"

#include <cctype>
#include <cmath>
#include <iostream>
#include <stack>
#include <unordered_set>

using namespace std::string_literals;

// Print an error message with information about the line and column the
// error occurred on
void parse_error(const std::string &file_name, int line, int col,
                 const std::string &err)
{
    std::cerr << "Error in " << file_name << ", line " << line << ", column "
              << col << ":\n" << err << "\n";
}

// Reads a comment, and returns whether the file ended before the comment did
bool get_comment(File &file) {
    char c;

    int start_line = file.get_line();
    int start_col = file.get_col();

    while (file.get(c)) {
        if (c == '\'') {
            return false;
        }
    }

    parse_error(file.get_name(), start_line, start_col,
                "Unexpected end-of-file while reading comment.");

    return true;
}

// Tries to get a number from the file that is ended by end_char, and returns
// the number as a double, or returns nullopt if there's a parsing error
std::optional<double> get_number(File &file, char end_char) {
    std::string str;
    char c;

    int start_line = file.get_line();
    int start_col = file.get_col();

    while (file.get(c) and c != end_char) {
        if (c == '\'') {
            if (get_comment(file)) {
                return std::nullopt;
            }
        } else {
            str += c;
        }
    }

    if (c != end_char) {
        parse_error(file.get_name(), start_line, start_col,
                    "Unexpected end-of-file when reading a number.");
        return std::nullopt;
    }

    if (not valid_number(str)) {
        parse_error(file.get_name(), start_line, start_col,
                    "Invalid number \"" + str + "\".");
        return std::nullopt;
    }
    try {
        return std::stod(str);
    } catch (const std::out_of_range &e) {
        parse_error(file.get_name(), start_line, start_col,
                    str + " is too large to be represented as a number.");
        return std::nullopt;
    }
}

// Tries to get a name from the file, returning it as a string if there is
// one, or nullopt if there's some sort of parsing error
// paren_started indicates whether an opening parenthesis preceded the
// current location in the file
std::optional<std::string> get_name(File &file, bool paren_started = false) {
    std::string name;
    char c;

    if (not paren_started) {
        while (file.get(c) and std::isspace(c)) {
            // Skip over any whitespace if we're expecting a name coming up
        }
        if (file.eof()) {
            parse_error(file.get_name(), file.get_line(), file.get_col(),
                        "End of file encountered when expecting name.");
            return std::nullopt;
        }
    }

    int start_line = file.get_line();
    int start_col = file.get_col();

    if (paren_started or c == '(') {
        while (file.get(c) and c != ')') {
            if (std::isalnum(c) or c == '_') {
                if (name.size() == 0 and std::isdigit(c)) {
                    parse_error(file.get_name(), file.get_line(), file.get_col(),
                                "\""s + c + "\" may not be used to start a name.");
                    return std::nullopt;
                }
                name += c;
            } else if (c == '\'') {
                if (get_comment(file)) {
                    return std::nullopt;
                }
            } else {
                parse_error(file.get_name(), file.get_line(), file.get_col(),
                            "Unexpected \""s + c
                            + "\" encountered when parsing name.");
                return std::nullopt;
            }
        }
        if (c != ')') {
            parse_error(file.get_name(), start_line, start_col,
                        "Unexpected end of file encountered when reading name.");
            return std::nullopt;
        }
    } else if (c == '\'') {
       if (get_comment(file)) {
           return std::nullopt;
       } else {
           return get_name(file);
       }
    } else if (std::isalpha(c)) {
        name = c;
    } else {
        parse_error(file.get_name(), file.get_line(), file.get_col(),
                    "Expected name, but \""s + c + "\" is not a valid name.");
        return std::nullopt;
    }

    if (name.size() == 0) {
        parse_error(file.get_name(), start_line, start_col,
                    "Cannot use zero-length name.");
        return std::nullopt;
    }

    return name;
}

// Gets a "" string from the file and returns its contents, or nullopt if there
// is some sort of parsing error
std::optional<std::string> get_string(File &file) {
    std::string str;
    char c;

    int start_line = file.get_line();
    int start_col = file.get_col();

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
        parse_error(file.get_name(), start_line, start_col,
                    "Unexpected end-of-file encountered when parsing string.");
        return std::nullopt;
    }

    return str;
}

// Returns a list of commands from the file, ended by a given character, or
// returns nullopt if there's some sort of parsing error
std::optional<CommandList> get_commands(File &file) {
    std::stack<int> loop_stack;
    std::stack<std::pair<int, int>> loop_locs;
    CommandList commands;
    char c;

    int start_line = file.get_line();
    int start_col = file.get_col();

    auto add_command = [&] (const auto &...args) {
        commands.emplace_back(args..., file.get_name(), file.get_line(),
                                       file.get_col());
    };

    while (file.get(c) and c != ']') {
        switch (c) {
            case '\'':
                if (get_comment(file)) {
                    return std::nullopt;
                }
                break;

            case ',':
                add_command(CommandType::PopStack);
                break;

            case '^':
                add_command(CommandType::Return);
                break;

            case '=':
                add_command(CommandType::AssignValue);
                break;

            case '!':
                add_command(CommandType::AssignClass);
                break;

            case '.':
                add_command(CommandType::GetFunction);
                break;

            case '?':
                add_command(CommandType::ExecuteFunc);
                break;

            case '*':
                add_command(CommandType::GetValue);
                break;

            case '$':
                add_command(CommandType::AssignSelf);
                break;

            case '"': {
                auto str_val = get_string(file);
                if (not str_val) {
                    return std::nullopt;
                }
                add_command(CommandType::PushString, *str_val);
                break;
            }

            case '/': {
                loop_locs.emplace(file.get_line(), file.get_col());
                auto name = get_name(file);
                if (not name) {
                    return std::nullopt;
                }
                loop_stack.push(commands.size());
                add_command(CommandType::LoopBegin, *name, 0);
                break;
            }

            case '\\': {
                if (loop_stack.size() == 0) {
                    parse_error(file.get_name(), file.get_line(), file.get_col(),
                                "Unexpected \"\\\" encountered outside of a loop.");
                    return std::nullopt;
                }
                auto &loop_begin = commands[loop_stack.top()];
                loop_begin.set_jump(commands.size());
                add_command(CommandType::LoopEnd, loop_begin.get_loop_var(),
                            loop_stack.top());
                loop_stack.pop();
                loop_locs.pop();
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
                    add_command(CommandType::DupElement, *num);
                } else {
                    file.unget();
                    auto name = get_name(file, true);
                    if (not name) {
                        return std::nullopt;
                    }
                    add_command(CommandType::PushName, *name);
                }
                break;

            case '<': {
                auto num_val = get_number(file, '>');
                if (not num_val) {
                    return std::nullopt;
                }
                add_command(CommandType::PushNumber, *num_val);
                break;
            }

            default:
                if (std::isalpha(c)) {
                    add_command(CommandType::PushName, std::string{c});
                } else if (std::isdigit(c)) {
                    add_command(CommandType::DupElement, c - '0');
                } else if (not std::isspace(c)) {
                    parse_error(file.get_name(), file.get_line(), file.get_col(),
                                "Invalid command \""s + c
                                + "\" encountered when parsing a function.");
                    return std::nullopt;
                }
                break;
        }
    }

    if (c != ']') {
        parse_error(file.get_name(), start_line, start_col,
                    "Error! End of file encountered when parsing function.");
        return std::nullopt;
    }

    if (loop_stack.size() > 0) {
        if (loop_stack.size() == 1) {
            parse_error(file.get_name(), loop_locs.top().first,
                        loop_locs.top().second,
                        "Loop is not closed before end of function.");
        }
        return std::nullopt;
    }

    return commands;
}

// Returns a pair of a function's name and the commands in it, or nullopt
// if there is a parsing error
std::optional<std::pair<std::string, CommandList>> get_func(File &file) {
    int start_line = file.get_line();
    int start_col = file.get_col();

    auto func_name = get_name(file);
    if (not func_name) {
        return std::nullopt;
    } else if (not std::islower(func_name->front())) {
        parse_error(file.get_name(), start_line, start_col,
                    "Function name \"" + *func_name
                    + "\" must start with a lowercase letter.");
        return std::nullopt;
    }

    auto func_cmds = get_commands(file);
    if (not func_cmds) {
        return std::nullopt;
    }

    return {{*func_name, *func_cmds}};
}

// Returns a pair of class's name, and the actual class, or nullopt if there is
// a parsing error
std::optional<std::pair<std::string, Class>> get_class(File &file, bool pedantic) {
    int start_line = file.get_line();
    int start_col = file.get_col();

    auto class_name = get_name(file);
    if (not class_name) {
        return std::nullopt;
    } else if (not std::isupper(class_name->front())) {
        parse_error(file.get_name(), start_line, start_col,
                    "Class name \"" + *class_name
                    + "\" must begin with a capital letter.");
        return std::nullopt;
    }

    Class new_class{*class_name};
    char c;
    while (file.get(c) and c != '}') {
        if (std::isspace(c)) {
            continue;
        } else if (c == '[') {
            int func_line = file.get_line();
            int func_col = file.get_col();

            auto func = get_func(file);
            if (not func) {
                return std::nullopt;
            }
            auto [func_name, commands] = *func;
            if (new_class.add_function(func_name, commands)) {
                parse_error(file.get_name(), func_line, func_col,
                            "\"" + *class_name + "\" has multiple definitions of \""
                            + func_name + "\".");
                return std::nullopt;
            }
        } else if (c == '\'') {
            if (get_comment(file)) {
                return std::nullopt;
            }
        } else if (not pedantic and (c == '(' or std::isalpha(c))) {
            file.unget();
            int parent_line = file.get_line();
            int parent_col = file.get_col();

            auto parent_name = get_name(file, false);
            if (not parent_name) {
                return std::nullopt;
            }
            if (new_class.add_parent(*parent_name)) {
                parse_error(file.get_name(), parent_line, parent_col,
                            *class_name + " inherits from \""
                            + *parent_name + "\" multiple times.");
                return std::nullopt;
            }
        } else {
            int err_line = file.get_line();
            int err_col = file.get_col();
            parse_error(file.get_name(), err_line, err_col,
                        "Unexpected \""s + c
                        + "\" character encountered when parsing class "
                        + *class_name + ".");
            return std::nullopt;
        }
    }

    if (c != '}') {
        parse_error(file.get_name(), start_line, start_col,
                    "Unexpected end of file when parsing class definition.");
        return std::nullopt;
    }

    return {{*class_name, new_class}};
}

// Returns a pair of all the classes in the given file, and all the files
// included by the file. However, if there is a parsing error, this will
// instead return std::nullopt
std::optional<std::pair<ClassMap, std::vector<std::string>>>
parse_file(const std::string &filename, bool pedantic) {
    File file{filename};
    if (not file.is_open()) {
        std::cerr << "Unable to open \"" << filename << "\".\n";
        return std::nullopt;
    }

    std::vector<std::string> included_files;
    ClassMap classes;
    char c;

    while (file.get(c)) {
        if (std::isspace(c)) {
            continue;
        } else if (c == '{') {
            int class_line = file.get_line();
            int class_col = file.get_col();

            auto class_pair = get_class(file, pedantic);
            if (not class_pair) {
                return std::nullopt;
            }
            auto [class_name, new_class] = *class_pair;
            if (classes.count(class_name)) {
                parse_error(file.get_name(), class_line, class_col,
                            "Class " + class_name + " is defined multiple times.");
                return std::nullopt;
            }
            classes.insert_or_assign(class_name, new_class);
        } else if (not pedantic and c == '"') {
            auto new_file = get_string(file);
            if (not new_file) {
                return std::nullopt;
            }
            included_files.push_back(*new_file);
        } else if (c == '\'') {
            if (get_comment(file)) {
                return std::nullopt;
            }
        } else {
            parse_error(file.get_name(), file.get_line(), file.get_col(),
                        "Unexpected character \""s + c + "\" encountered.");
            return std::nullopt;
        }
    }

    return {{classes, included_files}};
}

// Gets the classes from a file, including additional classes from files
// included by the given file. Returns std::nullopt if there is any sort
// of error
std::optional<ClassMap> get_classes(const std::string &filename, bool pedantic)
{
    std::unordered_set<std::string> already_read;
    std::vector<std::string> to_read{filename};
    auto classes = get_builtins();

    while (to_read.size() > 0) {
        auto new_file = to_read.back();
        to_read.pop_back();

        if (already_read.count(new_file) != 0) {
            continue;
        }

        auto file_pair = parse_file(new_file, pedantic);
        if (not file_pair) {
            return std::nullopt;
        }

        auto [new_classes, included_files] = *file_pair;
        for (auto &class_info: new_classes) {
            if (classes.count(class_info.first)) {
                std::cerr << "Error! Class \"" << class_info.first
                          << "\" defined multiple times!\n";
                return std::nullopt;
            }
        }
        classes.insert(new_classes.begin(), new_classes.end());

        auto directory = new_file.substr(0, new_file.find_last_of("/\\") + 1);
        for (auto &file: included_files) {
            to_read.push_back(directory + file);
        }
        already_read.insert(new_file);
    }

    return classes;
}
