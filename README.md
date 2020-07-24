# Pebble

Pebble is an open source interpreted scripting language designed to explore how syntax and programming primitives can be reimagined in a more familiar, human friendly way. Our mission is to create a language that is easy to pick up, straightforward to learn, and expressive enough to be impactful. Achieving these goals involves finding innovative solutions to reduce syntax constraints and designing novel ways to think about coding primitives.

## Motivation

As we move forward into this modern era, we are touched by technology in ways that are profound and unnoticed. Code governs our everyday. But we often take that code for granted, and we treat it as a black box.

Technology has the power to help solve the world's toughest and oldest problems. But to harness its creative power first requires an broad knowledge of how it works. And this is a responsibility which rests on everyone's shoulders. It should not be enough to let the "experts" do the work and simply trust their results; that technology is so powerful and personal means that it *must* be held accountable. It means that each citizen must understand enough about it to know when it isn't working for society. And while citizens need not be *authors* themselves, they must be *literate*.

We're developing Pebble because we believe that knowledge of coding is vital to be informed citizens and well-rounded individuals. Our goal is to create a language that anyone can pick up, understand, and master. It is our hope that in the process, newcomers will develop a stronger understanding of how code works and a more nuanced understanding of how coding can impact our society. 


## Using Pebble
Pebble is an interpreted language built over C++. To get started with Pebble, you'll need a `make` utility (either the standard Linux `make` or [mingw32-make](https://sourceforge.net/p/mingw-w64/wiki2/Make/) on Windows). Additionally, you will need a C++ compiler. We use `g++` as our standard.

Steps to install:
 * Clone the Pebble repo and run your `make` command in the repo root directory. This should produce a new executable `pebble.exe`
   ```
   $ make
   ```
 * In this same directory create a file titled `program.pebl` and write any Pebble code you would like to run in this file.
   ```python
   # program.pebl
   print "hello world"
   ```
 * Run your program by opening a terminal and running the command `./pebble.exe`
   ```
   $ ./pebble.exe
   ```

## Running the unit tests
Follow the following steps after you have installed Pebble to create the test build of the Pebble interpreter and run the unit tests
```
$ make TestBuilder
$ make pebble_testbuild.exe
$ ./pebble_testbuild.exe --byte-code --ignore-custom
```


# Language Specification
Currently it uses an experimental syntax (codenamed Boulder). The syntax is still in early beta and is subject to rapid change. These changes may not be reflected in this documentation. We will update this section as often as possible.

## Overview of Ideas
Pebble uses three core primitives which can be translated into different narratives/coding-paradigms. These three concepts are **Calls**, **Scopes** and **Sections**

### Calls
A call is the name given to a variable. For instance, in the statement `X = 4`, `X` is a call for `4`, whatever that is. Currently calls are untyped and Pebble is a dynamically typed language, but whether this is a central design feature is still under discussion. 

By legacy convention, calls must begin with an uppercase letter and must only consist of alphanumeric characters and underscores.

The same “thing” can have multiple calls. 
```python
X = 4
Y = 4
```
Whatever `4` is has the call `X` and the call `Y`. 

### Scope
Conceptually speaking, Scope is used in roughly the same was as in C++ and other traditional languages. It represents all calls which are defined at a given point in the code. Specifically, scope is determined by the indent level of a statement.
```python
if True:
    X = 4
print X
```
In the example above, `Nothing` will be printed because `X` is defined in the scope of the `if` statement (i.e. it is defined on a higher indent level) and thus not visible in the scope where `print X` is called

Similar to other langauges, "Methods" also have their own local scope which is separate from the scope where a Method is called. When statements are defined at indent level 0, however, this is considered to be **program scope** and all calls defined in this scope are accessible everywhere.

### Sections
A section is a block of imperative instructions which are executable. These are the actual lines of code written in Pebble, and are grouped by indent level. For example
```python
X = 4
print X
    Y = 4
    print Y
```
contains two sections of code. 


## Narratives
Using these three core primitives, different narratives can be constructed that support either imperative, object oriented, or functional (WIP) paradigms. Understand how to construct narratives first requires a general understanding of three operators, the `a` (typedef), `:` (sectionbinding), and `()` (scopebinding).

### Typedef operator
The `a` operator defines a new type. It has the usage `a <Call>` and it creates scope type of the same name as `<Call>`.

### SectionBinding operator
The `:` operator binds a section to call. It has the usage `<Call>:` (with no args) or `<Call>(<args>):` (with args) and it binds the next section to `<Call>`

### ScopeBinding operator
The `()` operator creates a new scope with optional parameters. It has the usage `<Call>(<args>)` or `(<args>)`. The latter creates a new scope with `<args>` being call parameters. The former does this and binds the scope to `<Call>`. In general, when a call is bound to a scope, it will evaluate its bound section, unless it is also being bound to a new section by the clause.
```python
# evaluates the bound section of Call
Call()

# does not evaluate the bound section of Call because rebinding is concurrently specified
# instead rebinds the section of Call to next section (indented portion)
Call():
    print "Called"

```

### Object Oriented Pebble
Classes can be considered to be typed scopes bound to a section

```python
a Rock:
    Color
    Weight

```

Instantiating an instance of a class is equivalent to evaluating the bound section of a typed scope
```python
MyRock = Rock()
```

Methods are scopes bound to a section. They can be invoked by the `()` (sectioneval) operator
```python
# binding a scope and section, section is not evaluated here
Throw():
    print "thrown"

# the scope is bound and the originally bound section evaluated
Throw()
```

The `.` (dot) operator functions as expected. An additional feature is that it preserves an additional relationship between the caller and the called "thing". This is illustrated in the following example
```python
a Rock:
    Color = "Grey"
    Weight

    Info():
        Name = "Info"
        print self.Info + ":" + caller.Color

MyRock = Rock()
MyRock.Info()
# Info:Grey
```
In this example, the keyword `caller` is a call to the "Object" to the left of the dot, which is whatever scope is calling the "Method". The `self` keyword refers to the current scope, which, inside a "Method" is the method's own scope. 

The last operator is the `here` operator. It is aliased to `inherits` to bootstrap inheritance
```python
a Rock:
    Color = "Grey"

a Pebble:
    # same as here Rock()
    inherits Rock()

MyPebble = Pebble()
print MyPebble.Color
# Grey
```
In this example, the `inherits` operator specifies that the section bound to `Rock` should be called in the current scope. Thus `Pebble` inherits the section bound to `Rock` which is exactly the section of code which defines its attributes.

