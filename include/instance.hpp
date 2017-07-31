#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "function.hpp"
#include "variable.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

class Class;

class Instance: public std::enable_shared_from_this<Instance> {
    public:
        Class &type;
        std::map<std::string, Variable> vars;

        Instance(Class &type);
        Function get_func(const std::string &name);
};

#endif
