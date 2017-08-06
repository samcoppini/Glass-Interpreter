#include "instance.hpp"
#include "class.hpp"

Instance::Instance(Class &type): type(type) {
}

// Sets a variable in the instance
void Instance::set_var(const std::string &name, Variable &var) {
    vars.insert_or_assign(name, var);
}

// Returns a function matching the given name, unless there is no such method
// in the class, in which case it returns nullopt
std::optional<Function> Instance::get_func(const std::string &name) {
    if (not type.has_function(name)) {
        return std::nullopt;
    }
    return {{type.get_function(name), this}};
}

// Returns the value of the variable in the instance
const Variable &Instance::get_var(const std::string &name) const {
    return vars.at(name);
}
