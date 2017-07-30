#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <variant>
#include <vector>

enum class CommandType {
    AssignClass,
    AssignSelf,
    AssignValue,
    ExecuteFunc,
    GetFunction,
    GetValue,
    DupElement,
    PopStack,
    PushName,
    PushNumber,
    PushString,
    Return,
    WhileLoop
};

class Command {
    protected:
        CommandType type;
        std::variant<std::string, double> data;
        std::vector<Command> loop_body;

    public:
        Command(CommandType type);
        Command(CommandType type, double dval);
        Command(CommandType type, const std::string &sval);
        Command(CommandType type, const std::string &sval, const std::vector<Command> &body);
};

using CommandList = std::vector<Command>;

#endif
