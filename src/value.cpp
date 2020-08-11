#include <iostream>

#include "value.h"


void StringDestructor(String* s)
{
    delete s;
}

String* StringConstructor(const String& value)
{
    std::string* s = new std::string;
    *s = value;
    return s;
}

void AssignValue(Value& v, const String& value)
{
    v.s = StringConstructor(value);
}

void AssignValue(Value& v, bool value)
{
    v.b = value;
}

void AssignValue(Value& v, double value)
{
    v.d = value;
}

void AssignValue(Value& v, int value)
{
    v.i = value;
}