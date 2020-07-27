#ifndef __CALL_H
#define __CALL_H

#include "abstract.h"

typedef std::string BindingType;


inline const BindingType ObjectType = "Object";
inline const BindingType IntegerType = "Integer";
inline const BindingType DecimalType = "Decimal";
inline const BindingType StringType = "String";
inline const BindingType BooleanType = "Boolean";
inline const BindingType NullType = "Nothing";
inline const BindingType ArrayType = "Array";
inline const BindingType TupleType = "Tuple";
inline const BindingType SomethingType = "Something";
inline const BindingType MethodType = "Method";


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
