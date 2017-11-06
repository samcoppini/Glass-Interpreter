#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include "function.hpp"

#include <optional>
#include <string>
#include <variant>

enum class VarType {
    Function,
    Instance,
    Name,
    Number,
    String
};

class Variable {
    private:
        VarType type;
        std::variant<double, std::string, Function, Instance *> data;

    public:
        Variable(double dval);
        Variable(Function func);
        Variable(Instance *inst);
        Variable(VarType type, const std::string &sval);

        std::optional<std::string> get_name() const;
        std::optional<std::string> get_string() const;
        std::optional<double> get_number() const;
        std::optional<Function> get_function() const;
        std::optional<Instance *> get_instance() const;
        VarType get_type() const;
        explicit operator bool() const;
};

std::string get_type_name(VarType type);

#endif
