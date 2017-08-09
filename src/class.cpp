#include "class.hpp"

// Adds a function to a class, unless a function with the same name already
// exists within the class. Returns whether or not the class already had
// a function with the name
bool Class::add_function(const std::string &name, const CommandList &commands) {
    if (has_function(name)) {
        return true;
    }

    functions[name] = commands;
    return false;
}

// Returns whether the class has a function with a given name
bool Class::has_function(const std::string &name) const {
    return functions.count(name);
}

// Returns the class method with the given name
CommandList &Class::get_function(const std::string &name) {
    return functions.at(name);
}

// Returns all of the functions in the class
const std::map<std::string, CommandList> &Class::get_functions() const {
    return functions;
}
