#ifndef CLASS_HPP
#define CLASS_HPP

#include "command.hpp"

#include <map>

class Class {
    public:
        std::map<std::string, CommandList> functions;
};

#endif
