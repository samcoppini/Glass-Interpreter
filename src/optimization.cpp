#include "optimization.hpp"

#include <stack>

// Performs an optimization pass on a sequence of commands
void optimize_commands(CommandList &commands) {
    // The sequence of commands we're looking for to replace
    const std::array<CommandType, 4> to_match {{
        CommandType::PushName, CommandType::PushName,
        CommandType::GetFunction, CommandType::ExecuteFunc
    }};

    for (size_t i = to_match.size() - 1; i < commands.size(); i++) {
        bool is_match = true;
        for (size_t j = 0; j < to_match.size(); j++) {
            if (commands[i - j].get_type() != to_match[to_match.size() - j - 1]) {
                is_match = false;
                break;
            }
        }
        if (is_match) {
            // If we matched the sequence of commands, replace the four
            // commands with a single CommandType::FuncCall
            commands[i - 0] = {commands[i - 3].get_string(), commands[i - 2].get_string()};
            commands[i - 1] = {CommandType::Nop};
            commands[i - 2] = {CommandType::Nop};
            commands[i - 3] = {CommandType::Nop};
            i += to_match.size() - 1;
        }
    }

    // After making our optimization pass, we need to remove the NOP commands
    // that we replaced certain commands with

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

void optimize_class(Class &to_optimize) {
    for (auto &func_info: to_optimize.get_functions()) {
        optimize_commands(to_optimize.get_function(func_info.first));
    }
}

void optimize_classes(std::map<std::string, Class> &classes) {
    for (auto &class_info: classes) {
        optimize_class(class_info.second);
    }
}
