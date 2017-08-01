#include "variable.hpp"

Variable::Variable(double dval): type(VarType::Number), data(dval) {
}

Variable::Variable(Function func): type(VarType::Function), data(func) {
}

Variable::Variable(std::shared_ptr<Instance> inst): type(VarType::Instance), data(inst) {
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
std::optional<std::shared_ptr<Instance>> Variable::get_instance() const {
    if (type == VarType::Instance) {
        return std::get<std::shared_ptr<Instance>>(data);
    } else {
        return std::nullopt;
    }
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

Variable::operator std::string() const {
    if (type == VarType::Number) {
        return "number: " + std::to_string(std::get<double>(data));
    } else if (type == VarType::String) {
        return "string: \"" + std::get<std::string>(data) + "\"";
    } else if (type == VarType::Name) {
        return "name: (" + std::get<std::string>(data) + ")";
    } else if (type == VarType::Function) {
        return "function";
    } else {
        return "instance";
    }
}
