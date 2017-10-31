#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <variant>
#include <vector>

enum class Builtin: int;

// An enumeration of all the different types of Glass commands
enum class CommandType {
    // Simple commands that map one-to-one to basic glass commands
    AssignClass, // !
    AssignSelf,  // $
    AssignValue, // =
    DupElement,  // (number)
    ExecuteFunc, // ?
    GetFunction, // .
    GetValue,    // *
    LoopBegin,   // /(name)
    LoopEnd,     // \ (must be matched with a LoopBegin command)
    PopStack,    // ,
    PushName,    // (name)
    PushNumber,  // <number>
    PushString,  // "str in quotes"
    Return,      // ^

    // A command representing one of the several built-in functions
    BuiltinFunction,

    // Optimized commands that are made up of several different commands
    FuncCall, // (objectName)(funcName).?
    NewInst,  // (objectName)(className)!

    // Does nothing, generated while optimizing and removed quickly afterwards
    Nop,
};

class Command {
    private:
        CommandType type;
        std::variant<std::string, double, Builtin> data;
        std::variant<std::size_t, std::string> extra_data;

        std::string file_name;
        int line, col;

    public:
        Command(Builtin builtin_type);

        Command(CommandType type, const std::string &file_name, int line,
                int col);

        Command(CommandType type, double dval, const std::string &file_name,
                int line, int col);

        Command(CommandType type, const std::string &sval,
                const std::string &file_name, int line, int col);

        Command(CommandType type, const std::string &sval, std::size_t jump,
                const std::string &file_name, int line, int col);

        Command(CommandType type, const std::string &oname,
                const std::string &fname, const std::string &file_name,
                int line, int col);

        void set_jump(std::size_t new_jump);

        CommandType get_type() const;
        Builtin get_builtin() const;
        double get_number() const;
        std::string get_string() const;
        std::size_t get_jump() const;
        std::string get_additional_name() const;
        std::string get_file_name() const;
        int get_line() const;
        int get_col() const;
};

using CommandList = std::vector<Command>;

#endif
