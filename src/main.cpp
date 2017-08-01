#include "instance.hpp"
#include "parse.hpp"
#include "variable.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
    std::string filename;
    bool debug_mode = false;

    for (int i = 1; i < argc; i++) {
        std::string arg{argv[i]};
        if (arg == "-d" or arg == "--debug") {
            debug_mode = true;
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
    } else if (not classes->at("M").functions.count("m")) {
        std::cerr << "Error! \"m\" function is not defined for class \"M\".\n";
        return 1;
    } else {
        std::vector<Variable> stack;
        std::map<std::string, Variable> globals;
        auto main_obj = std::make_shared<Instance>(classes->at("M"));
        if (classes->at("M").functions.count("c__")) {
            auto ctor = main_obj->get_func("c__");
            if (ctor.execute(*classes, stack, globals, debug_mode)) {
                return 1;
            }
        }
        auto main_func = main_obj->get_func("m");
        if (main_func.execute(*classes, stack, globals, debug_mode)) {
            return 1;
        } else {
            return 0;
        }
    }
}
