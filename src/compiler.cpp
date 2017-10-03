#include "builtins.hpp"
#include "compiler.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <set>

// Code that is included in every compiled source
const std::string COMPILED_CODE_DEFS[] = {
    "#include <math.h>",
    "#include <stdlib.h>",
    "#include <stdio.h>",
    "#include <string.h>",
    "",
    "struct Instance;",
    "",
    "typedef void (*(Class)[NUM_CLASS_VARS])(struct Instance *);",
    "",
    "enum Type {",
    "    TYPE_UNDEFINED,",
    "    TYPE_NUM,",
    "    TYPE_NAME,",
    "    TYPE_STR,",
    "    TYPE_INST,",
    "    TYPE_FUNC",
    "};",
    "",
    "struct Val {",
    "    union vals {",
    "        double dval;",
    "        char *sval;",
    "        struct Instance *ival;",
    "    } val;",
    "    enum Name name;",
    "    enum Type type;",
    "};",
    "",
    "struct Instance {",
    "    Class *class;",
    "    struct Val vars[NUM_CLASS_VARS];",
    "};",
    "",
    "struct Val global_vars[NUM_GLOBAL_VARS];",
    "",
    "struct DynamicArray {",
    "    struct Val *elems;",
    "    size_t allocated, length;",
    "} stack, dynamic_vars;",
    "",
    "void error(const char *msg) {",
    "    fprintf(stderr, msg);",
    "    exit(1);",
    "}",
    "",
    "void init_stack() {",
    "    stack.elems = malloc(sizeof(struct Val) * 16);",
    "    dynamic_vars.elems = malloc(sizeof(struct Val) * 16);",
    "    stack.allocated = 16;",
    "    dynamic_vars.allocated = 16;",
    "    stack.length = 0;",
    "    dynamic_vars.length = 0;",
    "}",
    "",
    "void stack_push(struct Val *val) {",
    "    if (stack.allocated == stack.length) {",
    "        stack.allocated <<= 1;",
    "        stack.elems = realloc(stack.elems, stack.allocated * sizeof(struct Val));",
    "        if (stack.elems == NULL) {",
    "            error(\"Error! Ran out of memory!\\n\");",
    "        }",
    "    }",
    "    stack.elems[stack.length++] = *val;",
    "}",
    "",
    "struct Val stack_pop() {",
    "    if (stack.length == 0) {",
    "        error(\"Error! Tried to pop an empty stack!\\n\");",
    "    }",
    "    return stack.elems[--stack.length];",
    "}",
    "",
    "char *strdup(const char *str) {",
    "    return strcpy(malloc(strlen(str) + 1), str);",
    "}",
    "",
    "void assign(enum Name name, struct Instance *this, struct Val *locals, struct Val *new_val) {",
    "    if (name < NUM_GLOBAL_VARS)",
    "        global_vars[name] = *new_val;",
    "    else if (name < NUM_GLOBAL_VARS + NUM_CLASS_VARS)",
    "        this->vars[name - NUM_GLOBAL_VARS] = *new_val;",
    "    else if (name < MIN_DYNAMIC_VAR)",
    "        locals[name - NUM_GLOBAL_VARS - NUM_CLASS_VARS] = *new_val;",
    "    else",
    "        dynamic_vars.elems[name - MIN_DYNAMIC_VAR] = *new_val;",
    "}",
    "",
    "struct Val get(enum Name name, struct Instance *this, struct Val *locals) {",
    "    if (name < NUM_GLOBAL_VARS)",
    "        return global_vars[name];",
    "    else if (name < NUM_GLOBAL_VARS + NUM_CLASS_VARS)",
    "        return this->vars[name - NUM_GLOBAL_VARS];",
    "    else if (name < MIN_DYNAMIC_VAR)",
    "        return locals[name - NUM_GLOBAL_VARS - NUM_CLASS_VARS];",
    "    else",
    "        return dynamic_vars.elems[name - MIN_DYNAMIC_VAR];",
    "}",
    "",
    "void dup(unsigned index) {",
    "    if (index >= stack.length)",
    "        error(\"Error! Tried to duplicate out-of-bounds stack element!\\n\");",
    "    stack_push(stack.elems + stack.length - index - 1);",
    "}",
    "",
    "int is_true(struct Val *val) {",
    "    return (val->type == TYPE_NUM && val->val.dval != 0.0) ||",
    "           (val->type == TYPE_STR && strlen(val->val.sval) > 1);",
    "}"
};

