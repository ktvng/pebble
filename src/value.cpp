#include <iostream>

#include "value.h"

/// wrapper which should be used to destroy a std::string used as a Call value
void StringDestructor(String* s)
{
    delete s;
}

/// wrapper to construct a new std::string used as a Call value
String* StringConstructor(const String& value)
{
    std::string* s = new std::string;
    *s = value;
    return s;
}

/// wrapper to assign [v] to [value]
void AssignValue(Value& v, const String& value)
{
    v.s = StringConstructor(value);
}

/// wrapper to assign [v] to [value]
void AssignValue(Value& v, bool value)
{
    v.b = value;
}

/// wrapper to assign [v] to [value]
void AssignValue(Value& v, double value)
{
    v.d = value;
}

/// wrapper to assign [v] to [value]
void AssignValue(Value& v, int value)
{
    v.i = value;
}
