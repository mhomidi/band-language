# Band Language

In the context of LLVM, "Band language" is a simple, minimalistic programming language that is often used for educational or illustrative purposes. Band language is not intended for practical use but rather serves as a means to demonstrate and teach various concepts related to compiler design, code generation, and program optimization using LLVM.

Developing a language with LLVM involves defining the syntax and semantics of the language, creating a lexer and parser to analyze the code, and generating LLVM Intermediate Representation (IR) code from the parsed source code. This IR code can then be further optimized and finally compiled to machine code or executed in an LLVM-based virtual machine.

## Sources

**AST**: represents the hierarchical structure of a program's source code in a tree-like format, used in language development for parsing and analyzing code.
**Parser**: analyzes the syntax of source code and converts it into a structured representation
**Lexer**: breaks down the source code into individual tokens or lexemes for subsequent parsing and analysis.

## Run
Just `make` and `a.out`. An example of code:
```
def foo(a b) a*a + 2*a*b + b*b;
```
