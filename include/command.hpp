#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <variant>
#include <vector>

enum class Builtin: int;

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
    WhileLoop,
    BuiltinFunction
};

class Command {
    public:
        CommandType type;
        std::variant<std::string, double, Builtin> data;
        std::vector<Command> loop_body;

        Command(CommandType type);
        Command(Builtin builtin_type);
        Command(CommandType type, double dval);
        Command(CommandType type, const std::string &sval);
        Command(CommandType type, const std::string &sval, const std::vector<Command> &body);
};

using CommandList = std::vector<Command>;

#endif
