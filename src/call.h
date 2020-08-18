#ifndef __CALL_H
#define __CALL_H

#include "abstract.h"
#include "value.h"

// ---------------------------------------------------------------------------------------------------------------------
// Type structure

/// contains the information about the type bound to a given call
/// [BoundName] is the displayable name
/// [InputTypes] is a list of types for any parameters to a function
/// [OutputType] is the type of the output for a functiohn
struct Type
{
    std::string* BoundName;
    std::vector<Type*> InputTypes;
    Type* OutputType;
};

/// temporary alias to enable a reduced subset of the type system's features
typedef const std::string* BindingType;



// ---------------------------------------------------------------------------------------------------------------------
// Simple type names

extern std::string ObjectType;
extern std::string IntegerType;
extern std::string DecimalType;
extern std::string StringType;
extern std::string BooleanType;
extern std::string NothingType;
extern std::string ArrayType;
extern std::string TupleType;
extern std::string MethodType;
extern std::string AnythingType;


extern std::string AbstractObjectType;
extern std::string AbstractIntegerType;
extern std::string AbstractDecimalType;
extern std::string AbstractStringType;
extern std::string AbstractBooleanType;
extern std::string AbstractArrayType;
extern std::string AbstractTupleType;
extern std::string AbstractMethodType;


// ---------------------------------------------------------------------------------------------------------------------
// Call structure

/// contains pointers to all field which can be bound to a call
/// [Name] refers to the displayable name of the Call
/// [BoundType] is the Type of the Call
/// [BoundSection] is a long long int which describes the bytecode instruction
///                (aka section) which the Call directs to
/// [BoundScope] is the scope used to lookup attributes via the '.' dot operator
/// [Value] is either a int/bool/double/std::string used to express the
///         primitive value associated with a simple "object"
struct Call
{
    const String* Name;
    BindingType BoundType;
    extArg_t BoundSection;
    Scope* BoundScope;
    BindingType CallType;

    // used for primitives
    Value BoundValue;

    extArg_t NumberOfParameters;
};

/// wrapper which should always be used when creating a new instance of Call
/// with [name]
Call* CallConstructor(const String* name=nullptr);

/// wrapper which should always be used when freeing an instance of Call [call]
void CallDestructor(Call* call);

/// true if a [call] refers to a primitive type
bool CallIsPrimitive(Call* call);



// ---------------------------------------------------------------------------------------------------------------------
// Type system

/// the scope of NothingCall which is also assigned to new Calls and calls 
/// which have yet to be bound to a scope
extern Scope NothingScope;

/// the scope used for Primitives which have values
extern Scope SomethingScope;

inline bool Strictly(const BindingType type, const Call* call)
{
    return call->BoundType == type 
        && call->BoundScope != &NothingScope;
}

/// true if [call] corresponds to the general definition of Nothing
inline bool IsNothing(const Call* call)
{
    return call->BoundScope == &NothingScope;
}

/// true if [call] corresponds to the strict definition of Nothing
inline bool IsPureNothing(const Call* call)
{
    return call->BoundType == &NothingType;
}

// ---------------------------------------------------------------------------------------------------------------------
// Call binding actions

/// bind [type] to [call]
void BindType(Call* call, BindingType type);

/// bind [section] to [call]
void BindSection(Call* call, extArg_t section);

/// bind [scope] to [call]
void BindScope(Call* call, Scope* scope);

/// bind [value] to [call]
void BindValue(Call* call, Value value);

/// bind [type] to [call] as the CallType
void EnforceCallType(Call* call, BindingType type);



// ---------------------------------------------------------------------------------------------------------------------
// Default call type casting
//
// methods to convert a [call] into a primitive type (only String/Integer/
// Decimal/Boolean)

/// obtain the String form of [call]'s value
String StringValueOf(const Call* call);

/// obtain the int form of [call]'s value
int IntegerValueOf(const Call* call);

/// obtain the double form of [call]'s value
double DecimalValueOf(const Call* call);

/// obtain the bool form of [call]'s value
bool BooleanValueOf(const Call* call);


// ---------------------------------------------------------------------------------------------------------------------
// Call matching by value
//
// as a call for a primitive value should only be constructed once, these
// methods match a primitive value against an arbitrary call

/// true if [call] has [value]
bool ValueMatchesPrimitiveCall(int value, const Call* call);

/// true if [call] has [value]
bool ValueMatchesPrimitiveCall(double value, const Call* call);

/// true if [call] has [value]
bool ValueMatchesPrimitiveCall(const String& value, const Call* call);

/// true if [call] has [value]
bool ValueMatchesPrimitiveCall(bool value, const Call* call);


// ---------------------------------------------------------------------------------------------------------------------
// Call lookup
//
// as a all for a primitive value should only be constructed once, these
// methods lookup a primitive value in a [list] (std::vector) of calls

/// true if [list] contains a Call where [value] satisfies 
/// ValueMatchesPrimitiveCall. leaves [foundCall] pointing to the matched call
bool ListContainsPrimitiveCall(const std::vector<Call*>& list, int value, Call** foundCall);

/// true if [list] contains a Call where [value] satisfies 
/// ValueMatchesPrimitiveCall. leaves [foundCall] pointing to the matched call
bool ListContainsPrimitiveCall(const std::vector<Call*>& list, double value, Call** foundCall);

/// true if [list] contains a Call where [value] satisfies 
/// ValueMatchesPrimitiveCall. leaves [foundCall] pointing to the matched call
bool ListContainsPrimitiveCall(const std::vector<Call*>& list, bool value, Call** foundCall);

/// true if [list] contains a Call where [value] satisfies 
/// ValueMatchesPrimitiveCall. leaves [foundCall] pointing to the matched call
bool ListContainsPrimitiveCall(const std::vector<Call*>& list, const String& value, Call** foundCall);

#endif
