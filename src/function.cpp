#include "builtins.hpp"
#include "class.hpp"
#include "function.hpp"
#include "instance.hpp"
#include "instanceManager.hpp"
#include "variable.hpp"

#include <cassert>
#include <cctype>
#include <cstddef>
#include <iostream>

void runtime_error(const Command &command, const std::string &err) {
    std::cerr << "Error in " << command.get_file_name() << ", line "
              << command.get_line() << ", col "
              << command.get_col() << ":\n" << err << "\n";
}

Function::Function(CommandList &commands, Instance *cur_obj):
commands(&commands), cur_obj(cur_obj) {
}

Instance *Function::get_obj() const {
    return cur_obj;
}

// If the array with the instances is moved, this function updates the Instance
// pointer in the function to be in the right position
void Function::move_instance(Instance *old_insts, Instance *new_insts) {
    std::ptrdiff_t index = cur_obj - old_insts;
    cur_obj = &new_insts[index];
}

// Executes a function, given references to the classes, stack and global
// variables. Returns whether there was an error of some sort
bool Function::execute(InstanceManager &manager,
                       std::map<std::string, Class> &classes,
                       std::vector<Variable> &stack,
                       std::map<std::string, Variable> &globals)
{
    std::map<std::string, Variable> locals;
    manager.new_scope(this, &locals);

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

    for (std::size_t i = 0; i < commands->size(); i++) {
        const auto &command = commands->at(i);
        switch (command.get_type()) {
            case CommandType::AssignClass: {
                auto cname = pop_stack(stack);
                auto name = pop_stack(stack);
                if (not name) {
                    runtime_error(command, "Attempted to pop empty stack.");
                    return true;
                }
                auto name_str = name->get_name();
                auto cname_str = cname->get_name();
                if (not name_str) {
                    runtime_error(command, "Cannot assign to non-name.");
                    return true;
                } else if (not cname_str) {
                    runtime_error(command,
                                  "Cannot create instance of non-name.");
                    return true;
                }
                if (not classes.count(*cname_str)) {
                    runtime_error(command, "Cannot instantiate non-class \""
                                           + *cname_str + "\".");
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
                    runtime_error(command, "Attempted to pop empty stack.");
                    return true;
                }
                auto name_str = name->get_name();
                if (not name_str) {
                    runtime_error(command, "Cannot assign to non-name.");
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
                    runtime_error(command, "Attempted to pop empty stack.");
                    return true;
                }
                auto name_str = name->get_name();
                if (not name_str) {
                    runtime_error(command, "Cannot assign to non-name.");
                    return true;
                } else {
                    set_val(*name_str, *val);
                }
                break;
            }

            case CommandType::DupElement: {
                auto dup = static_cast<std::size_t>(command.get_number());
                if (dup >= stack.size()) {
                    runtime_error(command,
                                  "Cannot duplicate out-of-range stack value.");
                    return true;
                }
                stack.push_back(stack[stack.size() - dup - 1]);
                break;
            }

            case CommandType::ExecuteFunc: {
                auto func = pop_stack(stack);
                if (not func) {
                    runtime_error(command, "Attempted to pop empty stack.");
                    return true;
                }
                auto to_run = func->get_function();
                if (not to_run) {
                    runtime_error(command, "Cannot execute a non-function.");
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
                    runtime_error(command, "Attempted to pop empty stack.");
                    return true;
                }
                auto fname_str = fname->get_name();
                auto oname_str = oname->get_name();
                if (not fname_str or not oname_str) {
                    runtime_error(command,
                                  "Cannot retrieve value of a non-name.");
                    return true;
                }
                auto obj_var = get_val(*oname_str);
                if (not obj_var) {
                    runtime_error(command, "\"" + *oname_str
                                           + "\" is not defined.");
                    return true;
                }
                auto object = obj_var->get_instance();
                if (not object) {
                    runtime_error(command,
                                  "Cannot retrieve function from non-instance.");
                    return true;
                }
                auto func = (*object)->get_func(*fname_str);
                if (not func) {
                    runtime_error(command, *oname_str + " has no function "
                                           + *fname_str + ".");
                    return true;
                }
                stack.emplace_back(*func);
                break;
            }

            case CommandType::GetValue: {
                auto name = pop_stack(stack);
                if (not name) {
                    runtime_error(command, "Attempted to pop empty stack.");
                    return true;
                }
                auto name_str = name->get_name();
                if (not name_str) {
                    runtime_error(command, "Cannot retrieve value of non-name.");
                    return true;
                }
                auto val = get_val(*name_str);
                if (not val) {
                    runtime_error(command, "\"" + *name_str + "\" is not defined.");
                    return true;
                }
                stack.push_back(*val);
                break;
            }

            case CommandType::LoopBegin: {
                auto val = get_val(command.get_loop_var());
                if (not val) {
                    runtime_error(command, "\"" + command.get_string()
                                           + "\" is not defined.");
                    return true;
                } else if (not *val) {
                    i = command.get_jump();
                }
                break;
            }

            case CommandType::LoopEnd: {
                auto val = get_val(command.get_loop_var());
                // We don't need to check if this variable is defined, because
                // the matching LoopBegin command would've failed if it wasn't
                if (*val) {
                    i = command.get_jump();
                }
                break;
            }

            case CommandType::PopStack:
                if (not pop_stack(stack)) {
                    runtime_error(command, "Attempted to pop empty stack.");
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

            case CommandType::FuncCall: {
                auto oname = command.get_first_name();
                auto fname = command.get_second_name();

                auto obj_var = get_val(oname);
                if (not obj_var) {
                    runtime_error(command, "\"" + oname + "\" is not defined.");
                    return true;
                }
                auto object = obj_var->get_instance();
                if (not object) {
                    runtime_error(command,
                                  "Cannot retrieve function from non-instance.");
                    return true;
                }
                auto func = (*object)->get_func(fname);
                if (not func) {
                    runtime_error(command, oname + " has no function " + fname
                                           + ".");
                    return true;
                }
                if (func->execute(manager, classes, stack, globals)) {
                    return true;
                }
                break;
            }

            case CommandType::NewInst: {
                auto oname = command.get_first_name();
                auto cname = command.get_second_name();
                if (not classes.count(cname)) {
                    runtime_error(command, "Cannot instantiate non-class "
                                           + cname + ".");
                    return true;
                }
                auto new_inst = manager.new_instance(classes.at(cname));
                set_val(oname, new_inst);
                auto ctor = new_inst->get_func("c__");
                if (ctor and ctor->execute(manager, classes, stack, globals)) {
                    return true;
                }
                break;
            }

            case CommandType::Nop:
                assert(false);
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
        return std::nullopt;
    } else {
        auto back_val = stack.back();
        stack.pop_back();
        return back_val;
    }
}