// Code used to implement the builtin functions
const std::map<Builtin, std::vector<std::string>> BUILTIN_IMPLS {{
    {Builtin::InputLine, {
        "int c, allocated = 32, i = 0;"
        "temp.type = TYPE_STR, temp.val.sval = malloc(allocated + 1);",
        "while ((c = getchar()) != EOF) {",
        "\ttemp.val.sval[i++] = c;",
        "\tif (i == allocated) {",
        "\t\tallocated <<= 1;",
        "\t\ttemp.val.sval = realloc(temp.val.sval, allocated + 1);",
        "\t}",
        "\tif (c == '\\n')",
        "\t\tbreak;",
        "}",
        "temp.val.sval[i] = '\\0';",
        "stack_push(&temp);"
    }},
    {Builtin::InputChar, {
        "temp.type = TYPE_STR, temp.val.sval = malloc(2);",
        "temp.val.sval[0] = getchar();",
        "temp.val.sval[1] = '\\0';",
        "stack_push(&temp);"
    }},
    {Builtin::InputEof, {
        "temp.type = TYPE_NUM, temp.val.dval = (feof(stdin) ? 1: 0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathAdd, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot add non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval + temp2.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathSub, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot subtract non-numbers!\\n\");",
        "temp.val.dval = (temp2.val.dval - temp.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathMult, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot multiply non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval * temp2.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathDiv, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot divide non-numbers!\\n\");",
        "temp.val.dval = (temp2.val.dval / temp.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathMod, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot divide non-numbers!\\n\");",
        "temp.val.dval = fmod(temp2.val.dval, temp.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathFloor, {
        "temp = stack_pop();",
        "if (temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot floor non-number!\\n\");",
        "temp.val.dval = floor(temp.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval == temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathNotEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval != temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathLessThan, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval > temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathLessOrEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval >= temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathGreaterThan, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval < temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathGreaterOrEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "    error(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval <= temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::OutputStr, {
        "temp = stack_pop();",
        "if (temp.type == TYPE_STR)",
        "\tprintf(\"%s\", temp.val.sval);",
        "else",
        "\terror(\"Cannot output non-string!\\n\");"
    }},
    {Builtin::OutputNumber, {
        "temp = stack_pop();",
        "if (temp.type == TYPE_NUM)",
        "\tprintf(\"%g\", temp.val.dval);"
    }},
    {Builtin::StrLength, {
        "temp = stack_pop();",
        "if (temp.type != TYPE_STR)",
        "\terror(\"Error! Cannot get the length of a non-string!\\n\");",
        "temp.val.dval = (double) strlen(temp.val.sval);",
        "temp.type = TYPE_NUM;",
        "stack_push(&temp);"
    }},
    {Builtin::StrIndex, {
        "int index;",
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp2.type != TYPE_STR)",
        "\terror(\"Error! Wrong types for string indexing!\\n\");",
        "index = (int) temp.val.dval;",
        "temp.val.sval = malloc(sizeof(2));",
        "temp.val.sval[1] = '\\0';",
        "temp.val.sval[0] = temp2.val.sval[index];",
        "temp.type = TYPE_STR;",
        "stack_push(&temp);"
    }},
    {Builtin::StrReplace, {
        "struct Val temp3;",
        "temp = stack_pop(), temp2 = stack_pop(), temp3 = stack_pop();",
        "if (temp.type != TYPE_STR || temp2.type != TYPE_NUM || temp3.type != TYPE_STR)",
        "\terror(\"Wrong types for string replace operation!\\n\");",
        "temp3.val.sval[(int) temp2.val.dval] = temp.val.sval[0];",
        "stack_push(&temp3);"
    }},
    {Builtin::StrConcatenate, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_STR || temp2.type != TYPE_STR)",
        "\terror(\"Error! Cannot concatenate non-strings!\\n\");",
        "temp2.val.sval = realloc(temp2.val.sval, strlen(temp2.val.sval) + strlen(temp.val.sval) + 1);",
        "strcat(temp2.val.sval, temp.val.sval);",
        "stack_push(&temp2);"
    }},
    {Builtin::StrSplit, {
        "unsigned index;",
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp2.type != TYPE_STR)",
        "\terror(\"Wrong types for string split operation!\\n\");",
        "index = (unsigned) temp.val.dval;",
        "temp.val.sval = malloc(strlen(temp2.val.sval) - index + 1);",
        "strcpy(temp.val.sval, temp2.val.sval + index);",
        "temp2.val.sval[index] = '\\0';",
        "temp.type = TYPE_STR;",
        "stack_push(&temp2);",
        "stack_push(&temp);"
    }},
    {Builtin::StrEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_STR || temp2.type != TYPE_STR)",
        "\terror(\"Error! Cannot compare non-strings!\\n\");",
        "temp.val.dval = (strcmp(temp.val.sval, temp2.val.sval) == 0 ? 1.0: 0.0);",
        "temp.type = TYPE_NUM;",
        "stack_push(&temp);"
    }},
    {Builtin::StrNumtoChar, {
        "temp = stack_pop();",
        "if (temp.type != TYPE_NUM)",
        "\terror(\"Cannot convert non-number to string!\\n\");",
        "temp2.val.sval = malloc(2);",
        "temp2.val.sval[0] = (char) temp.val.dval;",
        "temp2.val.sval[1] = '\\0';",
        "temp2.type = TYPE_STR;",
        "stack_push(&temp2);"
    }},
    {Builtin::StrChartoNum, {
        "temp = stack_pop();",
        "if (temp.type != TYPE_STR)",
        "\terror(\"Cannot convert non-string to number!\\n\");",
        "temp2.val.dval = (double) temp.val.sval[0];",
        "temp2.type = TYPE_NUM;",
        "stack_push(&temp2);"
    }},
    {Builtin::VarNew, {
        "if (dynamic_vars.length == dynamic_vars.allocated) {",
        "\tdynamic_vars.allocated <<= 1;",
        "\tdynamic_vars.elems = realloc(dynamic_vars.elems,"
        " sizeof(struct Val) * dynamic_vars.allocated);",
        "}",
        "temp.type = TYPE_NAME, temp.name = MIN_DYNAMIC_VAR + dynamic_vars.length;",
        "dynamic_vars.elems[dynamic_vars.length++].type = TYPE_UNDEFINED;",
        "stack_push(&temp);"
    }},
    {Builtin::VarDelete, {
        "temp = stack_pop();",
        "if (temp.type != TYPE_NAME)",
        "\terror(\"Error! Cannot delete non-name!\\n\");",
        "if (temp.name < MIN_DYNAMIC_VAR)",
        "\terror(\"Cannot delete non-generated name!\\n\");",
        "dynamic_vars.elems[temp.name - MIN_DYNAMIC_VAR].type = TYPE_UNDEFINED;"
    }}
}};

