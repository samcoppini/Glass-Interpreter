#include "file.hpp"

#include <cassert>

File::File(const std::string &file_name):
file_name(file_name), file(file_name), last_char('\0'), line(1), col(0),
last_is_next(false) {
}

void File::advance_character(char c) {
    if (last_char == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    last_char = c;
}

// Returns whether the file was successfully opened
bool File::is_open() const {
    return file.is_open();
}

// Returns the line of the last character read
int File::get_line() const {
    return line;
}

// Returns the column of the last character read
int File::get_col() const {
    return col;
}

// Returns the name of the file
std::string File::get_name() const {
    return file_name;
}

// Returns whether we've hit the end of the file yet
bool File::eof() const {
    return file.eof();
}

// Gets the next character in the file, and returns whether there's still more
// to read from the file
bool File::get(char &c) {
    if (last_is_next) {
        c = last_char;
        last_is_next = false;
    } else {
        file.get(c);
        advance_character(c);
    }
    return not file.eof();
}

// Goes back a character in the file. Cannot be done twice in a row without
// a get() in between the two unget() calls
void File::unget() {
    assert(not last_is_next);
    last_is_next = true;
}
