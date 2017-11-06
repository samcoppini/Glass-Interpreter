#include <cassert>
#include "variable.hpp"

Variable::Variable(double dval): type(VarType::Number), data(dval) {
}

Variable::Variable(Function func): type(VarType::Function), data(func) {
}

Variable::Variable(Instance *inst): type(VarType::Instance), data(inst) {
}

Variable::Variable(VarType type, const std::string &sval): type(type), data(sval) {
}

// Returns the name as a string if the variable holds a name, otherwise
// return nullopt
std::optional<std::string> Variable::get_name() const {
    if (type == VarType::Name) {
        return std::get<std::string>(data);
    } else {
        return std::nullopt;
    }
}

// Returns the string if the variable holds a string, otherwise return nullopt
std::optional<std::string> Variable::get_string() const {
    if (type == VarType::String) {
        return std::get<std::string>(data);
    } else {
        return std::nullopt;
    }
}

// Returns the number if the variable holds a number, otherwise return nullopt
std::optional<double> Variable::get_number() const {
    if (type == VarType::Number) {
        return std::get<double>(data);
    } else {
        return std::nullopt;
    }
}

// Returns the function if the variable holds a function, otherwise return nullopt
std::optional<Function> Variable::get_function() const {
    if (type == VarType::Function) {
        return std::get<Function>(data);
    } else {
        return std::nullopt;
    }
}

// Returns the instance if the variable holds an instance, otherwise return nullopt
std::optional<Instance *> Variable::get_instance() const {
    if (type == VarType::Instance) {
        return std::get<Instance *>(data);
    } else {
        return std::nullopt;
    }
}

// Returns the type of the variable
VarType Variable::get_type() const {
    return type;
}

Variable::operator bool() const {
    if (type == VarType::Number) {
        return std::get<double>(data) != 0.0;
    } else if (type == VarType::String) {
        return std::get<std::string>(data) != "";
    } else {
        return false;
    }
}

// Returns the string representation of a type
std::string get_type_name(VarType type) {
    switch (type) {
        case VarType::Function: return "function";
        case VarType::Instance: return "instance";
        case VarType::Name:     return "name";
        case VarType::Number:   return "number";
        case VarType::String:   return "string";
    }
    assert(false);
    return "";
}
