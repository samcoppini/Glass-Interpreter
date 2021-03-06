#ifndef MINIFY_HPP
#define MINIFY_HPP

#include <map>
#include <string>

class Class;

std::string get_minified_source(ClassMap &classes, std::size_t line_width,
                                bool minify_code, bool convert_code);

#endif
