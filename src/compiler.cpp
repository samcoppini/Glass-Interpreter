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
    "#include <stddef.h>",
    "#include <stdlib.h>",
    "#include <stdio.h>",
    "#include <string.h>",
    "",
    "typedef char bool;",
    "#define true ((bool) 1)",
    "#define false ((bool) 0)",
    "",
    "typedef void (*(Class)[NUM_CLASS_VARS])(size_t);",
    "",
    "enum Type {",
    "\tTYPE_UNDEFINED,",
    "\tTYPE_NUM,",
    "\tTYPE_NAME,",
    "\tTYPE_STR,",
    "\tTYPE_INST,",
    "\tTYPE_FUNC",
    "};",
    "",
    "struct String {",
    "\tchar *str;",
    "\tint ref_count;",
    "};",
    "",
    "struct Val {",
    "\tunion vals {",
    "\t\tdouble dval;",
    "\t\tstruct String *sval;",
    "\t\tsize_t ival;",
    "\t} val;",
    "\tenum Name name;",
    "\tenum Type type;",
    "};",
    "",
    "struct Instance {",
    "\tClass *class;",
    "\tstruct Val vars[NUM_CLASS_VARS];",
    "};",
    "",
    "struct Val global_vars[NUM_GLOBAL_VARS];",
    "",
    "struct DynamicArray {",
    "\tvoid *elems;",
    "\tsize_t num_elems, num_allocated, el_size;",
    "} *stack, *dynamic_vars, *instances, *insts_used, *this_objs, *locals_list;",
    "",
    "void error(const char *msg) {",
    "\tfprintf(stderr, msg);",
    "\texit(1);",
    "}",
    "",
    "struct DynamicArray *new_array(size_t el_size, size_t initial_cap) {",
    "\tstruct DynamicArray *array = malloc(sizeof(*array));",
    "\tarray->elems = malloc(el_size * initial_cap);",
    "\tarray->num_allocated = initial_cap;",
    "\tarray->el_size = el_size;",
    "\tarray->num_elems = 0;",
    "\treturn array;",
    "}",
    "",
    "void array_add(struct DynamicArray *array, void *to_copy) {",
    "\tif (array->num_elems == array->num_allocated) {",
    "\t\tarray->num_allocated <<= 1;",
    "\t\tarray->elems = realloc(array->elems, array->num_allocated * array->el_size);",
    "\t}",
    "\tmemcpy(((char *) array->elems) + (array->el_size * array->num_elems++), "
    "to_copy, array->el_size);",
    "}",
    "",
    "void *array_pop(struct DynamicArray *array) {",
    "\treturn ((char *) array->elems) + (array->el_size * --array->num_elems);",
    "}",
    "",
    "void init() {",
    "\tint i;",
    "\tstack = new_array(sizeof(struct Val), 16);",
    "\tdynamic_vars = new_array(sizeof(struct Val), 4);",
    "\tinstances = new_array(sizeof(struct Instance), 256);",
    "\tinsts_used = new_array(sizeof(bool), instances->num_allocated);",
    "\tthis_objs = new_array(sizeof(size_t), 16);",
    "\tlocals_list = new_array(sizeof(struct Val *), 16);",
    "\tfor (i = 0; i < insts_used->num_allocated; i++) {",
    "\t\t((bool *) insts_used->elems)[i] = false;",
    "\t}",
    "}",
    "",
    "void stack_push(struct Val *val) {",
    "\tarray_add(stack, val);",
    "}",
    "",
    "struct Val stack_pop() {",
    "\tif (stack->num_elems == 0) {",
    "\t\terror(\"Error! Tried to pop an empty stack!\\n\");",
    "\t}",
    "\treturn *((struct Val *) array_pop(stack));",
    "}",
    "",
    "struct String *copy_str(const char *str) {",
    "\tstruct String *new_str = malloc(sizeof(*new_str));",
    "\tnew_str->str = strcpy(malloc(strlen(str) + 1), str);",
    "\tnew_str->ref_count = 1;",
    "\treturn new_str;",
    "}",
    "",
    "struct String *new_str(int len) {",
    "\tstruct String *new_str = malloc(sizeof(*new_str));",
    "\tnew_str->str = malloc(len + 1);",
    "\tnew_str->ref_count = 1;",
    "\treturn new_str;",
    "}",
    "",
    "void release_str(struct String *str) {",
    "\tstr->ref_count--;",
    "\tif (str->ref_count == 0) {",
    "\t\tfree(str->str);",
    "\t\tfree(str);",
    "\t}",
    "}",
    "",
    "void cleanup() {",
    "\tint i, j;",
    "\tfor (i = 0; i < stack->num_elems; i++) {",
    "\t\tif (((struct Val *) stack->elems)[i].type == TYPE_STR)",
    "\t\t\trelease_str(((struct Val *) stack->elems)[i].val.sval);",
    "\t}",
    "\tfor (i = 0; i < dynamic_vars->num_elems; i++) {",
    "\t\tif (((struct Val *) dynamic_vars->elems)[i].type == TYPE_STR)",
    "\t\t\trelease_str(((struct Val *) dynamic_vars->elems)[i].val.sval);",
    "\t}",
    "\tfor (i = 0; i < NUM_GLOBAL_VARS; i++) {",
    "\t\tif (global_vars[i].type == TYPE_STR)",
    "\t\t\trelease_str(global_vars[i].val.sval);"
    "\t}",
    "\tfor (i = 0; i < insts_used->num_allocated; i++) {",
    "\t\tif (((bool *) insts_used->elems)[i]) {",
    "\t\t\tstruct Instance *instance = ((struct Instance *) instances->elems) + i;",
    "\t\t\tfor (j = 0; j < NUM_CLASS_VARS; j++) {",
    "\t\t\t\tif (instance->vars[j].type == TYPE_STR)",
    "\t\t\t\t\trelease_str(instance->vars[j].val.sval);",
    "\t\t\t}",
    "\t\t}",
    "\t}"
    "\tfree(stack->elems);",
    "\tfree(dynamic_vars->elems);",
    "\tfree(instances->elems);",
    "\tfree(this_objs->elems);",
    "\tfree(locals_list->elems);",
    "\tfree(insts_used->elems);",
    "\tfree(stack);",
    "\tfree(dynamic_vars);",
    "\tfree(instances);",
    "\tfree(this_objs);",
    "\tfree(locals_list);",
    "\tfree(insts_used);",
    "}",
    "",
    "void enter_scope(size_t this, struct Val *locals) {",
    "\tarray_add(this_objs, &this);",
    "\tarray_add(locals_list, &locals);",
    "}",
    "",
    "void leave_scope() {",
    "\tint i;",
    "\tstruct Val *locals = *((struct Val **) array_pop(locals_list));",
    "\tfor (i = 0; i < NUM_LOCAL_VARS; i++) {",
    "\t\tif (locals[i].type == TYPE_STR)",
    "\t\t\trelease_str(locals[i].val.sval);",
    "\t}",
    "\tarray_pop(this_objs);",
    "}",
    "",
    "void do_garbage_collection() {",
    "\tstruct DynamicArray *reachable_stack = new_array(sizeof(size_t), 32);",
    "\tsize_t i, j, num_used;",
    "\tfor (i = 0; i < this_objs->num_elems; i++) {",
    "\t\tarray_add(reachable_stack, ((size_t *) this_objs->elems) + i);",
    "\t}",
    "\tfor (i = 0; i < NUM_GLOBAL_VARS; i++) {",
    "\t\tif (global_vars[i].type == TYPE_INST || global_vars[i].type == TYPE_FUNC)",
    "\t\t\tarray_add(reachable_stack, &global_vars[i].val.ival);",
    "\t}",
    "\tfor (i = 0; i < stack->num_elems; i++) {",
    "\t\tenum Type type = ((struct Val *) stack->elems)[i].type;",
    "\t\tif (type == TYPE_FUNC || type == TYPE_INST)",
    "\t\t\tarray_add(reachable_stack, &(((struct Val *) stack->elems)[i].val.ival));",
    "\t}",
    "\tfor (i = 0; i < locals_list->num_elems; i++) {",
    "\t\tstruct Val *locals = ((struct Val **) locals_list->elems)[i];",
    "\t\tfor (j = 0; j < NUM_LOCAL_VARS; j++) {",
    "\t\t\tif (locals[j].type == TYPE_FUNC || locals[j].type == TYPE_INST)",
    "\t\t\t\tarray_add(reachable_stack, &locals[j].val.ival);",
    "\t\t}",
    "\t}",
    "\tfor (i = 0; i < insts_used->num_allocated; i++) {",
    "\t\t((bool *) insts_used->elems)[i] = false;",
    "\t}",
    "\twhile (reachable_stack->num_elems > 0) {",
    "\t\tsize_t index = *((size_t *) array_pop(reachable_stack));",
    "\t\tif (!((bool *) insts_used->elems)[index]) {",
    "\t\t\tstruct Instance *inst = ((struct Instance *) instances->elems) + index;",
    "\t\t\t((bool *) insts_used->elems)[index] = true;",
    "\t\t\tfor (i = 0; i < NUM_CLASS_VARS; i++) {",
    "\t\t\t\tif (inst->vars[i].type == TYPE_INST || inst->vars[i].type == TYPE_FUNC)",
    "\t\t\t\t\tarray_add(reachable_stack, &inst->vars[i].val.ival);",
    "\t\t\t}",
    "\t\t}",
    "\t}",
    "\tnum_used = 0;",
    "\tfor (i = 0; i < insts_used->num_allocated; i++) {",
    "\t\tif (((bool *) insts_used->elems)[i]) {",
    "\t\t\tnum_used++;",
    "\t\t} else {",
    "\t\t\tstruct Instance *inst = ((struct Instance *) instances->elems) + i;",
    "\t\t\tfor (j = 0; j < NUM_CLASS_VARS; j++) {",
    "\t\t\t\tif (inst->vars[j].type == TYPE_STR) {",
    "\t\t\t\t\trelease_str(inst->vars[j].val.sval);",
    "\t\t\t\t}",
    "\t\t\t}",
    "\t\t}",
    "\t}",
    "\tif (num_used > instances->num_allocated * 0.6) {",
    "\t\tinstances->num_allocated <<= 1;",
    "\t\tinstances->elems = realloc(instances->elems,"
    "instances->num_allocated * sizeof(struct Instance));",
    "\t\tinsts_used->elems = realloc(insts_used->elems,"
    "insts_used->num_allocated * sizeof(bool));",
    "\t\tfor (i = insts_used->num_allocated, insts_used->num_allocated <<= 1;"
    " i < insts_used->num_allocated; i++) {",
    "\t\t\t((bool *) insts_used->elems)[i] = false;",
    "\t\t}",
    "\t}",
    "\tfree(reachable_stack->elems);",
    "\tfree(reachable_stack);",
    "}",
    "",
    "size_t get_free_inst_index() {",
    "\tstatic struct Instance blank_inst = {",
    "\t\tNULL, {{0.0, 0, 0}}",
    "\t};",
    "\tstatic int cur_inst = 0;",
    "\twhile (cur_inst < instances->num_allocated) {",
    "\t\tif (!((bool *) insts_used->elems)[cur_inst]) {",
    "\t\t\t((bool *) insts_used->elems)[cur_inst] = true;",
    "\t\t\tmemcpy(((struct Instance *) instances->elems) + cur_inst, "
    "&blank_inst, sizeof(struct Instance));",
    "\t\t\treturn cur_inst++;",
    "\t\t}",
    "\t\tcur_inst++;",
    "\t}",
    "\tdo_garbage_collection();",
    "\tcur_inst = 0;",
    "\twhile (((bool *) insts_used->elems)[cur_inst])",
    "\t\tcur_inst++;",
    "\tmemcpy(((struct Instance *) instances->elems) + cur_inst, &blank_inst,"
    "sizeof(struct Instance));",
    "\treturn cur_inst++;",
    "}",
    "",
    "struct Instance *get_inst(size_t index) {",
    "\treturn ((struct Instance *) instances->elems) + index;",
    "}",
    "",
    "void assign(enum Name name, size_t this, struct Val *locals, struct Val *new_val) {",
    "\tstruct Val *old_val;",
    "\tif (name < NUM_GLOBAL_VARS)",
    "\t\told_val = &global_vars[name];",
    "\telse if (name < NUM_GLOBAL_VARS + NUM_CLASS_VARS)",
    "\t\told_val = &get_inst(this)->vars[name - NUM_GLOBAL_VARS];",
    "\telse if (name < MIN_DYNAMIC_VAR)",
    "\t\told_val = &locals[name - NUM_GLOBAL_VARS - NUM_CLASS_VARS];",
    "\telse",
    "\t\told_val = &((struct Val *) dynamic_vars->elems)[name - MIN_DYNAMIC_VAR];",
    "\tif (old_val->type == TYPE_STR)",
    "\t\trelease_str(old_val->val.sval);",
    "\t*old_val = *new_val;",
    "}",
    "",
    "struct Val get(enum Name name, size_t this, struct Val *locals) {",
    "\tstruct Val val;",
    "\tif (name < NUM_GLOBAL_VARS)",
    "\t\tval = global_vars[name];",
    "\telse if (name < NUM_GLOBAL_VARS + NUM_CLASS_VARS)",
    "\t\tval = get_inst(this)->vars[name - NUM_GLOBAL_VARS];",
    "\telse if (name < MIN_DYNAMIC_VAR)",
    "\t\tval = locals[name - NUM_GLOBAL_VARS - NUM_CLASS_VARS];",
    "\telse",
    "\t\tval = ((struct Val *) dynamic_vars->elems)[name - MIN_DYNAMIC_VAR];",
    "\tif (val.type == TYPE_STR)",
    "\t\tval.val.sval->ref_count++;",
    "\treturn val;",
    "}",
    "",
    "void dup(unsigned index) {",
    "\tstruct Val *to_dup = ((struct Val *) stack->elems) + stack->num_elems - index - 1;",
    "\tif (index >= stack->num_elems)",
    "\t\terror(\"Error! Tried to duplicate out-of-bounds stack element!\\n\");",
    "\tstack_push(to_dup);",
    "\tif (to_dup->type == TYPE_STR)",
    "\t\tto_dup->val.sval->ref_count++;",
    "}",
    "",
    "int is_true(struct Val *val) {",
    "\treturn (val->type == TYPE_NUM && val->val.dval != 0.0) ||",
    "\t       (val->type == TYPE_STR && val->val.sval->str[0] != '\\0');",
    "}"
};

