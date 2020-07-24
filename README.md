# Pebble

Pebble is an open-source interpreted scripting language designed to explore how syntax and programming primitives can be reimagined in a more familiar, human friendly way. Our mission is to create a language that is easy to pick up, straighforward to learn, and expressive enough to be impactful. Achieving these goals involves finding innovative solutions to reduce syntax constraints and designing novel ways to think about coding primitives.

## Motiviation

As we move forward into this modern era, we are touched by technology in ways that are profound and unnoticed. Code governs our everyday. But we often take that code for granted, and we treat it as a black box.

Technology has the power to help solve the world's toughest and oldest problems. But to harness it's creative power first requires an broad knowledge of how it works. And this is a responsibility which rests on everyone's shoulders. It shouldn't be enough to let the "experts" do the work and simply trust their results; that technology is so powerful and personal means that it *must* be held accountable. It means that each citizen must understand enough about it to know when it isn't working for society. And while citizens need not be *authors* themselves, they must be *literate*.

We're developing Pebble because we believe that knowledge of coding is vital to be informed citizens and well rounded individuals. Our goal is to create a language that anyone can pickup, understand, and master. It is our hope that in the process, newcomers will develop a stronger understanding of how code works and a more nuanced understanding of how coding can impact our society. 


## Using Pebble
Pebble is an intepreted language built over C++. To get started with Pebble, you'll need a `make` utility (either the standard Linux `make` or [mingw32-make](https://sourceforge.net/p/mingw-w64/wiki2/Make/) on Windows). Additionally, you will need a C++ compiler. We use `g++` as our standard.

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


## Language Specification
Currently it uses an experimental syntax (codenamed Boulder). The syntax is still in early beta and is subject to rapid change. These changes may not be reflected in this documentation. We will update this section as often as possible.

### Overview of Ideas (out of date)
Pebble abstracts coding into three fundemental, interrelated concepts. References, Objects, and Methods. Building on those are the ideas of Scope and Block

#### References
References are names and nouns. In real life they're the words we use to refer to things. For example, `Bob` can be used to refer a person (named Bob of course). Other references to that same person might be `Him` or `the Person`. In Pebble, references always start with a capital letter, just like with proper nouns in English. References can point to either an Object or a Method.

#### Objects
Objects are the things. In the example above, `Bob`, `Him`, and `the Person` are all references to the same underlying thing--in this case my friend Bob. Objects in Pebble have attributes (eg. `Bob's Wallet`) which are themselves references.

#### Methods
Methods are the actions which can be done on objects. They're also named by references (eg. `DriveMyCar()`). 

#### Scope
Scope refers to the references available at a given place in the code. When `Bob` evaluates a method, any references are evaluated in his scope. If `Alice` had some attribute `SecretLetter`, the clearly `Bob` would not be able to access it!

#### Blocks
Blocks are lines of code with the same scope. The `:` operator tells the Pebble interpreter that a block is expected. Blocks also inherit the scope of the block caller.

### Overview of Keywords (out of date)
#### Control Flow
These keywords are used in the obvious way
* `if <expr> :`
* `for each <Ref> in <Expr> :`
* `while <expr> :`

Each of these expects a block which inherits the program scope at each call.

#### Operations
These keywords signal binary operations in the obvious way
* Math: `+ - * /`
* Logic: `and && or ||`
* Comparison: `> < >= <= == != is (is not)`

These keywords signal unary operations in the obvious way
* Logic: `not !`

#### Articles
The definite and indefinite articles `the`, `a`, and `an` can be used to improve readability, they are treated as enclosed commments by the interpreter and ignored.

#### Evaluation
The keywords `consider` and `take` can be used before an expression to explicitly keep the result of executing the expression in the program memory. The immediate last result can be referenced by the keywords `it` and `that` to break up and simplify chaining.

#### Caller
The keyword `caller` should be used from inside a method and refers to the object which calls the method. If the method is global, and has no caller, then `Nothing` will be passed in as the caller

### Example Code

![Pebble code example](./assets/img/pebble_example1.png)
