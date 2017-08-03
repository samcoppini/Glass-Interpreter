#include "instanceManager.hpp"
#include "instance.hpp"

#include <iostream>
#include <queue>

InstanceManager::InstanceManager(std::vector<Variable> &stack,
                                 std::map<std::string, Variable> &globals):
stack(stack), globals(globals) {
}

InstanceManager::~InstanceManager() {
    for (auto &inst: instances) {
        delete inst.first;
    }
}

// Adds the local variables from a new scope to the manager
void InstanceManager::new_scope(Instance *cur_instance,
                                std::map<std::string, Variable> *new_locals)
{
    executing_objects.push_back(cur_instance);
    locals.push_back(new_locals);
}

// Removes the variables from the current scope from the managed variables,
// and performs garbage collection
void InstanceManager::unwind_scope() {
    executing_objects.pop_back();
    locals.pop_back();
    collect_garbage();
}

// Returns a pointer to a newly-allocated instance of a certain type
Instance *InstanceManager::new_instance(Class &type) {
    auto inst_ptr = new Instance{type};
    instances[inst_ptr] = false;
    return inst_ptr;
}

// Performs a mark and swap garbage collection
void InstanceManager::collect_garbage() {
    // Sets all allocated instances as being unmarked
    for (auto &inst: instances) {
        inst.second = false;
    }

    // The queue of instances to be marked and saved from being collected
    std::queue<Instance *> queue;

    // Adds all the local
    for (auto &local_vars: locals) {
        for (auto var: *local_vars) {
            if (auto inst = var.second.get_instance()) {
                queue.push(*inst);
            } else if (auto func = var.second.get_function()) {
                queue.push(func->get_obj());
            }
        }
    }

    // Adds global variables to the queue
    for (auto &var: globals) {
        if (auto inst = var.second.get_instance()) {
            queue.push(*inst);
        } else if (auto func = var.second.get_function()) {
            queue.push(func->get_obj());
        }
    }

    // Adds variables from the stack to the queue
    for (auto &var: stack) {
        if (auto inst = var.get_instance()) {
            queue.push(*inst);
        } else if (auto func = var.get_function()) {
            queue.push(func->get_obj());
        }
    }

    // Goes through the queue, marking every instance as being safe from
    // garbage collection, and adding everything it points to to the queue
    while (not queue.empty()) {
        auto inst = queue.front();
        queue.pop();
        if (not instances[inst]) {
            for (auto &var: inst->vars) {
                if (auto new_inst = var.second.get_instance()) {
                    queue.push(*new_inst);
                } else if (auto func = var.second.get_function()) {
                    queue.push(func->get_obj());
                }
            }
            instances[inst] = true;
        }
    }

    // Deletes every instance that wasn't marked
    for (auto it = instances.begin(); it != instances.end();) {
        if (not it->second) {
            delete it->first;
            it = instances.erase(it);
        } else {
            ++it;
        }
    }
}
