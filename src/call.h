#ifndef __CALL_H
#define __CALL_H

#include "abstract.h"

typedef const std::string* BindingType;


inline const std::string ObjectType = "Object";
inline const std::string IntegerType = "Integer";
inline const std::string DecimalType = "Decimal";
inline const std::string StringType = "String";
inline const std::string BooleanType = "Boolean";
inline const std::string NullType = "Nothing";
inline const std::string ArrayType = "Array";
inline const std::string TupleType = "Tuple";
inline const std::string SomethingType = "Something";
inline const std::string MethodType = "Method";


struct Call
{
    const String* Name;
    BindingType BoundType;
    extArg_t BoundSection;
    Scope* BoundScope;

    // used for primitives
    void* Value;
};

Call* CallConstructor(const String* name=nullptr);
void CallDestructor(Call* call);

bool CallIsPrimitive(Call* call);

void BindType(Call* call, BindingType type);

void BindSection(Call* call, extArg_t section);

void BindScope(Call* call, Scope* scope);

void BindValue(Call* call, void* value);

String StringValueOf(const Call* call);
int IntegerValueOf(const Call* call);
double DecimalValueOf(const Call* call);
bool BooleanValueOf(const Call* call);

bool ValueMatchesPrimitiveCall(int value, Call* call);
bool ValueMatchesPrimitiveCall(double value, Call* call);
bool ValueMatchesPrimitiveCall(String& value, Call* call);
bool ValueMatchesPrimitiveCall(bool value, Call* call);

bool ListContainsPrimitiveCall(std::vector<Call*> list, int value, Call** foundCall);
bool ListContainsPrimitiveCall(std::vector<Call*> list, double value, Call** foundCall);
bool ListContainsPrimitiveCall(std::vector<Call*> list, bool value, Call** foundCall);
bool ListContainsPrimitiveCall(std::vector<Call*> list, String& value, Call** foundCall);

#endif
