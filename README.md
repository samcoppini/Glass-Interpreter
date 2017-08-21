Glass Interpreter
=================
An interpreter written in C++ for the esoteric programming language, [Glass](https://esolangs.org/wiki/Glass).

## Usage:
    usage: glass glass_file [args...]
    --convert   Convert glass code with extensions to standard glass
    --help      Display this help message
    --minify    Outputs a minified version of the source code
    --pedantic  Disallow extensions to the base language of Glass
    --width     Restricts the length of lines of minified source

## Building
Download the source by using the following command in your command prompt:
```sh
$ git clone https://github.com/samcoppini/Glass-Interpreter.git
```
or, alternatively, just download a [zip file of the source code](https://github.com/samcoppini/Glass-Interpreter/archive/master.zip).

After downloading it, simply use `make` to create the executable. Requires a C++17 compatible compiler.
