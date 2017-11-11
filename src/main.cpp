#include "compiler.hpp"
#include "instance.hpp"
#include "instanceManager.hpp"
#include "minify.hpp"
#include "optimization.hpp"
#include "parse.hpp"
#include "variable.hpp"

#include <fstream>
#include <iostream>
#include <set>

void print_help(const std::string &interpreter_name) {
    std::cout << "usage: "
              << interpreter_name
              << " glass_file [args...]" << "\n";

    std::cout << "--convert   Convert glass code with extensions to standard glass\n"
              << "--compile   Convert the source to a C program\n"
              << "--help      Display this help message\n"
              << "--no-opt    Don't perform optimizations\n"
              << "--minify    Outputs a minified version of the source code\n"
              << "--pedantic  Disallow extensions to the base language of Glass\n"
              << "--width     Restricts the length of lines of minified source\n";
}

int main(int argc, char *argv[]) {
    std::string filename, out_file;
    bool minify_code = false, pedantic = false, convert_code = false,
         optimize = true;
    std::size_t width = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg{argv[i]};
        if (arg == "--minify") {
            minify_code = true;
        } else if (arg == "--pedantic") {
            pedantic = true;
        } else if (arg == "--convert") {
            convert_code = true;
        } else if (arg == "--no-opt") {
            optimize = false;
        } else if (arg == "--compile") {
            if (i + 1 == argc) {
                std::cerr << "Error! --compile argument supplied, but no output"
                          << " file was given!\n";
                return 1;
            } else if (not out_file.empty()) {
                std::cerr << "Error! Multiple compilation targets specified!\n";
                return 1;
            }
            out_file = argv[++i];
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
        } else if (filename.empty()) {
            filename = arg;
        } else {
            std::cerr << "Error! Multiple files provided!\n";
            return 1;
        }
    }

    if (filename.empty()) {
        print_help(argv[0]);
        return 1;
    } else if (width != 0 and not (minify_code or convert_code)) {
        std::cerr << "Error! Width command-line parameter specified without"
                  << " --minify or --convert!\n";
        return 1;
    } else if ((convert_code or minify_code) and not out_file.empty()) {
        std::cerr << "Error! Cannot " << (convert_code ? "convert" : "minify")
                  << " and compile code at the same time!\n";
        return 1;
    }

    auto class_pair = get_classes(filename, pedantic);
    if (not class_pair) {
        return 1;
    }
    auto [classes, included_files] = *class_pair;
    if (included_files.size() > 0) {
        std::set<std::string> already_read = {filename};
        std::string directory = filename.substr(0, filename.find_last_of("/\\") + 1);
        for (auto &file: included_files) {
            file = directory + file;
        }
        while (included_files.size() > 0) {
            auto to_read = included_files.back();
            included_files.pop_back();
            if (already_read.count(to_read) == 0) {
                class_pair = get_classes(to_read, pedantic, false);
                if (not class_pair) {
                    return 1;
                }
                auto [new_classes, new_files] = *class_pair;
                for (auto &[name, func]: new_classes) {
                    if (classes.count(name)) {
                        std::cerr << "Error! Class \"" << name
                                  << "\" defined multiple times!\n";
                        return 1;
                    }
                    classes.insert_or_assign(name, func);
                }
                directory = to_read.substr(0, to_read.find_last_of("/\\") + 1);
                for (auto &file: new_files) {
                    included_files.push_back(directory + file);
                }
                already_read.insert(to_read);
            }
        }
    }
    if (check_inheritance(classes)) {
        return 1;
    }
    if (not minify_code or convert_code) {
        for (auto &class_info: classes) {
            class_info.second.handle_inheritance(classes);
        }
    }
    if (minify_code or convert_code) {
        std::cout << get_minified_source(classes, width, minify_code, convert_code);
        return 0;
    } else if (classes.count("M") == 0) {
        std::cerr << "Error! Class \"M\" is not defined!\n";
        return 1;
    } else if (not classes.at("M").has_function("m")) {
        std::cerr << "Error! \"m\" function is not defined for class \"M\".\n";
        return 1;
    }

    if (optimize) {
        optimize_classes(classes);
    }

    if (not out_file.empty()) {
        return compile_classes(classes, out_file);
    } else {
        std::vector<Variable> stack;
        std::map<std::string, Variable> globals;
        InstanceManager manager{stack, globals};

        globals.emplace("_Main", manager.new_instance(classes.at("M")));
        auto main_obj = *globals.at("_Main").get_instance();

        if (classes.at("M").has_function("c__")) {
            auto ctor = main_obj->get_func("c__");
            if (ctor->execute(manager, classes, stack, globals)) {
                return 1;
            }
        }
        auto main_func = main_obj->get_func("m");
        if (main_func->execute(manager, classes, stack, globals)) {
            return 1;
        } else {
            return 0;
        }
    }
}
