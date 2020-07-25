#include <cstdarg>
#include <iostream>

#include "object.h"
#include "diagnostics.h"
#include "program.h"
#include "reference.h"
#include "scope.h"
#include "token.h"

Object* ObjectConstructor()
{
    Object* obj = new Object;
    obj->Attributes = ScopeConstructor(nullptr);
    obj->Attributes->IsDurable = true;
    obj->Class = NullClass;
    obj->Value = nullptr;
    obj->Action = nullptr;
    obj->DefinitionScope = nullptr;

    return obj;
}

/// used in the BytecodeRuntime
Object* ObjectConstructor(ObjectClass cls, void* value)
{
    Object* obj = new Object;
    obj->Attributes = ScopeConstructor(nullptr);
    obj->Class = cls;
    obj->Value = value;
    obj->Action = nullptr;
    obj->DefinitionScope = nullptr;

    return obj;
}

void ObjectValueDestructor(Object* obj)
{
    auto klass = obj->Class;
    if(klass == IntegerClass)
    {
        delete static_cast<int*>(obj->Value);
    }
    else if(klass == DecimalClass)
    {
        delete static_cast<double*>(obj->Value);
    }
    else if(klass == BooleanClass)
    {
        delete static_cast<bool*>(obj->Value);
    }
    else if(klass == StringClass)
    {
        delete static_cast<std::string*>(obj->Value);
    }
}

void ObjectValueDestructor(ObjectClass klass, void* val)
{
    if(klass == IntegerClass)
    {
        delete static_cast<int*>(val);
    }
    else if(klass == DecimalClass)
    {
        delete static_cast<double*>(val);
    }
    else if(klass == BooleanClass)
    {
        delete static_cast<bool*>(val);
    }
    else if(klass == StringClass)
    {
        delete static_cast<std::string*>(val);
    }
}

void ObjectDestructor(Object* obj)
{
    ScopeDestructor(obj->Attributes);
    if(IsCallable(obj))
    {
        MethodDestructor(obj->Action);
    }
    if(obj->Value != nullptr)
    {
        ObjectValueDestructor(obj);
    }
    delete obj;
}


String* ObjectValueConstructor(String value)
{
    std::string* s = new std::string;
    *s = value;
    return s;
}
bool* ObjectValueConstructor(bool value)
{
    bool* s = new bool;
    *s = value;
    return s;
}
double* ObjectValueConstructor(double value)
{
    double* s = new double;
    *s = value;
    return s;
}
int* ObjectValueConstructor(int value)
{
    int* s = new int;
    *s = value;
    return s;
}

// ---------------------------------------------------------------------------------------------------------------------
// Methods

/// constructor for a Method* object with [inheritedScope]
Method* MethodConstructor()
{
    Method* m = new Method;
    m->CodeBlock = nullptr;
    m->ParameterNames = {};

    return m;
}

void MethodDestructor(Method* m)
{
    delete m;
}



Object* ObjectOf(const Reference* ref)
{
    return ref->To;
}

bool IsCallable(const Reference* ref)
{
    return ref->To->Action != nullptr;
}

bool IsCallable(const Object* obj)
{
    return obj->Action != nullptr;
}

// TODO: should ensure that ref is actually an object
bool IsNumeric(const Reference* ref)
{
    return ObjectOf(ref)->Class == IntegerClass || ObjectOf(ref)->Class == DecimalClass;
}

/// 
bool IsString(const Reference* ref)
{
    return ObjectOf(ref) != nullptr && ObjectOf(ref)->Class == StringClass;
}






ObjectClass GetPrecedenceClass(const Object* obj1, const Object* obj2)
{
    if(obj1->Class == DecimalClass || obj2->Class == DecimalClass)
        return DecimalClass;
    return IntegerClass;
}



