#include "string-things.hpp"

// Reproduce a string, converting characters to the appropriate escape sequences
std::string escape_str(const std::string &str) {
    std::string new_str;
    for (const auto &c: str) {
        switch (c) {
            case '"':    new_str += "\\\""; break;
            case '\\':   new_str += "\\\\"; break;
            case '\a':   new_str += "\\a";  break;
            case '\b':   new_str += "\\b";  break;
            case '\x1b': new_str += "\\e";  break;
            case '\f':   new_str += "\\f";  break;
            case '\n':   new_str += "\\n";  break;
            case '\r':   new_str += "\\r";  break;
            case '\t':   new_str += "\\t";  break;
            case '\v':   new_str += "\\f";  break;
            default:     new_str += c;      break;
        }
    }
    return new_str;
}
