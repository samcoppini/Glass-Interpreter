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
    BuiltinFunction,
    FuncCall,
    Nop,
};

class Command {
    private:
        CommandType type;
        std::variant<std::string, double, Builtin> data;
        std::variant<std::size_t, std::string> extra_data;

    public:
        Command(CommandType type);
        Command(Builtin builtin_type);
        Command(CommandType type, double dval);
        Command(CommandType type, const std::string &sval);
        Command(CommandType type, const std::string &sval, std::size_t jump);
        Command(const std::string &oname, const std::string &fname);

        void set_jump(std::size_t new_jump);

        CommandType get_type() const;
        Builtin get_builtin() const;
        double get_number() const;
        std::string get_string() const;
        std::size_t get_jump() const;
        std::string get_func_name() const;
};

using CommandList = std::vector<Command>;

#endif
