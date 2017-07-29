#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Please provide a Glass file to use the interpreter!\n";
        return 1;
    }

    std::ifstream file{argv[1]};
    if (not file.is_open()) {
        std::cerr << "Unable to open \"" << argv[1] << "\".\n";
        return 1;
    }

    return 0;
}
