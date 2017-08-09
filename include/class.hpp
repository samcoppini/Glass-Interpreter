#ifndef CLASS_HPP
#define CLASS_HPP

#include "command.hpp"

#include <map>

class Class {
    private:
        std::map<std::string, CommandList> functions;

    public:
        bool add_function(const std::string &name, const CommandList &commands);
        bool has_function(const std::string &name) const;
        CommandList &get_function(const std::string &name);
        const std::map<std::string, CommandList> &get_functions() const;
};

#endif
