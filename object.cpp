#include "object.h"
#include "diagnostics.h"

Reference* MakeGeneric(String name, ObjectClass objClass)
{
    Reference* ref = new Reference;
    Object* obj = new Object;

    ref->Name = name;
    ref->ToObject = obj;

    obj->Class = objClass;
    
    return ref;
}

Reference* Make(String name, ObjectClass objClass, int value)
{
    Reference* ref = MakeGeneric(name, objClass);
    int* i = new int;
    *i = value;
    ref->ToObject->Value = i;

    return ref;
}

Reference* Make(String name, ObjectClass objClass, double value)
{
    Reference* ref = MakeGeneric(name, objClass);
    double* d = new double;
    *d = value;
    ref->ToObject->Value = d;

    return ref;
}

Reference* Make(String name, ObjectClass objClass, bool value)
{
    Reference* ref = MakeGeneric(name, objClass);
    bool* b = new bool;
    *b = value;
    ref->ToObject->Value = b;

    return ref;
}

Reference* Make(String name, ObjectClass objClass, const String value)
{
    Reference* ref = MakeGeneric(name, objClass);
    std::string* s = new std::string;
    *s = value;
    ref->ToObject->Value = s;
    
    return ref;
}

Reference* Make(String name, Object* obj)
{
    Reference* ref = new Reference;
    ref->Name = name;
    ref->ToObject = obj;

    return ref;
}

Reference* Make(const String name)
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
