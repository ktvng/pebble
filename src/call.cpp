#include "call.h"

#include "diagnostics.h"

Call* CallConstructor(const String* name)
{
    auto call = new Call;
    call->Name = name;
    call->BoundScope = nullptr;
    call->BoundSection = 0;
    call->Value = nullptr;
    call->BoundType = nullptr;

    return call;
}

void CallDestructor(Call* call)
{
    delete call;
}

void BindType(Call* call, BindingType type)
{
    call->BoundType = type;
}

void BindSection(Call* call, extArg_t section)
{
    call->BoundSection = section;
}

void BindScope(Call* call, Scope* scope)
{
    call->BoundScope = scope;
}

void BindValue(Call* call, void* value)
{
    call->Value = value;
}

String StringValueOf(const Call* call)
{
    if(call->BoundType == &IntegerType)
    {
        return std::to_string(*static_cast<int*>(call->Value));
    }
    else if(call->BoundType == &DecimalType)
    {
        return std::to_string(*static_cast<double*>(call->Value));
    }
    else if(call->BoundType == &BooleanType)
    {
        if(*static_cast<bool*>(call->Value))
        {
            return "true";
        }
        return "false";
    }
    else if(call->BoundType == &StringType)
    {
        return *static_cast<String*>(call->Value);
    }
    else
    {
        return "<" + *call->BoundType + ">";
    }
}

int IntegerValueOf(const Call* call)
{
    if(call->BoundType != &IntegerType)
    {
        LogIt(LogSeverityType::Sev1_Notify, "GetIntValue", "only implemented for IntegerType");
        return 0;
    }
    return *static_cast<int*>(call->Value);
}

double DecimalValueOf(const Call* call)
{
    if(call->BoundType == &DecimalType)
    {
        return *static_cast<double*>(call->Value);
    }
    if(call->BoundType == &IntegerType)
    {
        return static_cast<double>(*static_cast<int*>(call->Value));
    }
    LogIt(LogSeverityType::Sev1_Notify, "GetDecimalValue", "only implemented for Integer and Decimal Typees");
    return 0;
}

bool BooleanValueOf(const Call* call)
{
    if(call->BoundType == &BooleanType)
    {
        return *static_cast<bool*>(call->Value);
    }
    else if (call->BoundType == &IntegerType)
    {
        return static_cast<bool>(*static_cast<int*>(call->Value));
    }
    else if(call->BoundType == &NullType)
    {
        return false;
    }
    else
    {
        return true;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Call Type matching
bool CallIsType(Call* call, BindingType cls)
{
    return call->BoundType == cls;
}

bool CallIsPrimitive(Call* call)
{
    auto type = call->BoundType;
    return type == &IntegerType || type == &DecimalType || type == &BooleanType || type == &StringType;
}

// ---------------------------------------------------------------------------------------------------------------------
// Matching primitives

bool ValueMatchesPrimitiveCall(int value, Call* call)
{
    return CallIsType(call, &IntegerType) && *static_cast<int*>(call->Value) == value;
}

bool ValueMatchesPrimitiveCall(String& value, Call* call)
{
    LogDiagnostics(call);
    return CallIsType(call, &StringType) && *static_cast<String*>(call->Value) == value;
}

bool ValueMatchesPrimitiveCall(double value, Call* call)
{
    return CallIsType(call, &DecimalType) && *static_cast<double*>(call->Value) == value;
}

bool ValueMatchesPrimitiveCall(bool value, Call* call)
{
    return CallIsType(call, &BooleanType) && *static_cast<bool*>(call->Value) == value;
}

bool ListContainsPrimitiveCall(std::vector<Call*> list, int value, Call** foundCall)
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

bool ListContainsPrimitiveCall(std::vector<Call*> list, String& value, Call** foundCall)
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

bool ListContainsPrimitiveCall(std::vector<Call*> list, double value, Call** foundCall)
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

bool ListContainsPrimitiveCall(std::vector<Call*> list, bool value, Call** foundCall)
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
