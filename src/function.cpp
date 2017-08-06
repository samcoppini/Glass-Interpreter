#include "builtins.hpp"
#include "class.hpp"
#include "function.hpp"
#include "instance.hpp"
#include "instanceManager.hpp"
#include "variable.hpp"

#include <cctype>
#include <iostream>

Function::Function(CommandList &commands, Instance *cur_obj):
commands(commands), cur_obj(cur_obj) {
}

Instance *Function::get_obj() const {
    return cur_obj;
}

// Executes a function, given references to the classes, stack and global
// variables. Returns whether there was an error of some sort
bool Function::execute(InstanceManager &manager,
                       std::map<std::string, Class> &classes,
                       std::vector<Variable> &stack,
                       std::map<std::string, Variable> &globals)
{
    std::map<std::string, Variable> locals;
    manager.new_scope(cur_obj, &locals);

    // Gets the value of a name from the proper context
    auto get_val = [&] (const std::string &name) -> std::optional<Variable> {
        try {
            if (name[0] == '_') {
                return locals.at(name);
            } else if (std::islower(name[0])) {
                return cur_obj->get_var(name);
            } else {
                return globals.at(name);
            }
        } catch (const std::out_of_range &e) {
            std::cerr << "Error! \"" << name << "\" is not defined!\n";
            return std::nullopt;
        }
    };

    // Sets the value of a name in the proper context
    auto set_val = [&] (const std::string &name, Variable var) {
        if (name[0] == '_') {
            locals.insert_or_assign(name, var);
        } else if (std::islower(name[0])) {
            cur_obj->set_var(name, var);
        } else {
            globals.insert_or_assign(name, var);
        }
    };

    for (unsigned i = 0; i < commands.size(); i++) {
        const auto &command = commands[i];
        switch (command.get_type()) {
            case CommandType::AssignClass: {
                auto cname = pop_stack(stack);
                auto name = pop_stack(stack);
                if (not name) {
                    return true;
                }
                auto name_str = name->get_name();
                auto cname_str = cname->get_name();
                if (not name_str) {
                    std::cerr << "Error! Cannot assign to non-name!\n";
                    return true;
                } else if (not cname_str) {
                    std::cerr << "Error! Cannot create instance of non-name!\n";
                    return true;
                }
                if (not classes.count(*cname_str)) {
                    std::cerr << "Error! Cannot instantiate non-class \""
                              << *cname_str << "\"!\n";
                    return true;
                }
                auto new_inst = manager.new_instance(classes.at(*cname_str));
                set_val(*name_str, new_inst);
                auto ctor = new_inst->get_func("c__");
                if (ctor and ctor->execute(manager, classes, stack, globals)) {
                    return true;
                }
                break;
            }

            case CommandType::AssignSelf: {
                auto name = pop_stack(stack);
                if (not name) {
                    return true;
                }
                auto name_str = name->get_name();
                if (not name_str) {
                    std::cerr << "Error! Cannot assign to non-name!\n";
                    return true;
                } else {
                    set_val(*name_str, {cur_obj});
                }
                break;
            }

            case CommandType::AssignValue: {
                auto val = pop_stack(stack);
                auto name = pop_stack(stack);
                if (not name) {
                    return true;
                }
                auto name_str = name->get_name();
                if (not name_str) {
                    std::cerr << "Error! Attempted to assign to non-name!\n";
                    return true;
                } else {
                    set_val(*name_str, *val);
                }
                break;
            }

            case CommandType::DupElement: {
                int index = stack.size() - static_cast<int>(command.get_number()) - 1;
                if (index < 0) {
                    std::cerr << "Error! Attempted to duplicate out-of-range stack value!\n";
                    return true;
                }
                stack.push_back(stack[index]);
                break;
            }

            case CommandType::ExecuteFunc: {
                auto func = pop_stack(stack);
                if (not func) {
                    return true;
                }
                auto to_run = func->get_function();
                if (not to_run) {
                    std::cerr << "Error! Attempted to execute a non-function!\n";
                    return true;
                }
                if (to_run->execute(manager, classes, stack, globals)) {
                    return true;
                }
                break;
            }

            case CommandType::GetFunction: {
                auto fname = pop_stack(stack);
                auto oname = pop_stack(stack);
                if (not oname) {
                    return true;
                }
                auto fname_str = fname->get_name();
                auto oname_str = oname->get_name();
                if (not fname_str or not oname_str) {
                    std::cerr << "Error! Attempted to retrieve value of a non-name!\n";
                    return true;
                }
                auto obj_var = get_val(*oname_str);
                if (not obj_var) {
                    return true;
                }
                auto object = obj_var->get_instance();
                if (not object) {
                    std::cerr << "Error! Cannot retrieve function from non-instance!\n";
                    return true;
                }
                auto func = (*object)->get_func(*fname_str);
                if (not func) {
                    std::cerr << "Error! \"" << *oname_str << "\" has no function \""
                              << *fname_str << "\"!\n";
                    return true;
                }
                stack.emplace_back(*func);
                break;
            }

            case CommandType::GetValue: {
                auto name = pop_stack(stack);
                if (not name) {
                    return true;
                }
                auto name_str = name->get_name();
                if (not name_str) {
                    std::cerr << "Error! Cannot retrieve value of non-name!\n";
                    return true;
                }
                auto val = get_val(*name_str);
                if (not val) {
                    return true;
                }
                stack.push_back(*val);
                break;
            }

            case CommandType::LoopBegin: {
                auto val = get_val(command.get_string());
                if (not val) {
                    return true;
                } else if (not *val) {
                    i = command.get_jump();
                }
                break;
            }

            case CommandType::LoopEnd:
                i = command.get_jump() - 1;
                break;

            case CommandType::PopStack:
                if (not pop_stack(stack)) {
                    return true;
                }
                break;

            case CommandType::PushName:
                stack.emplace_back(VarType::Name, command.get_string());
                break;

            case CommandType::PushNumber:
                stack.emplace_back(command.get_number());
                break;

            case CommandType::PushString:
                stack.emplace_back(VarType::String, command.get_string());
                break;

            case CommandType::Return:
                manager.unwind_scope();
                return false;

            case CommandType::BuiltinFunction:
                if (handle_builtin(command.get_builtin(), stack, globals)) {
                    return true;
                }
                break;
        }
    }

    manager.unwind_scope();
    return false;
}

// Pops the stack, returning the value of the former top, unless the stack
// was empty, in which case it returns std::nullopt
std::optional<Variable> pop_stack(std::vector<Variable> &stack) {
    if (stack.size() == 0) {
        std::cerr << "Error! Attempted to pop empty stack!\n";
        return std::nullopt;
    } else {
        auto back_val = stack.back();
        stack.pop_back();
        return back_val;
    }
}