// Returns the escaped version of a string
std::string escape_str(const std::string &str) {
    std::string new_str;
    for (auto &c: str) {
        if (c == '\n') {
            new_str += "\\n";
        } else {
            new_str += c;
        }
    }
    return new_str;
}

// Returns a list of the names used in the source, seperated by whether it's
// a global name, class-scope name or a local name
std::array<std::set<std::string>, 3> get_names(const
std::map<std::string, Class> &classes)
{
    std::array<std::set<std::string>, 3> vars;
    std::set<std::string> &global_vars = vars[0];
    std::set<std::string> &class_vars  = vars[1];
    std::set<std::string> &local_vars  = vars[2];

    auto add_name = [&] (const std::string &name) {
        std::set<std::string> *context;
        if (name[0] == '_') {
            context = &local_vars;
        } else if (std::islower(name[0])) {
            context = &class_vars;
        } else {
            context = &global_vars;
        }
        if (context->count(name) == 0) {
            context->insert(name);
        }
    };

    for (auto &class_info: classes) {
        for (auto &func_info: class_info.second.get_functions()) {
            for (auto &command: func_info.second) {
                if (command.get_type() == CommandType::PushName or
                    command.get_type() == CommandType::LoopBegin)
                {
                    add_name(command.get_string());
                }
            }
        }
    }

    return vars;
}

