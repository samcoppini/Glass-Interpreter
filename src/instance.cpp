#include "instance.hpp"
#include "class.hpp"

Instance::Instance(Class &type): type(type) {
}

Function Instance::get_func(const std::string &name) {
    return {type.functions[name], shared_from_this()};
}
