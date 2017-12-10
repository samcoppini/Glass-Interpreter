#include <functional>
#include <iostream>
#include <unordered_set>

#include "class.hpp"

Class::Class(const std::string &name): name(name) {
}

// Adds a function to a class, unless a function with the same name already
// exists within the class. Returns whether or not the class already had
// a function with the name
bool Class::add_function(const std::string &name, const CommandList &commands) {
    if (has_function(name)) {
        return true;
    }

    functions[name] = commands;
    return false;
}

// Adds a parent to a class, unless the class already inherits from the given
// class. Returns if the class already inherits from the given class
bool Class::add_parent(const std::string &class_name) {
    for (const auto &parent: parents) {
        if (parent == class_name) {
            return true;
        }
    }
    parents.push_back(class_name);
    return false;
}

// Returns whether the class has a function with a given name
bool Class::has_function(const std::string &name) const {
    return functions.count(name);
}

// Returns the class method with the given name
CommandList &Class::get_function(const std::string &name) {
    return functions.at(name);
}

// Handles inheritance, adding functions to the class that are inherited from
// its parent classes, and adjusts the constructor to call the parent classes'
// constructors first
void Class::handle_inheritance(ClassMap &classes) {
    // As we may have to rename functions when inheriting constructors, so we
    // have to get all the unique function names so as to prevent a constructor
    // being given the same name as an existing function
    std::unordered_set<std::string> function_names;
    for (const auto &class_info: classes) {
        for (const auto &func_info: class_info.second.get_functions()) {
            function_names.insert(func_info.first);
        }
    }

    // Go through the parents, and copy the parents' functions to the child
    for (const auto &parent: parents) {
        if (classes.at(parent).parents.size() > 0) {
            classes.at(parent).handle_inheritance(classes);
        }
        for (const auto &[name, func]: classes.at(parent).functions) {
            if (name != "c__") {
                // If the function isn't a constructor, add it to the child
                add_function(name, func);
            } else {
                // Copy the parent's constructor with a changed, unique name, so
                // as to not conflict with an existing function
                auto ctor_name = "c__" + parent;
                while (function_names.count(ctor_name)) {
                    ctor_name += "_";
                }
                add_function(ctor_name, func);

                auto &ctor = functions["c__"];
                if (ctor.size() < 2 or
                    ctor[0].get_type() != CommandType::PushName or
                    ctor[0].get_string() != "_t" or
                    ctor[1].get_type() != CommandType::AssignSelf)
                {
                    // Add two instructions to the beginning, assigning a self
                    // pointer to a temporary variable, so we can call the parent
                    // constructors on the newly-created object
                    ctor.emplace(ctor.begin(), CommandType::AssignSelf, "", 0, 0);
                    ctor.emplace(ctor.begin(), CommandType::PushName, "_t", "", 0, 0);
                }

                // Add the call to the parent constructor to the beginning of
                // the constructor, after the self-assignment to a temporary
                CommandList ctor_call {
                    {CommandType::PushName, "_t", "", 0, 0},
                    {CommandType::PushName, ctor_name, "", 0, 0},
                    {CommandType::GetFunction, "", 0, 0},
                    {CommandType::ExecuteFunc, "", 0, 0}
                };
                ctor.insert(ctor.begin() + 2, ctor_call.begin(), ctor_call.end());
            }
        }
    }

    // Empty the parents list so we know we don't have anything to inherit from
    parents = {};
}

// Returns all of the functions in the class
const FuncMap &Class::get_functions() const {
    return functions;
}

// Returns all of the parents of a class
const std::vector<std::string> &Class::get_parents() const {
    return parents;
}

const std::string &Class::get_name() const {
    return name;
}

// Checks the inheritance graph of the given classes, making sure that there
// isn't a cycle in inheritance, and that every parent class actually exists.
// Returns true if there's some sort of error, and false otherwise
bool check_inheritance(const ClassMap &classes) {
    enum class State {Unvisited, Processing, Processed};
    std::unordered_map<std::string, State> class_states;

    for (const auto &class_info: classes) {
        class_states[class_info.first] = State::Unvisited;
    }

    std::function<bool(const std::string &)> check_cycle =
    [&] (const std::string &name) -> bool
    {
        if (classes.count(name) == 0) {
            std::cerr << "Error! Class \"" << name << "\" does not exist!\n";
            return true;
        }
        class_states[name] = State::Processing;
        for (const auto &parent: classes.at(name).parents) {
            if (class_states[parent] == State::Processing) {
                std::cerr << "Error! Inheritance cycle detected with class \""
                          << parent << "\".\n";
                return true;
            }
            if (class_states[parent] == State::Unvisited) {
                if (check_cycle(parent)) {
                    return true;
                }
            }
        }
        class_states[name] = State::Processed;
        return false;
    };

    for (const auto &class_info: classes) {
        if (class_states[class_info.first] == State::Unvisited) {
            if (check_cycle(class_info.first)) {
                return true;
            }
        }
    }

    return false;
}
