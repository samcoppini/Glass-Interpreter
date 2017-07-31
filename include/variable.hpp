#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include "function.hpp"

#include <optional>
#include <memory>
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
    protected:
        VarType type;
        std::variant<double, std::string, Function, std::shared_ptr<Instance>> data;

    public:
        Variable(double dval);
        Variable(Function func);
        Variable(std::shared_ptr<Instance> inst);
        Variable(VarType type, const std::string &sval);

        std::optional<std::string> get_name() const;
        std::optional<std::string> get_string() const;
        std::optional<double> get_number() const;
        std::optional<Function> get_function() const;
        std::optional<std::shared_ptr<Instance>> get_instance() const;
        explicit operator bool() const;
        operator std::string() const;
};

#endif
