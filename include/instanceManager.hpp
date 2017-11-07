#ifndef INSTANCE_MANAGER_HPP
#define INSTANCE_MANAGER_HPP

#include <map>
#include <string>
#include <vector>

class Class;
class Function;
class Instance;
class Variable;

// Number of instances to initially allocate
const int NUM_STARTING_INSTANCES = 256;

// A class for managing dynamically created instances and
// handling garbage collection
class InstanceManager {
    private:
        // The local variables from each currently executing function
        std::vector<std::map<std::string, Variable> *> locals;

        // The objects that are currently executing functions
        std::vector<Function *> executing_funcs;

        // The stack for the program
        std::vector<Variable> &stack;

        // The global variables
        std::map<std::string, Variable> &globals;

        // An array of allocated instances
        Instance *instances;

        // An array telling whether each allocated instance is currently in use
        bool *instances_used;

        // The number of instances allocated in the instances array
        size_t num_instances;

        // The index of the next instance to use when getting a new instance
        size_t next_instance;

    public:
        InstanceManager(std::vector<Variable> &stack,
                        std::map<std::string, Variable> &globals);
        ~InstanceManager();
        void new_scope(Function *executing_func,
                       std::map<std::string, Variable> *new_locals);
        void unwind_scope();
        Instance *new_instance(Class &type);
        void collect_garbage();
};

#endif
