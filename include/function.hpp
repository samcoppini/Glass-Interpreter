#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "command.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

class Class;
class Instance;
class Variable;

class Function {
    protected:
        CommandList commands;
        std::shared_ptr<Instance> cur_obj;

    public:
        Function(CommandList &commands, std::shared_ptr<Instance> cur_obj);
        bool execute(std::map<std::string, Class> &classes, std::vector<Variable> &stack, std::map<std::string, Variable> &globals);
};

#endif