// Code used to implement the builtin functions
const std::map<Builtin, std::vector<std::string>> BUILTIN_IMPLS {{
    {Builtin::InputLine, {
        "int c, allocated = 32, i = 0;",
        "temp.type = TYPE_STR, temp.val.sval = new_str(allocated);",
        "while ((c = getchar()) != EOF) {",
        "\ttemp.val.sval->str[i++] = c;",
        "\tif (i == allocated) {",
        "\t\tallocated <<= 1;",
        "\t\ttemp.val.sval->str = realloc(temp.val.sval->str, allocated + 1);",
        "\t}",
        "\tif (c == '\\n')",
        "\t\tbreak;",
        "}",
        "temp.val.sval->str[i] = '\\0';",
        "stack_push(&temp);"
    }},
    {Builtin::InputChar, {
        "temp.type = TYPE_STR, temp.val.sval = new_str(2);",
        "temp.val.sval->str[0] = getchar();",
        "temp.val.sval->str[1] = '\\0';",
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
        "\terror(\"Error! Cannot subtract non-numbers!\\n\");",
        "temp.val.dval = (temp2.val.dval - temp.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathMult, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot multiply non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval * temp2.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathDiv, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot divide non-numbers!\\n\");",
        "temp.val.dval = (temp2.val.dval / temp.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathMod, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot divide non-numbers!\\n\");",
        "temp.val.dval = fmod(temp2.val.dval, temp.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathFloor, {
        "temp = stack_pop();",
        "if (temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot floor non-number!\\n\");",
        "temp.val.dval = floor(temp.val.dval);",
        "stack_push(&temp);"
    }},
    {Builtin::MathEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval == temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathNotEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval != temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathLessThan, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval > temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathLessOrEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval >= temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathGreaterThan, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval < temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::MathGreaterOrEqual, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp.type != TYPE_NUM)",
        "\terror(\"Error! Cannot compare non-numbers!\\n\");",
        "temp.val.dval = (temp.val.dval <= temp2.val.dval ? 1.0: 0.0);",
        "stack_push(&temp);"
    }},
    {Builtin::OutputStr, {
        "temp = stack_pop();",
        "if (temp.type == TYPE_STR)",
        "\tprintf(\"%s\", temp.val.sval->str);",
        "else",
        "\terror(\"Cannot output non-string!\\n\");",
        "release_str(temp.val.sval);"
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
        "temp2.val.dval = (double) strlen(temp.val.sval->str);",
        "temp2.type = TYPE_NUM;",
        "stack_push(&temp2);",
        "release_str(temp.val.sval);"
    }},
    {Builtin::StrIndex, {
        "int index;",
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp2.type != TYPE_STR)",
        "\terror(\"Error! Wrong types for string indexing!\\n\");",
        "index = (int) temp.val.dval;",
        "temp.val.sval = new_str(2);",
        "temp.val.sval->str[1] = '\\0';",
        "temp.val.sval->str[0] = temp2.val.sval->str[index];",
        "temp.type = TYPE_STR;",
        "stack_push(&temp);",
        "release_str(temp2.val.sval);"
    }},
    {Builtin::StrReplace, {
        "struct Val temp3;",
        "temp = stack_pop(), temp2 = stack_pop(), temp3 = stack_pop();",
        "if (temp.type != TYPE_STR || temp2.type != TYPE_NUM || temp3.type != TYPE_STR)",
        "\terror(\"Wrong types for string replace operation!\\n\");",
        "if (temp3.val.sval->ref_count > 1) {",
        "\trelease_str(temp3.val.sval);",
        "\ttemp3.val.sval = copy_str(temp3.val.sval->str);",
        "}",
        "temp3.val.sval->str[(int) temp2.val.dval] = temp.val.sval->str[0];",
        "release_str(temp.val.sval);",
        "stack_push(&temp3);"
    }},
    {Builtin::StrConcatenate, {
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_STR || temp2.type != TYPE_STR)",
        "\terror(\"Error! Cannot concatenate non-strings!\\n\");",
        "if (temp2.val.sval->ref_count == 1) {",
        "\ttemp2.val.sval->str = realloc(temp2.val.sval->str, "
        "strlen(temp2.val.sval->str) + strlen(temp.val.sval->str) + 1);",
        "} else {",
        "\tstruct String *old_str = temp2.val.sval;",
        "\ttemp2.val.sval = new_str(strlen(old_str->str) +"
        "strlen(temp.val.sval->str) + 1);",
        "\tstrcpy(temp2.val.sval->str, old_str->str);",
        "\trelease_str(old_str);",
        "}",
        "strcat(temp2.val.sval->str, temp.val.sval->str);",
        "stack_push(&temp2);",
        "release_str(temp.val.sval);"
    }},
    {Builtin::StrSplit, {
        "unsigned index;",
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_NUM || temp2.type != TYPE_STR)",
        "\terror(\"Wrong types for string split operation!\\n\");",
        "index = (unsigned) temp.val.dval;",
        "temp.val.sval = new_str(strlen(temp2.val.sval->str) - index);",
        "strcpy(temp.val.sval->str, temp2.val.sval->str + index);",
        "if (temp2.val.sval->ref_count > 1) {",
        "\tstruct String *old_str = temp2.val.sval;",
        "\ttemp2.val.sval = copy_str(old_str->str);",
        "\trelease_str(old_str);",
        "}",
        "temp2.val.sval->str[index] = '\\0';",
        "temp.type = TYPE_STR;",
        "stack_push(&temp2);",
        "stack_push(&temp);"
    }},
    {Builtin::StrEqual, {
        "double result;",
        "temp = stack_pop(), temp2 = stack_pop();",
        "if (temp.type != TYPE_STR || temp2.type != TYPE_STR)",
        "\terror(\"Error! Cannot compare non-strings!\\n\");",
        "result = (strcmp(temp.val.sval->str, temp2.val.sval->str) == 0 ? 1.0: 0.0);",
        "release_str(temp.val.sval);",
        "release_str(temp2.val.sval);",
        "temp.type = TYPE_NUM;",
        "temp.val.dval = result;",
        "stack_push(&temp);"
    }},
    {Builtin::StrNumtoChar, {
        "temp = stack_pop();",
        "if (temp.type != TYPE_NUM)",
        "\terror(\"Cannot convert non-number to string!\\n\");",
        "temp2.val.sval = new_str(2);",
        "temp2.val.sval->str[0] = (char) temp.val.dval;",
        "temp2.val.sval->str[1] = '\\0';",
        "temp2.type = TYPE_STR;",
        "stack_push(&temp2);"
    }},
    {Builtin::StrChartoNum, {
        "temp = stack_pop();",
        "if (temp.type != TYPE_STR)",
        "\terror(\"Cannot convert non-string to number!\\n\");",
        "temp2.val.dval = (double) temp.val.sval->str[0];",
        "temp2.type = TYPE_NUM;",
        "stack_push(&temp2);",
        "release_str(temp.val.sval);"
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
                file << "F_" << class_name << "_" << func.first << "(size_t)";
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
             << "size_t new_C_" << class_name << "() {\n"
             << "\tsize_t index = get_free_inst_index();\n"
             << "\tget_inst(index)->class = &C_" << class_name << ";\n";
        if (class_info.get_functions().count("c__")) {
            file << "\tF_" << class_name << "_c__(index);\n";
        }
        file << "\treturn index;\n}\n";
    }

    // Generate a table of the different classes' factory functions
    file << "\nsize_t (*(factory_funcs)[NUM_GLOBAL_VARS])() = {\n";
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
    // If the function is a builtin, just output the definition of
    // the function from BUILTIN_IMPLS and leave
    if (commands.size() == 1 and
        commands[0].get_type() == CommandType::BuiltinFunction)
    {
        file << "\tstruct Val temp, temp2;\n";
        for (auto &line: BUILTIN_IMPLS.at(commands[0].get_builtin())) {
            file << "\t" << line << "\n";
        }
        return;
    }

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
         << "\tstruct Val temp, temp2;\n"
         << "\tenter_scope(this, local_vars);\n";

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
                    "if (get_inst(temp.val.ival)->class"
                    "[temp.name - NUM_GLOBAL_VARS] == NULL)",
                    "\terror(\"Cannot execute non-existent function!\\n\");",
                    "(*get_inst(temp.val.ival)->class)"
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
                    new_str += "is_true(&get_inst(this)->vars["
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
                add_lines("temp = stack_pop();",
                          "if (temp.type == TYPE_STR)",
                           "\trelease_str(temp.val.sval);");
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
                    "temp.type = TYPE_STR, temp.val.sval = copy_str(\""
                    + escape_str(command.get_string()) + "\");",
                    "stack_push(&temp);"
                );
                break;

            case CommandType::Return:
                add_lines("leave_scope();",
                          "return;");
                break;

            default:
                break;
        }
    }
    add_lines("leave_scope();");
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
                 << "(size_t this) {\n";
            output_commands(file, commands, global_indices, class_indices,
                            local_indices);
            file << "}\n";
        }
    }
}

// Outputs the main function, used to start the program
void output_main_func(std::ofstream &file) {
    file << "\nint main() {\n"
         << "\tsize_t main_obj;\n"
         << "\tinit();\n"
         << "\tatexit(cleanup);\n"
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
