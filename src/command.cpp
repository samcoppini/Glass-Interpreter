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

Command::Command(CommandType type, const std::string &sval, unsigned jump):
type(type), data(sval), jump_loc(jump) {
}
