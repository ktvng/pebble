#ifndef __VALUE_H
#define __VALUE_H

#include "abstract.h"

union Value
{
    std::int64_t i;
    double d;
    bool b;
    std::string* s;
};

String* StringConstructor(const String& value);
void StringDestructor(String* s);

void AssignValue(Value& v, const String& value);
void AssignValue(Value& v, bool value);
void AssignValue(Value& v, double value);
void AssignValue(Value& v, int value);

#endif
