#include "instance.hpp"
#include "instanceManager.hpp"
#include "minify.hpp"
#include "parse.hpp"
#include "variable.hpp"

#include <fstream>
#include <iostream>

void print_help(const std::string &interpreter_name) {
    std::cout << "usage: "
              << interpreter_name
              << " glass_file " << "[--minify [--width w] | --help]\n";

    std::cout << "--help     Display this help message\n"
              << "--minify   Outputs a minified version of the source code\n"
              << "--width    Restricts the length of lines of minified source\n";
}

int main(int argc, char *argv[]) {
    std::string filename;
    bool minify_code = false;
    std::size_t width = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg{argv[i]};
        if (arg == "--minify") {
            minify_code = true;
        } else if (arg == "--width") {
            if (i + 1 == argc) {
                std::cerr << "Error! " << arg << " argument supplied, but no"
                          << " width was specified!\n";
                return 1;
            }
            arg = argv[++i];
            if (width != 0) {
                std::cerr << "Error! Width for minification supplied multiple"
                          << " times!\n";
                return 1;
            }
            try {
                width = std::stoul(arg);
            } catch (const std::invalid_argument &e) {
                std::cerr << "Error! Supplied width is not a valid value!\n";
                return 1;
            } catch (const std::out_of_range &e) {
                width = 0;
            }
        } else if (arg == "--help") {
            print_help(argv[0]);
            return 0;
        } else if (arg[0] == '-') {
            std::cerr << "Error! Invalid command-line argument \""
                      << arg << "\"!\n";
            return 1;
        } else {
            filename = arg;
        }
    }

    if (filename == "") {
        print_help(argv[0]);
        return 1;
    } else if (width != 0 and not minify_code) {
        std::cerr << "Error! Width command-line parameter specified without"
                  << " --minify!\n";
        return 1;
    }

    std::ifstream file{filename};
    if (not file.is_open()) {
        std::cerr << "Unable to open \"" << filename << "\".\n";
        return 1;
    }

    auto classes = get_classes(file);
    if (not classes) {
        return 1;
    } else if (classes->count("M") == 0) {
        std::cerr << "Error! Class \"M\" is not defined!\n";
        return 1;
    } else if (not classes->at("M").has_function("m")) {
        std::cerr << "Error! \"m\" function is not defined for class \"M\".\n";
        return 1;
    } else if (minify_code) {
        std::cout << get_minified_source(*classes, width);
    } else {
        std::vector<Variable> stack;
        std::map<std::string, Variable> globals;
        InstanceManager manager{stack, globals};

        globals.emplace("_Main", manager.new_instance(classes->at("M")));
        auto main_obj = *globals.at("_Main").get_instance();

        if (classes->at("M").has_function("c__")) {
            auto ctor = main_obj->get_func("c__");
            if (ctor->execute(manager, *classes, stack, globals)) {
                return 1;
            }
        }
        auto main_func = main_obj->get_func("m");
        if (main_func->execute(manager, *classes, stack, globals)) {
            return 1;
        } else {
            return 0;
        }
    }
}
