#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "command.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

class Class;
class Instance;
class InstanceManager;
class Variable;

class Function {
    private:
        CommandList *commands;
        Instance *cur_obj;

    public:
        Function(CommandList &commands, Instance *cur_obj);
        void move_instance(Instance *old_insts, Instance *new_insts);
        Instance *get_obj() const;
        bool execute(InstanceManager &manager,
                     std::map<std::string, Class> &classes,
                     std::vector<Variable> &stack,
                     std::map<std::string, Variable> &globals);
};

std::optional<Variable> pop_stack(std::vector<Variable> &stack);

#endif
