#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "function.hpp"
#include "variable.hpp"

#include <map>
#include <string>
#include <vector>

class Class;

class Instance {
    public:
        Class &type;
        std::map<std::string, Variable> vars;

        Instance(Class &type);
        Function get_func(const std::string &name);
};

#endif
