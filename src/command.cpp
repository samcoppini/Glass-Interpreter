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
type(type), data(sval), extra_data(jump), file_name(file_name), line(line),
col(col) {
}

Command::Command(CommandType type, const std::string &oname,
                 const std::string &fname, const std::string &file_name,
                 int line, int col):
type(type), data(oname), extra_data(fname), file_name(file_name), line(line),
col(col) {
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
    extra_data = new_jump;
}

std::size_t Command::get_jump() const {
    return std::get<std::size_t>(extra_data);
}

std::string Command::get_additional_name() const {
    return std::get<std::string>(extra_data);
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
