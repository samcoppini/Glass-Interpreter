#include "command.hpp"

Command::Command(CommandType type): type(type) {
}

Command::Command(Builtin builtin_type):
type(CommandType::BuiltinFunction), data(builtin_type) {
}

Command::Command(CommandType type, double dval): type(type), data(dval) {
}

Command::Command(CommandType type, const std::string &sval): type(type), data(sval) {
}

Command::Command(CommandType type, const std::string &sval, std::size_t jump):
type(type), data(sval), extra_data(jump) {
}

Command::Command(CommandType type, const std::string &oname, const std::string &fname):
type(type), data(oname), extra_data(fname) {
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
