#include "call.h"

#include "value.h"
#include "vm.h"
#include "diagnostics.h"


// ---------------------------------------------------------------------------------------------------------------------
// Call structure methods

/// wrapper which should always be used when creating a new instance of Call
/// with [name]
Call* CallConstructor(const String* name)
{
    auto call = new Call;
    call->Name = name;
    call->BoundScope = nullptr;
    call->BoundSection = 0;
    call->BoundType = nullptr;
    call->CallType = nullptr;

    call->BoundValue.s = 0;
    call->NumberOfParameters = 0;

    return call;
}

/// wrapper which should always be used when freeing an instance of Call [call]
void CallDestructor(Call* call)
{
    delete call;
}

// ---------------------------------------------------------------------------------------------------------------------
// Simple type names

std::string ObjectType = "Object";
std::string IntegerType = "Integer";
std::string DecimalType = "Decimal";
std::string StringType = "String";
std::string BooleanType = "Boolean";
std::string NothingType = "Nothing";
std::string ArrayType = "Array";
std::string TupleType = "Tuple";
std::string MethodType = "Method";
std::string AnythingType = "Anything";

std::string AbstractObjectType = "AbstractObject";
std::string AbstractIntegerType = "AbstractInteger";
std::string AbstractDecimalType = "AbstractDecimal";
std::string AbstractStringType = "AbstractString";
std::string AbstractBooleanType = "AbstractBoolean";
std::string AbstractArrayType = "AbstractArray";
std::string AbstractTupleType = "AbstractTuple";
std::string AbstractMethodType = "AbstractMethod";


// ---------------------------------------------------------------------------------------------------------------------
// Call binding methods

/// bind [type] to [call]
void BindType(Call* call, BindingType type)
{
    call->BoundType = type;
}

/// bind [section] to [call]
void BindSection(Call* call, extArg_t section)
{
    call->BoundSection = section;
}

/// bind [scope] to [call]
void BindScope(Call* call, Scope* scope)
{
    call->BoundScope = scope;
}

/// bind [value] to [call]
void BindValue(Call* call, Value value)
{
    call->BoundValue = value;
}

/// bind [type] to [call] as the CallType
void EnforceCallType(Call* call, BindingType type)
{
    if(call->CallType == nullptr)
    {
        call->CallType = type;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Default call type casting


/// obtain the String form of [call]'s value
String StringValueOf(const Call* call)
{
    if(call->BoundType == &IntegerType)
    {
        return std::to_string(call->BoundValue.i);
    }
    else if(call->BoundType == &DecimalType)
    {
        return std::to_string(call->BoundValue.d);
    }
    else if(call->BoundType == &BooleanType)
    {
        if(call->BoundValue.b)
        {
            return "true";
        }
        return "false";
    }
    else if(call->BoundType == &StringType)
    {
        return *call->BoundValue.s;
    }
    else
    {
        return "<" + *call->BoundType + ">";
    }
}

/// obtain the int form of [call]'s value
int IntegerValueOf(const Call* call)
{
    if(call->BoundType != &IntegerType)
    {
        LogIt(LogSeverityType::Sev1_Notify, "GetIntValue", "only implemented for IntegerType");
        return 0;
    }
    return call->BoundValue.i;
}

/// obtain the double form of [call]'s value
double DecimalValueOf(const Call* call)
{
    if(call->BoundType == &DecimalType)
    {
        return call->BoundValue.d;
    }
    if(call->BoundType == &IntegerType)
    {
        return static_cast<double>(call->BoundValue.i);
    }
    LogIt(LogSeverityType::Sev1_Notify, "GetDecimalValue", "only implemented for Integer and Decimal Typees");
    return 0;
}

/// obtain the bool form of [call]'s value
bool BooleanValueOf(const Call* call)
{
    if(call->BoundType == &BooleanType)
    {
        return call->BoundValue.b;
    }
    else if(call->BoundType == &NothingType)
    {
        return false;
    }

    LogIt(LogSeverityType::Sev1_Notify, "BooleanValueOf", "only implemented for Boolean and Nothing Typees");
    return 0;    
}


// ---------------------------------------------------------------------------------------------------------------------
// Call Type matching

/// true if [call] is type [cls]
bool CallIsType(const Call* call, BindingType cls)
{
    return call->BoundType == cls;
}

/// true if [call] refers to a primitive type
bool CallIsPrimitive(Call* call)
{
    auto type = call->BoundType;
    return type == &IntegerType || type == &DecimalType || type == &BooleanType || type == &StringType;
}


// ---------------------------------------------------------------------------------------------------------------------
// Call matching by value

/// true if [call] has [value]
bool ValueMatchesPrimitiveCall(int value, const Call* call)
{
    return CallIsType(call, &IntegerType) && call->BoundScope != &NothingScope && call->BoundValue.i == value;
}

/// true if [call] has [value]
bool ValueMatchesPrimitiveCall(const String& value, const Call* call)
{
    return CallIsType(call, &StringType) && call->BoundScope != &NothingScope &&  *call->BoundValue.s == value;
}

/// true if [call] has [value]
bool ValueMatchesPrimitiveCall(double value, const Call* call)
{
    return CallIsType(call, &DecimalType) && call->BoundScope != &NothingScope && call->BoundValue.d == value;
}

/// true if [call] has [value]
bool ValueMatchesPrimitiveCall(bool value, const Call* call)
{
    return CallIsType(call, &BooleanType) && call->BoundScope != &NothingScope && call->BoundValue.b == value;
}


// ---------------------------------------------------------------------------------------------------------------------
// Call lookup


/// true if [list] contains a Call where [value] satisfies 
/// ValueMatchesPrimitiveCall. leaves [foundCall] pointing to the matched call
bool ListContainsPrimitiveCall(const std::vector<Call*>& list, int value, Call** foundCall)
{
    for(auto call: list)
    {
        if(ValueMatchesPrimitiveCall(value, call))
        {
            *foundCall = call;
            return true;
        }
    }
    
    *foundCall = nullptr; 
    return false;
}

/// true if [list] contains a Call where [value] satisfies 
/// ValueMatchesPrimitiveCall. leaves [foundCall] pointing to the matched call
bool ListContainsPrimitiveCall(const std::vector<Call*>& list, const String& value, Call** foundCall)
{
    for(auto call: list)
    {
        if(ValueMatchesPrimitiveCall(value, call))
        {
            *foundCall = call;
            return true;
        }
    }
    
    return false;
}

/// true if [list] contains a Call where [value] satisfies 
/// ValueMatchesPrimitiveCall. leaves [foundCall] pointing to the matched call
bool ListContainsPrimitiveCall(const std::vector<Call*>& list, double value, Call** foundCall)
{
    for(auto call: list)
    {
        if(ValueMatchesPrimitiveCall(value, call))
        {
            *foundCall = call;
            return true;
        }
    }
    
    return false;
}

/// true if [list] contains a Call where [value] satisfies 
/// ValueMatchesPrimitiveCall. leaves [foundCall] pointing to the matched call
bool ListContainsPrimitiveCall(const std::vector<Call*>& list, bool value, Call** foundCall)
{
    for(auto call: list)
    {
        if(ValueMatchesPrimitiveCall(value, call))
        {
            *foundCall = call;
            return true;
        }
    }

    return false;
}
