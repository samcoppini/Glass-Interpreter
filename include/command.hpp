#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <variant>
#include <vector>

enum class CommandType {
    AssignClass,
    AssignSelf,
    AssignValue,
    DupElement,
    ExecuteFunc,
    GetFunction,
    GetValue,
    PopStack,
    PushName,
    PushNumber,
    PushString,
    Return,
    WhileLoop
};

class Command {
    public:
        CommandType type;
        std::variant<std::string, double> data;
        std::vector<Command> loop_body;

        Command(CommandType type);
        Command(CommandType type, double dval);
        Command(CommandType type, const std::string &sval);
        Command(CommandType type, const std::string &sval, const std::vector<Command> &body);
};

using CommandList = std::vector<Command>;

#endif
