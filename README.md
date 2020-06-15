# Chief
Chief is a language designed to mimic natural English as closely as possible while
still providing advanced capabilities.

Currently Chief is in early beta and many features are unimplemented.


## Using Chief
Chief is an intepreted language built over C++. Currently it does not have any real syntax. To write programs in Chief, `make` the makefile and run `chief.exe` to run a program. This program must be in the same directory as `chief.exe` and be named `program`.

## Language Specifications
General guidelines. Chief utilizes objects and references to objects. References are treated as proper nouns and should be captialized (e.g. `MyInt`). All other specifiers should be lowercased. 

The following atomic operations are defined

* 'define:' Defines a refernce to an object. All references must be defined before use
    * `define MyFirstInt to be 5`
* `add:` Add two objects together. 
    * `add MyFirstInt and MySecondInt`
* `print:` Print an object.
    * `print "Hello world"`
    * `display MyInt`
* `=:` Set the Reference left of the equals sign to the Reference right of the equals sign
    * `MyAwesomeInt = MyInt1 + MyInt2`
