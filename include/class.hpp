#ifndef CLASS_HPP
#define CLASS_HPP

#include "command.hpp"

#include <map>

class Class {
    friend bool check_inheritance(const std::map<std::string, Class>&);
    private:
        std::map<std::string, CommandList> functions;
        std::vector<std::string> parents;

    public:
        bool add_function(const std::string &name, const CommandList &commands);
        bool add_parent(const std::string &class_name);
        bool has_function(const std::string &name) const;
        CommandList &get_function(const std::string &name);
        void handle_inheritance(std::map<std::string, Class> &classes);
        const std::map<std::string, CommandList> &get_functions() const;
        const std::vector<std::string> &get_parents() const;
};

#endif
