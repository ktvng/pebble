#include <cstdarg>

#include "object.h"
#include "diagnostics.h"

bool ObjectHasReference(const ObjectReferenceMap* map, const Reference* ref)
{
    for(Reference* objRef: map->References)
    {
        if(ref == objRef)
            return true;
    }
    return false;
}

ObjectReferenceMap* EntryInIndexOf(const Object* obj)
{
    for(ObjectReferenceMap* map: ObjectsIndex)
    {
        if(map->Object == obj)
            return map;
    }
    return nullptr;
}

void IndexObject(Object* obj, Reference* ref)
{
    ObjectReferenceMap* map = EntryInIndexOf(obj);

    if(map == nullptr)
    {
        ObjectReferenceMap* objMap = new ObjectReferenceMap;
        std::vector<Reference*> refs = { ref };
        
        *objMap = ObjectReferenceMap{ obj, refs };
        ObjectsIndex.push_back(objMap);
        
        return;
    }
    
    map->References.push_back(ref);
}

Reference* CreateReferenceInternal(String name, ObjectClass objClass)
{
    Reference* ref = new Reference;
    Object* obj = new Object;
    IndexObject(obj, ref);

    ref->Name = name;
    ref->ToObject = obj;

    obj->Class = objClass;
    
    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, int value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    int* i = new int;
    *i = value;
    ref->ToObject->Value = i;

    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, double value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    double* d = new double;
    *d = value;
    ref->ToObject->Value = d;

    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, bool value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    bool* b = new bool;
    *b = value;
    ref->ToObject->Value = b;

    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, const String value)
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
    IndexObject(obj, ref);

    ref->Name = name;
    ref->ToObject = obj;

    return ref;
}

Reference* CreateNullReference()
{
    static Object nullObject;
    nullObject.Class = NullClass;
    
    Reference* ref = new Reference { c_returnReferenceName, &nullObject };
    IndexObject(&nullObject, ref);
    
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
    LogIt(LogSeverityType::Sev1_Notify, "GetStringValue", "unimplemented for Reference type and generic objects");
    return "";
}

int GetIntValue(const Object& obj)
{
    if(obj.Class != IntegerClass)
    {
        LogIt(LogSeverityType::Sev1_Notify, "GetIntValue", "only implemented for IntegerClass");
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
    LogIt(LogSeverityType::Sev1_Notify, "GetDecimalValue", "only implemented for Integer and Decimal classes");
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


Reference* CreateReferenceToNewObject(String name, Token* valueToken)
{
    String value = valueToken->Content;
    bool b;

    switch(valueToken->Type)
    {
        case TokenType::Integer:
        return CreateReferenceToNewObject(name, IntegerClass, std::stoi(value));

        case TokenType::Boolean:
        b = value == "true" ? true : false;
        return CreateReferenceToNewObject(name, BooleanClass, b);

        case TokenType::String:
        return CreateReferenceToNewObject(name, StringClass, value);

        case TokenType::Decimal:
        return CreateReferenceToNewObject(name, DecimalClass, std::stod(value));

        case TokenType::Reference:
        default:
        LogIt(LogSeverityType::Sev1_Notify, "CreateReferenceToNewObject", "unimplemented in this case (generic References/Simple)");
        return CreateNullReference();
    }
}

Reference* CreateReferenceToNewObject(Token* nameToken, Token* valueToken)
{
    return CreateReferenceToNewObject(nameToken->Content, valueToken);
}