// Prints an enumeration of all the names used in the input file to the
// compiled file
void output_name_enums(std::ofstream &file,
                       const std::set<std::string> &global_vars,
                       const std::set<std::string> &class_vars,
                       const std::set<std::string> &local_vars)
{
    file << "enum Name {\n";

    for (auto &var: global_vars) {
        file << "\tN_" << var << ",\n";
    }

    for (auto &var: class_vars) {
        file << "\tN_" << var << ",\n";
    }

    for (auto &var: local_vars) {
        file << "\tN_" << var << ",\n";
    }

    file << "\tNUM_GLOBAL_VARS = " << global_vars.size() << ",\n"
         << "\tNUM_CLASS_VARS = " << class_vars.size() << ",\n"
         << "\tNUM_LOCAL_VARS = " << std::max<int>(1, local_vars.size()) << ",\n"
         << "\tMIN_DYNAMIC_VAR = NUM_GLOBAL_VARS + NUM_CLASS_VARS + NUM_LOCAL_VARS"
         << "\n};\n\n";
}

// Outputs the definitions necessary to have all of the class vtables, and
// sets up the class constructors
void output_class_defs(std::ofstream &file,
                       const std::map<std::string, Class> &classes,
                       const std::set<std::string> &global_vars,
                       const std::set<std::string> &class_vars)
{
    for (auto &[class_name, class_info]: classes) {
        if (global_vars.count(class_name) == 0) {
            // If this global name isn't a class, skip it
            continue;
        }
        // Forward declare all of the class's functions
        bool is_first = true;
        for (auto &func: class_info.get_functions()) {
            if (class_vars.count(func.first)) {
                if (is_first) {
                    file << "\nvoid ";
                    is_first = false;
                } else {
                    file << ",\n     ";
                }
                file << "F_" << class_name << "_" << func.first
                     << "(struct Instance *)";
            }
        }
        if (not is_first) {
            file << ";\n";
        }

        // Generate the class's vtable
        file << "\nClass C_" << class_name << " = {";
        is_first = true;
        for (auto var_info: class_vars) {
            if (is_first) {
                is_first = false;
                file << "\n\t";
            } else {
                file << ",\n\t";
            }
            if (class_info.get_functions().count(var_info)) {
                file << "F_" << class_name << "_" << var_info;
            } else {
                file << "NULL";
            }
        }
        file << "\n};\n\n"
        // Create a factory function for the class
             << "struct Instance *new_C_" << class_name << "() {\n"
             << "\tstruct Instance *obj = calloc(1, sizeof(struct Instance));\n"
             << "\tobj->class = &C_" << class_name << ";\n";
        if (class_info.get_functions().count("c__")) {
            file << "\tF_" << class_name << "_c__(obj);\n";
        }
        file << "\treturn obj;\n}\n";
    }

    // Generate a table of the different classes' factory functions
    file << "\nstruct Instance *(*(factory_funcs)[NUM_GLOBAL_VARS])() = {\n";
    bool is_first = true;
    for (auto &name: global_vars) {
        if (is_first) {
            file << "\t";
            is_first = false;
        } else {
            file << ",\n\t";
        }
        if (classes.count(name)) {
            file << "new_C_" << name;
        } else {
            file << "NULL";
        }
    }
    file << "\n};\n";
}

