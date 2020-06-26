# Chief
Chief is a language designed to mimic natural English as closely as possible while
still providing advanced capabilities. Knowledge of programming, more and more, is becoming both useful and necessary. Every day we interact with technology in ways we can only begin to imagine. Code governs some of our most nuclear and personal experiences. But we often take that code for granted, and we treat it as a black box.

We developed Chief because we believe that knowledge of coding is vital to be informed citizens and well rounded individuals. Our goal is to create a language that anyone can pickup, understand, and master. It is our hope that in the processes, newcomers will develop a stronger understanding of how code works and a more nuanced understanding of how coding can impact us as a society. In short, we aim to make learning how to code accessible.

Currently Chief is in early beta and many features are unimplemented. Please check frequently for updates.


## Using Chief
Chief is an intepreted language built over C++. Currently it uses an experimental syntax for testing purposes, but the eventual goal is to avoid using any formal syntax. To write programs in Chief, run whatever `make` command you have in the root directory of the repo (you will need `g++` installed). Create a `program` file in this directory with you code and run `chief.exe` to execute this code. Note that the `program` file and the `chief.exe` executable must be in the same directory.

## Language Specifications
General guidelines. Chief utilizes objects and references to objects. References are treated as proper nouns and should be captialized (e.g. `MyInt`). All other specifiers should be lowercased. 

Currently the following functionality is supported with a pythonic syntax
* Defining and assigning basic (untyped) variables
    * `X = 43`
* Basic arithmetic
    * `X = X + 34`
* Function defintions and calls
    * ```python
        def Fact(X):
            if(X):
                return X * Fact(X-1)
            return 1

        X = Fact(4)
        # 24


Eventually the goal will be to implement functionality like the following atomic operations are defined

* `define:` Defines a refernce to an object. All references must be defined before use
    * `define MyFirstInt to be 5`
* `add:` Add two objects together. 
    * `add MyFirstInt and MySecondInt`
* `print:` Print an object.
    * `print "Hello world"`
    * `display MyInt`
* `=` Set the Reference left of the equals sign to the Reference right of the equals sign
    * `MyAwesomeInt = MyInt1 + MyInt2`
