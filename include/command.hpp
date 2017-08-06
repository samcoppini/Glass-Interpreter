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
    private:
        CommandType type;
        std::variant<std::string, double, Builtin> data;
        unsigned jump_loc;

    public:
        Command(CommandType type);
        Command(Builtin builtin_type);
        Command(CommandType type, double dval);
        Command(CommandType type, const std::string &sval);
        Command(CommandType type, const std::string &sval, unsigned jump);

        void set_jump(unsigned new_jump);

        CommandType get_type() const;
        Builtin get_builtin() const;
        double get_number() const;
        std::string get_string() const;
        unsigned get_jump() const;
};

using CommandList = std::vector<Command>;

#endif
