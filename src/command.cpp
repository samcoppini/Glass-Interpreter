#include "command.hpp"

Command::Command(CommandType type, const std::string &file_name, int line,
                 int col):
type(type), file_name(file_name), line(line), col(col) {
}

Command::Command(Builtin builtin_type):
type(CommandType::BuiltinFunction), data(builtin_type), file_name(""),
line(0), col(0) {
}

Command::Command(CommandType type, double dval, const std::string &file_name,
                 int line, int col):
type(type), data(dval), file_name(file_name), line(line), col(col) {
}

Command::Command(CommandType type, const std::string &sval,
                 const std::string &file_name, int line, int col):
type(type), data(sval), file_name(file_name), line(line), col(col) {
}

Command::Command(CommandType type, const std::string &sval, std::size_t jump,
                 const std::string &file_name, int line, int col):
type(type), data(std::pair{jump, sval}), file_name(file_name), line(line),
col(col) {
}

Command::Command(CommandType type, const std::string &name1,
                 const std::string &name2, const std::string &file_name,
                 int line, int col, int line2, int col2):
type(type), data(std::pair{name1, name2}), file_name(file_name), line(line),
col(col), line2(line2), col2(col2) {
}

CommandType Command::get_type() const {
    return type;
}

Builtin Command::get_builtin() const {
    return std::get<Builtin>(data);
}

double Command::get_number() const {
    return std::get<double>(data);
}

std::string Command::get_string() const {
    return std::get<std::string>(data);
}

void Command::set_jump(std::size_t new_jump) {
    std::get<std::pair<std::size_t, std::string>>(data).first = new_jump;
}

std::size_t Command::get_jump() const {
    return std::get<std::pair<std::size_t, std::string>>(data).first;
}

std::string Command::get_loop_var() const {
    return std::get<std::pair<std::size_t, std::string>>(data).second;
}

std::string Command::get_first_name() const {
    return std::get<std::pair<std::string, std::string>>(data).first;
}

std::string Command::get_second_name() const {
    return std::get<std::pair<std::string, std::string>>(data).second;
}

std::string Command::get_file_name() const {
    return file_name;
}

int Command::get_line() const {
    return line;
}

int Command::get_col() const {
    return col;
}

int Command::get_2nd_line() const {
    return line2;
}

int Command::get_2nd_col() const {
    return col2;
}
