#include "string-things.hpp"

#include <cctype>

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

// Returns whether a string is a valid number. Unlike the conditions under
// which stod will throw, the whole string must be a valid number, with no
// trailing characters
bool valid_number(const std::string &str) {
    bool has_digit = false;
    size_t i = 0;

    if (i < str.size() and (str.at(i) == '-' or str.at(i) == '+')) {
        i++;
    }

    while (i < str.size() and std::isdigit(str.at(i))) {
        has_digit = true;
        i++;
    }

    // Numbers after a decimal point
    if (i < str.size() and str.at(i) == '.') {
        i++;
        while (i < str.size() and std::isdigit(str.at(i))) {
            has_digit = true;
            i++;
        }
    }

    // If there hasn't been a digit by now, it's impossible for the number to
    // be valid
    if (not has_digit)
        return false;

    // Check for an exponent at the end
    if (i < str.size() and std::tolower(str.at(i)) == 'e') {
        has_digit = false;
        i++;
        if (i < str.size() and (str.at(i) == '-' or str.at(i) == '+')) {
            i++;
        }

        while (i < str.size() and std::isdigit(str.at(i))) {
            has_digit = true;
            i++;
        }

        if (not has_digit) {
            return false;
        }
    }

    // In order for the number to have been valid, we must've been able to
    // march through the entire string, so we MUST be at the end of the
    // string if it was valid
    return i == str.size();
}