// Translates the Glass commands to C source code
void output_commands(std::ofstream &file,
                     const CommandList &commands,
                     const std::map<std::string, int> &global_indices,
                     const std::map<std::string, int> &class_indices,
                     const std::map<std::string, int> &local_indices)
{
    // Create a table for storing the local variables, all initialized
    // to be undefined at first
    file << "\tstruct Val local_vars[NUM_LOCAL_VARS] = {";
    bool is_first = true;
    for (int i = 0; i < std::max<int>(1, local_indices.size()); i++) {
        if (is_first) {
            file << "\n\t\t";
            is_first = false;
        } else {
            file << ",\n\t\t";
        }
        file << "{0.0, 0, TYPE_UNDEFINED}";
    }
    file << "\n\t};\n"
         << "\tstruct Val temp, temp2;\n";

    int tab_level = 1;

    auto print_lines = [&] (const auto &func, const auto &line, const auto &...lines) {
        for (int i = 0; i < tab_level; i++) {
            file << "\t";
        }
        file << line << "\n";
        if constexpr (sizeof...(lines) > 0) {
            func(func, lines...);
        }
    };

    auto add_lines = [&] (const auto &...args) {
        print_lines(print_lines, args...);
    };

    for (auto &command: commands) {
        if (command.get_type() == CommandType::LoopEnd) {
            tab_level--;
        }
        switch (command.get_type()) {
            case CommandType::AssignClass:
                add_lines(
                    "temp = stack_pop();",
                    "if (temp.type != TYPE_NAME || temp.name >= NUM_GLOBAL_VARS)",
                    "\terror(\"Invalid class name!\\n\");",
                    "temp2 = stack_pop();",
                    "if (temp.type != TYPE_NAME)",
                    "\terror(\"Cannot assign to non-name!\\n\");",
                    "temp.val.ival = factory_funcs[temp.name]();",
                    "temp.type = TYPE_INST;",
                    "assign(temp2.name, this, local_vars, &temp);");
                break;

            case CommandType::AssignSelf:
                add_lines(
                    "temp = stack_pop();",
                    "if (temp.type != TYPE_NAME)",
                    "\terror(\"Cannot assign to non-name!\\n\");",
                    "temp2.type = TYPE_INST, temp2.val.ival = this;",
                    "assign(temp.name, this, local_vars, &temp2);");
                break;

            case CommandType::AssignValue:
                add_lines(
                    "temp = stack_pop(), temp2 = stack_pop();",
                    "if (temp2.type != TYPE_NAME) {",
                    "\terror(\"Cannot assign to a non-name!\\n\");",
                    "}",
                    "assign(temp2.name, this, local_vars, &temp);");
                break;

            case CommandType::DupElement:
                add_lines(
                    "dup((unsigned) " +
                    std::to_string(command.get_number()) + ");");
                break;

            case CommandType::ExecuteFunc:
                add_lines(
                    "temp = stack_pop();",
                    "if (temp.type != TYPE_FUNC)",
                    "\terror(\"Cannot execute non-function!\\n\");",
                    "if (temp.val.ival->class[temp.name - NUM_GLOBAL_VARS] == NULL)",
                    "\terror(\"Cannot execute non-existent function!\\n\");",
                    "(*temp.val.ival->class)"
                    "[temp.name - NUM_GLOBAL_VARS](temp.val.ival);");
                break;

            case CommandType::GetFunction:
                add_lines(
                    "temp = stack_pop(), temp2 = stack_pop();",
                    "if (temp.type != TYPE_NAME || temp.name < NUM_GLOBAL_VARS "
                    "|| temp.name > NUM_GLOBAL_VARS + NUM_CLASS_VARS)",
                    "\terror(\"Invalid function name!\\n\");",
                    "if (temp2.type != TYPE_NAME)",
                    "\terror(\"Cannot retrieve value of non-name!\\n\");",
                    "temp2 = get(temp2.name, this, local_vars);",
                    "if (temp2.type != TYPE_INST)",
                    "\terror(\"Cannot retrieve function of non-instance!\\n\");",
                    "temp.type = TYPE_FUNC, temp.val.ival = temp2.val.ival;",
                    "stack_push(&temp);");
                break;

            case CommandType::GetValue:
                add_lines(
                    "temp = stack_pop();",
                    "if (temp.type != TYPE_NAME)",
                    "\terror(\"Cannot retrieve value of a non-name!\\n\");",
                    "temp = get(temp.name, this, local_vars);",
                    "if (temp.type == TYPE_UNDEFINED)",
                    "\terror(\"Error! Cannot retrieve undefined value!\\n\");",
                    "stack_push(&temp);");
                break;

            case CommandType::LoopBegin: {
                std::string new_str = "while (";
                if (command.get_string()[0] == '_') {
                    new_str += "is_true(&local_vars["
                               + std::to_string(local_indices.at(command.get_string()));
                } else if (std::islower(command.get_string()[0])) {
                    new_str += "is_true(&this->vars["
                               + std::to_string(class_indices.at(command.get_string()));
                } else {
                    new_str += "is_true(&global_vars["
                               + std::to_string(global_indices.at(command.get_string()));
                }
                add_lines(new_str + "])) {");
                tab_level++;
                break;
            }

            case CommandType::LoopEnd:
                add_lines("}");
                break;

            case CommandType::PopStack:
                add_lines("stack_pop();");
                break;

            case CommandType::PushName:
                add_lines("temp.name = N_" + command.get_string() + ";",
                          "temp.type = TYPE_NAME;",
                          "stack_push(&temp);");
                break;

            case CommandType::PushNumber:
                add_lines(
                    "temp.val.dval = " +
                    std::to_string(command.get_number()) + ";",
                    "temp.type = TYPE_NUM;",
                    "stack_push(&temp);");
                break;

            case CommandType::PushString:
                add_lines(
                    "temp.type = TYPE_STR, temp.val.sval = strdup(\""
                    + escape_str(command.get_string()) + "\");",
                    "stack_push(&temp);"
                );
                break;

            case CommandType::Return:
                add_lines("return;");
                break;

            case CommandType::BuiltinFunction:
                for (auto &line: BUILTIN_IMPLS.at(command.get_builtin())) {
                    add_lines(line);
                }
                break;

            default:
                break;
        }
    }
}

