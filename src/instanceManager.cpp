#include "instanceManager.hpp"
#include "instance.hpp"

#include <cstring>
#include <iostream>
#include <queue>

InstanceManager::InstanceManager(std::vector<Variable> &stack,
                                 std::map<std::string, Variable> &globals):
stack(stack), globals(globals), num_instances(NUM_STARTING_INSTANCES), next_instance(0) {
    instances = static_cast<Instance *>(operator new[](num_instances * sizeof(Instance)));
    instances_used = new bool[num_instances]();
}

InstanceManager::~InstanceManager() {
    // Go through the array of instances, calling the destructors for any
    // instance that is currently in use
    for (size_t i = 0; i < num_instances; i++) {
        if (instances_used[i]) {
            instances[i].~Instance();
        }
    }

    delete [] instances_used;

    // Free the memory without calling any destructors
    operator delete [] (static_cast<void *>(instances));
}

// Adds the local variables from a new scope to the manager
void InstanceManager::new_scope(Function *executing_func,
                                std::map<std::string, Variable> *new_locals)
{
    executing_funcs.push_back(executing_func);
    locals.push_back(new_locals);
}

// Removes the variables from the current scope from the managed variables
void InstanceManager::unwind_scope() {
    executing_funcs.pop_back();
    locals.pop_back();
}

// Returns a pointer to a newly-allocated instance of a certain type
Instance *InstanceManager::new_instance(Class &type) {
    while (next_instance < num_instances and instances_used[next_instance]) {
        next_instance++;
    }

    if (next_instance < num_instances) {
        instances_used[next_instance] = true;
        return new (&instances[next_instance++]) Instance(type);
    } else {
        collect_garbage();
        return new_instance(type);
    }
}

// Performs a mark and swap garbage collection
void InstanceManager::collect_garbage() {
    // Sets all the instances as being unreachable
    for (size_t i = 0; i < num_instances; i++) {
        instances_used[i] = false;
    }

    // A vector of pointers to variables that have instance pointers in them,
    // kept track of in case we need to move the instances and the pointers
    // need to be updated
    std::vector<Variable *> reachable_vars;

    // The queue of instances to be marked and saved from being collected
    std::queue<Variable *> queue;

    // Add local variables with instance pointers to the queue
    for (auto &local_vars: locals) {
        for (auto &var: *local_vars) {
            if (var.second.get_type() == VarType::Function or
                var.second.get_type() == VarType::Instance)
            {
                queue.push(&var.second);
            }
        }
    }

    // Add global variables with instance pointers to the queue
    for (auto &var: globals) {
        if (var.second.get_type() == VarType::Function or
            var.second.get_type() == VarType::Instance)
        {
            queue.push(&var.second);
        }
    }

    // Add all the variables in the stack with instance pointers to the queue
    for (auto &var: stack) {
        if (var.get_type() == VarType::Function or
            var.get_type() == VarType::Instance)
        {
            queue.push(&var);
        }
    }

    // Go through the queue, marking all the variables with instances in the
    // queue as reachable, and add all the variables with instances that it
    // points to
    size_t insts_reachable = 0;
    while (not queue.empty()) {
        Variable *var = queue.front();
        queue.pop();

        // Check to see if the variable has been added to reachable_vars already
        if (not var->is_marked()) {
            // If we have not handled this variable, add it to reachable_vars,
            // mark it as already visited, then check its instance pointer to
            // see if we've already marked it as reachable
            var->set_marked(true);
            reachable_vars.push_back(var);
            std::ptrdiff_t index;
            if (var->get_type() == VarType::Instance) {
                index = *(var->get_instance()) - instances;
            } else {
                index = var->get_function()->get_obj() - instances;
            }
            // If we haven't already set this instance pointer as being
            // reachable, set it as reachable, and add all of its variables
            // with instance pointers to the queue
            if (not instances_used[index]) {
                instances_used[index] = true;
                insts_reachable++;
                for (auto &var: instances[index].vars) {
                    if (var.second.get_type() == VarType::Instance or
                        var.second.get_type() == VarType::Function)
                    {
                        queue.push(&var.second);
                    }
                }
            }
        }
    }

    // Calls the destructor for every instance that isn't reachable
    for (size_t i = 0; i < num_instances; i++) {
        if (not instances_used[i]) {
            instances[i].~Instance();
        }
    }

    // If enough of the instances are still in use, we double the allocated
    // memory for the instances and move the old instances to the larger array
    if (static_cast<double>(insts_reachable) / num_instances > 0.75) {
        // Allocate more memory for the instances and the array keeping track
        // of whether they're still in use
        size_t new_num_insts = num_instances << 1;
        Instance *new_instances =  static_cast<Instance *>(
            operator new[](new_num_insts * sizeof(Instance))
        );
        bool *new_instances_used = new bool[new_num_insts];

        // Update the pointers in all the variables with instances so that
        // they point to instances in the new array
        for (auto &var: reachable_vars) {
            var->move_instance(instances, new_instances);
        }

        // Update the currently-executing functions to make sure their this
        // pointers are pointing to instances in the new array
        for (auto &func: executing_funcs) {
            func->move_instance(instances, new_instances);
        }

        // Move all of the reachable instances to the new array, with the same
        // indexes they had in the old array
        for (size_t i = 0; i < num_instances; i++) {
            if (instances_used[i]) {
                new (&new_instances[i]) Instance(std::move(instances[i]));
            }
        }

        // Copy the information regarding whether each instance is in use from
        // the old instances_used array, then zero out the rest of the new array
        std::memcpy(new_instances_used, instances_used, num_instances);
        std::memset(new_instances_used + num_instances, 0, num_instances);

        // Delete the old arrays and move the new arrays into their place
        delete [] instances_used;
        operator delete [] (static_cast<void *>(instances));
        instances_used = new_instances_used;
        instances = new_instances;
        num_instances = new_num_insts;
    }

    // Unmark all of the variables as not having been marked
    for (auto &var: reachable_vars) {
        var->set_marked(false);
    }

    next_instance = 0;
}
