#ifndef INSTANCE_MANAGER_HPP
#define INSTANCE_MANAGER_HPP

#include <map>
#include <string>
#include <vector>

class Class;
class Instance;
class Variable;

// A class for managing dynamically created instances and
// handling garbage collection
class InstanceManager {
    private:
        // All of the instances created by the manager
        std::map<Instance *, bool> instances;

        // The local variables from each currently executing function
        std::vector<std::map<std::string, Variable> *> locals;

        // The objects that are currently executing functions
        std::vector<Instance *> executing_objects;

        // The stack for the program
        std::vector<Variable> &stack;

        // The global variables
        std::map<std::string, Variable> &globals;

    public:
        InstanceManager(std::vector<Variable> &stack,
                        std::map<std::string, Variable> &globals);
        ~InstanceManager();
        void new_scope(Instance *cur_instance,
                       std::map<std::string, Variable> *new_locals);
        void unwind_scope();
        Instance *new_instance(Class &type);
        void free_everything();
        void collect_garbage();
};

#endif