// Outputs all of the functions necessary for implementing the all of
// the Glass classes' methods
void output_functions(std::ofstream &file,
                      const std::map<std::string, Class> &classes,
                      const std::set<std::string> &global_vars,
                      const std::set<std::string> &class_vars,
                      const std::set<std::string> &local_vars)
{
    std::map<std::string, int> global_indices, class_indices, local_indices;
    int i = 0;
    for (auto &global_var: global_vars) {
        global_indices[global_var] = i++;
    }
    i = 0;
    for (auto &class_var: class_vars) {
        class_indices[class_var] = i++;
    }
    i = 0;
    for (auto &local_var: local_vars) {
        local_indices[local_var] = i++;
    }

    for (auto &[class_name, class_info]: classes) {
        if (not global_vars.count(class_name)) {
            continue;
        }
        for (auto &[func_name, commands]: class_info.get_functions()) {
            if (not class_vars.count(func_name) and
                not (class_name == "M" and func_name == "m")) {
                continue;
            }
            file << "\nvoid F_" << class_name << "_" << func_name
                 << "(struct Instance *this) {\n";
            output_commands(file, commands, global_indices, class_indices,
                            local_indices);
            file << "}\n";
        }
    }
}

// Outputs the main function, used to start the program
void output_main_func(std::ofstream &file) {
    file << "\nint main() {\n"
         << "\tstruct Instance *main_obj;\n"
         << "\tinit_stack();\n"
         << "\tmain_obj = new_C_M();\n"
         << "\tF_M_m(main_obj);\n"
         << "}\n";
}

// Compiles the given classes to ANSI C.
// Returns whether there was some error during compilation
bool compile_classes(const std::map<std::string, Class> &classes,
                     const std::string &file_name)
{
    std::ofstream file{file_name};
    if (not file.is_open()) {
        std::cerr << "Error! Unable to open \"" << file_name << "\"!\n";
        return true;
    }

    auto [global_vars, class_vars, local_vars] = get_names(classes);
    global_vars.insert("M");
    class_vars.insert("c__");
    output_name_enums(file, global_vars, class_vars, local_vars);

    for (auto &line: COMPILED_CODE_DEFS) {
        file << line << "\n";
    }

    output_class_defs(file, classes, global_vars, class_vars);
    output_functions(file, classes, global_vars, class_vars, local_vars);
    output_main_func(file);

    return false;
}
