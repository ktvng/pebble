
# Overview 
"a Variable" = a variable name?
    e.g. 
        X = 4
        Y = 4
start with upper case
        
Variables = "words" that can be bound to a scope/section (uppercase name)
Scopes = a list of variables available
Sections = a block of code

Primitives: (Only 1 copy ever exists)
Decimals 3.4

3.4
 (variable)
    special scope
    special section
    
Strings "hello"
Integers 23
Booleans true

X = 3.4
    x binds 3.4 special section
    x binds 3.4 special scope


X():
    binds a new scope and a new section
    (implicitly returns caller, if no caller then self)

Y = X (R->L) (Y points to same mem as X)
    transfers whatever scope/section are bound to X to Y

a X
    bind a type to the variable X
    (implicitly returns self)


# Section: Andrew 
What is up my Guys? It's ya boi DJ Whiteman back at it again with another live share `pebble.pebl` edition.

# Section: Jillian
i am jillian

# Section: Kevin
self: 
{
    name: "Kevin",
    id: 343,
    greeting: "yo"
}
