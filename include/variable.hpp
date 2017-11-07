#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include "function.hpp"

#include <optional>
#include <string>
#include <variant>

// The possible types for a Glass value
enum class VarType {
    Function,
    Instance,
    Name,
    Number,
    String
};

// A class for representing a value in Glass
class Variable {
    private:
        // The type of the variable
        VarType type;

        // Whether the variable has been marked by the garbage collector as
        // having been visited already
        bool marked = false;

        // A variant that has the actual information held by the variable
        std::variant<double, std::string, Function, Instance *> data;

    public:
        Variable(double dval);
        Variable(Function func);
        Variable(Instance *inst);
        Variable(VarType type, const std::string &sval);

        void set_marked(bool marked);
        void move_instance(Instance *old_insts, Instance *new_insts);

        std::optional<std::string> get_name() const;
        std::optional<std::string> get_string() const;
        std::optional<double> get_number() const;
        std::optional<Function> get_function() const;
        std::optional<Instance *> get_instance() const;
        VarType get_type() const;
        bool is_marked() const;
        explicit operator bool() const;
};

std::string get_type_name(VarType type);

#endif
