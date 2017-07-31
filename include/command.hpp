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
    LoopBegin,
    LoopEnd,
    PopStack,
    PushName,
    PushNumber,
    PushString,
    Return,
    BuiltinFunction
};

class Command {
    public:
        CommandType type;
        std::variant<std::string, double, Builtin> data;
        unsigned jump_loc;

        Command(CommandType type);
        Command(Builtin builtin_type);
        Command(CommandType type, double dval);
        Command(CommandType type, const std::string &sval);
        Command(CommandType type, const std::string &sval, unsigned jump);
};

using CommandList = std::vector<Command>;

#endif
