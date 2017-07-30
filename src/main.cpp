#include "parse.hpp"

#include <cmath>
#include <fstream>
#include <iostream>

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
        }
    }
    std::cout << "\n";
}

void print_classes(const std::map<std::string, Class> &classes) {
    for (const auto &class_pair: classes) {
        std::cout << "{";
        if (class_pair.first.size() == 1) {
            std::cout << class_pair.first;
        } else {
            std::cout << "(" << class_pair.first << ")";
        }
        if (class_pair.second.functions.size() == 0) {
            std::cout << "}";
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

int main(int argc, char *argv[]) {
    std::string filename;
    bool format_code = false;

    for (int i = 1; i < argc; i++) {
        std::string arg{argv[i]};
        if (arg == "-f" or arg == "--format") {
            format_code = true;
        } else {
            filename = arg;
        }
    }

    if (filename == "") {
        std::cerr << "Please provide a Glass file to use the interpreter!\n";
        return 1;
    }

    std::ifstream file{argv[1]};
    if (not file.is_open()) {
        std::cerr << "Unable to open \"" << argv[1] << "\".\n";
        return 1;
    }

    auto classes = get_classes(file);
    if (not classes) {
        return 1;
    } else if (classes->count("M") == 0) {
        std::cerr << "Error! Class \"M\" is not defined!\n";
        return 1;
    } else if (format_code) {
        print_classes(*classes);
    }

    return 0;
}
