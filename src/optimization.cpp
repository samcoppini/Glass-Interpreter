#include "optimization.hpp"

#include <functional>
#include <stack>

// A list of the different patterns of command types for optimizations we can
// match, and the corresponding functions to call to make the proper
// command replacements
const std::map<std::vector<CommandType>, std::function<void(CommandList&, size_t)>>
COMMAND_REPLACEMENTS = {
    {{CommandType::PushName, CommandType::PushName,
      CommandType::GetFunction, CommandType::ExecuteFunc},
     [] (CommandList &commands, size_t end_index) {
         commands[end_index - 0] = {CommandType::FuncCall,
                                    commands[end_index - 3].get_string(),
                                    commands[end_index - 2].get_string(),
                                    commands[end_index - 1].get_file_name(),
                                    commands[end_index - 1].get_line(),
                                    commands[end_index - 1].get_col()};
         commands[end_index - 1] = {CommandType::Nop, "", 0, 0};
         commands[end_index - 2] = {CommandType::Nop, "", 0, 0};
         commands[end_index - 3] = {CommandType::Nop, "", 0, 0};
     }},
     {{CommandType::PushName, CommandType::PushName,
       CommandType::AssignClass},
      [] (CommandList &commands, size_t end_index) {
          commands[end_index - 0] = {CommandType::NewInst,
                                     commands[end_index - 2].get_string(),
                                     commands[end_index - 1].get_string(),
                                     commands[end_index].get_file_name(),
                                     commands[end_index].get_line(),
                                     commands[end_index].get_col()};
          commands[end_index - 1] = {CommandType::Nop, "", 0, 0};
          commands[end_index - 2] = {CommandType::Nop, "", 0, 0};
      }}
};

// Looks through the list of commands, looking for sequences of commands that
// we can collapse into one command
void collapse_commands(CommandList &commands) {
    for (size_t i = 0; i < commands.size(); i++) {
        for (const auto &[to_match, replace_func]: COMMAND_REPLACEMENTS) {
            if (i < to_match.size() - 1)
                continue;

            bool found_match = true;
            for (size_t j = 0; j < to_match.size(); j++) {
                if (commands[i - j].get_type() != to_match[to_match.size() - j - 1]) {
                    found_match = false;
                    break;
                }
            }
            if (found_match) {
                replace_func(commands, i);
            }
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

void optimize_classes(std::map<std::string, Class> &classes) {
    for (auto &class_info: classes) {
        optimize_class(class_info.second);
    }
}
