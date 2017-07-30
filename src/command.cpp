#include "command.hpp"

Command::Command(CommandType type): type(type) {
}

Command::Command(CommandType type, double dval): type(type), data(dval) {
}

Command::Command(CommandType type, const std::string &sval): type(type), data(sval) {
}

Command::Command(CommandType type, const std::string &sval, const std::vector<Command> &body):
type(type), data(sval), loop_body(body) {
}
