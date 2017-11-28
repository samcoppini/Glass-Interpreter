#include "optimization.hpp"

#include <stack>

void optimize_instantiations(CommandList &commands, size_t i) {
    if (i < 2) {
        return;
    }

    if (commands[i - 1].get_type() == CommandType::PushName and
        commands[i - 2].get_type() == CommandType::PushName)
    {
        commands[i] = {CommandType::NewInst,
                       commands[i - 2].get_string(),
                       commands[i - 1].get_string(),
                       commands[i].get_file_name(),
                       commands[i].get_line(),
                       commands[i].get_col()};
        commands[i - 1] = {CommandType::Nop, "", 0, 0};
        commands[i - 2] = {CommandType::Nop, "", 0, 0};
    }
}

void optimize_func_executions(CommandList &commands, size_t i) {
    if (i < 3) {
        return;
    }

    if (commands[i - 1].get_type() == CommandType::GetFunction and
        commands[i - 2].get_type() == CommandType::PushName and
        commands[i - 3].get_type() == CommandType::PushName)
    {
        commands[i] = {CommandType::FuncCall,
                       commands[i - 3].get_string(),
                       commands[i - 2].get_string(),
                       commands[i - 1].get_file_name(),
                       commands[i - 1].get_line(),
                       commands[i - 1].get_col(),
                       commands[i].get_line(),
                       commands[i].get_col()};
        commands[i - 1] = {CommandType::Nop, "", 0, 0};
        commands[i - 2] = {CommandType::Nop, "", 0, 0};
        commands[i - 3] = {CommandType::Nop, "", 0, 0};
    }
}

void optimize_assignments(CommandList &commands, size_t i) {
    if (i < 2) {
        return;
    }
    if (commands[i - 1].get_type() == CommandType::DupElement and
        commands[i - 1].get_number() == 1 and
        commands[i - 2].get_type() == CommandType::PushName)
    {
        if (i + 1 < commands.size() and
            commands[i + 1].get_type() == CommandType::PopStack)
        {
            commands[i - 2] = {CommandType::AssignTo,
                               commands[i - 2].get_string(),
                               commands[i - 1].get_file_name(),
                               commands[i - 1].get_line(),
                               commands[i - 1].get_col()};
            commands[i - 1] = {CommandType::Nop, "", 0, 0};
            commands[i]     = {CommandType::Nop, "", 0, 0};
            commands[i + 1] = {CommandType::Nop, "", 0, 0};
        } else {
            commands[i - 1] = {CommandType::AssignTo,
                               commands[i - 2].get_string(),
                               commands[i - 1].get_file_name(),
                               commands[i - 1].get_line(),
                               commands[i - 1].get_col()};
            commands[i - 2] = {CommandType::DupElement, 0.0,
                               commands[i - 1].get_file_name(),
                               commands[i - 1].get_line(),
                               commands[i - 1].get_col()};
            commands[i] = {CommandType::Nop, "", 0, 0};
        }
    }
}

// Looks through the list of commands, looking for sequences of commands that
// we can collapse into one command
void collapse_commands(CommandList &commands) {
    for (size_t i = 0; i < commands.size(); i++) {
        switch (commands[i].get_type()) {
            case CommandType::AssignClass:
                optimize_instantiations(commands, i);
                break;

            case CommandType::AssignValue:
                optimize_assignments(commands, i);
                break;

            case CommandType::ExecuteFunc:
                optimize_func_executions(commands, i);
                break;

            default:
                break;
        }
    }
}

// Removes NOP commands from a sequence of commands
void remove_nops(CommandList &commands) {
    // Number of NOP commands that we have removed so far
    size_t to_replace = 0;

    // Since we may have moved the beginnings and endings of loops in the
    // code, we need to update the jump_vals with updated locations, which
    // we keep in a stack
    std::stack<std::size_t> loop_stack;

    for (size_t i = 0; i < commands.size() - to_replace;) {
        commands[i] = commands[i + to_replace];
        if (commands[i].get_type() == CommandType::Nop) {
            to_replace++;
            continue;
        } else if (commands[i].get_type() == CommandType::LoopBegin) {
            loop_stack.push(i);
        } else if (commands[i].get_type() == CommandType::LoopEnd) {
            auto loop_start = loop_stack.top();
            commands[loop_start].set_jump(i);
            commands[i].set_jump(loop_start);
            loop_stack.pop();
        }
        i++;
    }

    // Get rid of additional commands at the end of the command list
    commands.erase(commands.end() - to_replace, commands.end());
}

// Optimizes the functions in a class
void optimize_class(Class &to_optimize) {
    for (auto &func_info: to_optimize.get_functions()) {
        auto &commands = to_optimize.get_function(func_info.first);
        collapse_commands(commands);
        remove_nops(commands);
    }
}

void optimize_classes(ClassMap &classes) {
    for (auto &class_info: classes) {
        optimize_class(class_info.second);
    }
}
