This interpreter offers a few different extensions to the base language, and has slightly behavior compared to the reference interpreter.

# Inheritance
Every self-respecting object-oriented language has facilities for inheritance, so naturally I add it as an extension. To have one class inherit from another, simply put the name of the parent after the name of the child. Like so:

    {(SomeChild)(Parent1)(Parent2)}

Classes named earlier will override the functions from the parents named later. The constructors of the parents will be called in the reverse order that they are listed in. So in the following program, the constructor of the Child class sets some_val to 1.

    {(Parent1) [(c__) (some_val)<1>=]}
    {(Parent2) [(c__) (some_val)<2>=]}
    {(Child) (Parent1) (Parent2)}

# Imports
Recognizing the value of splitting up code into multiple files, this interpreter allows the importing of classes from other files. To import classes from another file, simply have a string literal with the name of the file outside any class definition. For instance, the following file imports the classes from the file "included.glass".

    "included.glass"
    {M
        [m (object)(ImportedClass)!]
    }

# Disabling extensions
To disable any extensions to the language, pass the `--pedantic` flag to the interpreter. To convert code using extensions to standard Glass code, simply pass the `--convert` flag to the interpreter.

# Other features
## Minification/Obfuscation
This interpreter provides the ability to minify/obfuscate Glass programs by passing the `--minify` flag to the interpreter. For example, this code:

    {M
        [m
            (loop_val)<10>=
            (output)O!
            (math)A!
            /(loop_val)
                "Hey!\n"(output)o.?
                (loop_val)(loop_val)*<1>(math)s.?=
            \
        ]
    }

becomes:

    {M[mb<10>=gO!hA!/b"Hey!\n"go.?bb*<1>hs.?=\]}

## Main class constructor
For whatever reason, the reference interpreter fails to call the constructor when the first instance of `M` is created. This interpreter fixes this oversight.

## Destructor
This interpreter does **not** call the special function `d__` when an object is destructed. It doesn't really make sense for Glass to have destructors, since it's a garbage-collected language, and there aren't any resources that objects have to clean up. It should be noted that the reference interpreter also doesn't implement destructors either, and no known Glass program uses them.
