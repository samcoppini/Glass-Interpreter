# Glass self-interpreter
This directory contains the files for a Glass interpreter written in Glass. It is written with the extensions of inheritance and the module system, but is not capable of interpreting code using those extensions. A minified, converted version of the self-interpreter can be found in `self.glass`, and it is capable of running itself.

To use the interpreter, run `main.glass`, and give as input the code for a valid Glass program, optionally followed by a semicolon and any input you would like to give to the Glass program. Try not to give an invalid Glass program to the self-interpreter, as it is not designed to handle errors gracefully.
