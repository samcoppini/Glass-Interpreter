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
    AssignTo, // Equivalent to (name)(1)=,
    FuncCall, // (objectName)(funcName).?
    NewInst,  // (objectName)(className)!

    // Does nothing, generated while optimizing and removed quickly afterwards
    Nop,
};

// A class for representing a single Glass command
class Command {
    private:
        // The type of command this is
        CommandType type;

        // Variant with extra data for some specific types of commands
        std::variant<
            // Used for DupElement and PushNumber
            double,

            // Used for PushName and PushString
            std::string,

            // Used for BuiltinFunction
            Builtin,

            // Used for LoopBegin and LoopEnd
            std::pair<std::size_t, std::string>,

            // Used for FuncCall and NewInst
            std::pair<std::string, std::string>
        > data;

        // The name of the file this command was found on
        std::string file_name;

        // The position in the file the command was found on
        int line, col;

        // The secondary position of the command if it is made up of
        // multiple tokens
        int line2 = 0, col2 = 0;

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
                int line, int col, int line2 = 0, int col2 = 0);

        void set_jump(std::size_t new_jump);

        CommandType get_type() const;
        Builtin get_builtin() const;
        double get_number() const;
        const std::string &get_string() const;
        std::size_t get_jump() const;
        const std::string &get_loop_var() const;
        const std::string &get_first_name() const;
        const std::string &get_second_name() const;
        const std::string &get_file_name() const;
        int get_line() const;
        int get_col() const;
        int get_2nd_line() const;
        int get_2nd_col() const;
};

using CommandList = std::vector<Command>;

#endif
