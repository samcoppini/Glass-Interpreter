#ifndef CLASS_HPP
#define CLASS_HPP

#include "command.hpp"

#include <unordered_map>

class Class;

using FuncMap = std::unordered_map<std::string, CommandList>;
using ClassMap = std::unordered_map<std::string, Class>;

class Class {
    friend bool check_inheritance(const ClassMap &);
    private:
        FuncMap functions;
        std::vector<std::string> parents;
        std::string name;

    public:
        Class(const std::string &name);

        bool add_function(const std::string &name, const CommandList &commands);
        bool add_parent(const std::string &class_name);
        bool has_function(const std::string &name) const;
        CommandList &get_function(const std::string &name);
        void handle_inheritance(ClassMap &classes);
        const FuncMap &get_functions() const;
        const std::vector<std::string> &get_parents() const;
        const std::string &get_name() const;
};

#endif
