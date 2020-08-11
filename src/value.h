#ifndef __VALUE_H
#define __VALUE_H

#include "abstract.h"


// ---------------------------------------------------------------------------------------------------------------------
// Value union

/// a Value is a memory block used to store the value of Primitive calls which
/// should be 64 bits in size and can refer to the following
union Value
{
    std::int64_t i;
    double d;
    bool b;
    std::string* s;
};

/// wrapper to construct a new std::string used as a Call value
String* StringConstructor(const String& value);

/// wrapper which should be used to destroy a std::string used as a Call value
void StringDestructor(String* s);

/// wrappers to assign [v] to [value]
void AssignValue(Value& v, const String& value);
void AssignValue(Value& v, bool value);
void AssignValue(Value& v, double value);
void AssignValue(Value& v, int value);

#endif