String GetStringValue(const Object* obj)
{
    if(obj->Class == IntegerClass)
    {
        return std::to_string(*static_cast<int*>(obj->Value));
    }
    else if(obj->Class == DecimalClass)
    {
        return std::to_string(*static_cast<double*>(obj->Value));
    }
    else if(obj->Class == BooleanClass)
    {
        if(*static_cast<bool*>(obj->Value))
        {
            return "true";
        }
        return "false";
    }
    else if(obj->Class == StringClass)
    {
        return *static_cast<String*>(obj->Value);
    }
    else if(obj->Class == NullClass)
    {
        return "Nothing";
    }
    else if(obj->Class == BaseClass)
    {
        return "Object";
    }
    else if(obj->Class == SomethingClass)
    {
        return "Something";
    }
    else if(obj->Class == MethodClass)
    {
        return "Method";
    }
    else
    {
        return "<" + obj->Class + ">";
    }
}

int GetIntValue(const Object* obj)
{
    if(obj->Class != IntegerClass)
    {
        LogIt(LogSeverityType::Sev1_Notify, "GetIntValue", "only implemented for IntegerClass");
        return 0;
    }
    return *static_cast<int*>(obj->Value);
}

double GetDecimalValue(const Object* obj)
{
    if(obj->Class == DecimalClass)
    {
        return *static_cast<double*>(obj->Value);
    }
    if(obj->Class == IntegerClass)
    {
        return static_cast<double>(*static_cast<int*>(obj->Value));
    }
    LogIt(LogSeverityType::Sev1_Notify, "GetDecimalValue", "only implemented for Integer and Decimal classes");
    return 0;
}

bool GetBoolValue(const Object* obj)
{
    if(obj->Class == BooleanClass)
    {
        return *static_cast<bool*>(obj->Value);
    }
    else if (obj->Class == IntegerClass)
    {
        return static_cast<bool>(*static_cast<int*>(obj->Value));
    }
    else if(obj->Class == NullClass)
    {
        return false;
    }
    else
    {
        return true;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// object class matching
bool ObjectIsClass(Object* obj, ObjectClass cls)
{
    return obj->Class == cls;
}



// ---------------------------------------------------------------------------------------------------------------------
// Matching primitives

bool ValueMatchesPrimitiveObject(int value, Object* obj)
{
    return ObjectIsClass(obj, IntegerClass) && *static_cast<int*>(obj->Value) == value;
}

bool ValueMatchesPrimitiveObject(String value, Object* obj)
{
    return ObjectIsClass(obj, StringClass) && *static_cast<String*>(obj->Value) == value;
}

bool ValueMatchesPrimitiveObject(double value, Object* obj)
{
    return ObjectIsClass(obj, DecimalClass) && *static_cast<double*>(obj->Value) == value;
}

bool ValueMatchesPrimitiveObject(bool value, Object* obj)
{
    return ObjectIsClass(obj, BooleanClass) && *static_cast<bool*>(obj->Value) == value;
}

bool ListContainsPrimitiveObject(std::vector<Object*> list, int value, Object** foundObject)
{
    for(auto obj: list)
    {
        if(ValueMatchesPrimitiveObject(value, obj))
        {
            *foundObject = obj;
            return true;
        }
    }
    
    *foundObject = nullptr; 
    return false;
}

bool ListContainsPrimitiveObject(std::vector<Object*> list, String& value, Object** foundObject)
{
    for(auto obj: list)
    {
        if(ValueMatchesPrimitiveObject(value, obj))
        {
            *foundObject = obj;
            return true;
        }
    }
    
    return false;
}

bool ListContainsPrimitiveObject(std::vector<Object*> list, double value, Object** foundObject)
{
    for(auto obj: list)
    {
        if(ValueMatchesPrimitiveObject(value, obj))
        {
            *foundObject = obj;
            return true;
        }
    }
    
    return false;
}

bool ListContainsPrimitiveObject(std::vector<Object*> list, bool value, Object** foundObject)
{
    for(auto obj: list)
    {
        if(ValueMatchesPrimitiveObject(value, obj))
        {
            *foundObject = obj;
            return true;
        }
    }
    
    return false;
}