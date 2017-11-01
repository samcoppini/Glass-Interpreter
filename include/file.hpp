#ifndef FILE_HPP
#define FILE_HPP

#include <fstream>

// A wrapper class around a std::ifstream that keeps track of the current
// line and column in the file
class File {
    private:
        // The name of the file being read
        std::string file_name;

        // The file being read
        std::ifstream file;

        // The last character that was, used in case of an unget() call
        char last_char;

        // The current position in the file
        int line, col;

        // Whether we've called unget() since the last get() and the next char
        // to return is last_char instead of one read from the file being read
        bool last_is_next;

        void advance_character(char c);

    public:
        File(const std::string &file_name);

        bool is_open() const;

        int get_line() const;

        int get_col() const;

        std::string get_name() const;

        bool eof() const;

        bool get(char &c);

        void unget();
};

#endif
