#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "function.hpp"
#include "variable.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

class Class;

class Instance {
    friend class InstanceManager;
    private:
        Class &type;
        std::map<std::string, Variable> vars;

    public:
        Instance(Class &type);

        void set_var(const std::string &name, Variable &var);

        std::optional<Function> get_func(const std::string &name);
        const Variable &get_var(const std::string &name) const;
};

#endif
