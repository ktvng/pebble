#include <cstdarg>

#include "object.h"
#include "diagnostics.h"

Reference* CreateReferenceInternal(String name, ObjectClass objClass)
{
    Reference* ref = new Reference;
    Object* obj = new Object;

    ref->Name = name;
    ref->ToObject = obj;

    obj->Class = objClass;
    
    return ref;
}

Reference* CreateReference(String name, ObjectClass objClass, int value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    int* i = new int;
    *i = value;
    ref->ToObject->Value = i;

    return ref;
}

Reference* CreateReference(String name, ObjectClass objClass, double value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    double* d = new double;
    *d = value;
    ref->ToObject->Value = d;

    return ref;
}

Reference* CreateReference(String name, ObjectClass objClass, bool value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    bool* b = new bool;
    *b = value;
    ref->ToObject->Value = b;

    return ref;
}

Reference* CreateReference(String name, ObjectClass objClass, const String value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    std::string* s = new std::string;
    *s = value;
    ref->ToObject->Value = s;
    
    return ref;
}

Reference* CreateReference(String name, Object* obj)
{
    Reference* ref = new Reference;
    ref->Name = name;
    ref->ToObject = obj;

    return ref;
}

Reference* CreateReference(const String name)
{
    static Object nullObject;
    nullObject.Class = NullClass;
    
    Reference* ref = new Reference { name, &nullObject };
    return ref;
}



bool IsNumeric(const Reference& ref)
{
    return ref.ToObject->Class == IntegerClass || ref.ToObject->Class == DecimalClass;
}

ObjectClass GetPrecedenceClass(const Object& obj1, const Object& obj2)
{
    if(obj1.Class == DecimalClass || obj2.Class == DecimalClass)
        return DecimalClass;
    return IntegerClass;
}



String GetStringValue(const Object& obj)
{
    if(obj.Class == IntegerClass)
    {
        return std::to_string(*static_cast<int*>(obj.Value));
    }
    else if(obj.Class == DecimalClass)
    {
        return std::to_string(*static_cast<double*>(obj.Value));
    }
    else if(obj.Class == BooleanClass)
    {
        if(*static_cast<bool*>(obj.Value))
        {
            return "true";
        }
        return "false";
    }
    else if(obj.Class == StringClass)
    {
        return *static_cast<String*>(obj.Value);
    }
    else if(obj.Class == NullClass)
    {
        return "null";
    }
    DebugPrint("unknown class");
    return "";
}

int GetIntValue(const Object& obj)
{
    if(obj.Class != IntegerClass)
    {
        DebugPrint("object has no integer value");
        return 0;
    }
    return *static_cast<int*>(obj.Value);
}

double GetDecimalValue(const Object& obj)
{
    if(obj.Class == DecimalClass)
    {
        return *static_cast<double*>(obj.Value);
    }
    if(obj.Class == IntegerClass)
    {
        return static_cast<double>(*static_cast<int*>(obj.Value));
    }
    DebugPrint("object has no decimal value");
    return 0;
}

bool GetBoolValue(const Object& obj)
{
    if(obj.Class == BooleanClass)
    {
        return *static_cast<bool*>(obj.Value);
    }
    else if (obj.Class == IntegerClass)
    {
        return static_cast<bool>(*static_cast<int*>(obj.Value));
    }
    else if(obj.Class == NullClass)
    {
        return false;
    }
    else
    {
        return true;
    }
}


Reference* CreateReference(String name, Token* valueToken)
{
    String value = valueToken->Content;
    bool b;

    switch(valueToken->Type)
    {
        case TokenType::Integer:
        return CreateReference(name, IntegerClass, std::stoi(value));

        case TokenType::Boolean:
        b = value == "true" ? true : false;
        return CreateReference(name, BooleanClass, b);

        case TokenType::String:
        return CreateReference(name, StringClass, value);

        case TokenType::Decimal:
        return CreateReference(name, DecimalClass, std::stod(value));

        default:
        break;
    }

    // Does not support generic objects;
    DebugPrint("Cannot create reference");
    return CreateReference(name);
}

Reference* CreateReference(Token* nameToken, Token* valueToken)
{
    return CreateReference(nameToken->Content, valueToken);
}

