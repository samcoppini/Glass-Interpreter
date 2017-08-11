#ifndef MINIFY_HPP
#define MINIFY_HPP

#include <map>
#include <string>

class Class;

std::string get_minified_source(std::map<std::string, Class> &classes,
                                std::size_t line_width);

#endif
