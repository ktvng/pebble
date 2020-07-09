#include <cstdarg>
#include <iostream>

#include "object.h"
#include "diagnostics.h"
#include "program.h"
#include "reference.h"
#include "scope.h"
#include "token.h"

Reference* ReferenceConstructor()
{
    // LogItDebug("space allocated for new reference", "ReferenceConstructor");
    Reference* ref = new Reference; 
    ref->Name = "";
    ref->To = nullptr;

    return ref;
}

Object* ObjectConstructor()
{
    // LogItDebug("space allocated for new object", "ObjectConstructor");
    Object* obj = new Object;
    obj->Attributes = ScopeConstructor(CurrentScope());
    obj->Class = NullClass;
    obj->Value = nullptr;
    obj->Action = nullptr;

    return obj;
}

void ObjectDestructor(Object* obj)
{
    ScopeDestructor(obj->Attributes);
    if(IsCallable(obj))
    {
        MethodDestructor(obj->Action);
    }
    delete obj;
}


// ---------------------------------------------------------------------------------------------------------------------
// Methods

/// constructor for a Method* object with [inheritedScope]
Method* MethodConstructor(Scope* inheritedScope)
{
    Method* m = new Method;
    m->CodeBlock = nullptr;
    m->ParameterNames = {};

    return m;
}

void MethodDestructor(Method* m)
{
    DeleteBlockRecursive(m->CodeBlock);
    delete m;
}



Object* ObjectOf(const Reference* ref)
{
    if(ref->To == nullptr)
        return nullptr;

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





bool ObjectHasReference(const ObjectReferenceMap& map, const Reference* ref)
{
    for(Reference* objRef: map.References)
    {
        if(ref == objRef)
            return true;
    }
    return false;
}

/// returns the ObjectReferenceMap corresonding to [obj] or nullptr if not found
bool FoundEntryInIndexOf(const Object* obj, ObjectReferenceMap** foundMap)
{
    for(ObjectReferenceMap& map: PROGRAM->ObjectsIndex)
    {
        if(map.IndexedObject == obj)
        {
            *foundMap = &map;            
            return true;
        }
    }
    foundMap = nullptr;
    return false;
}

void IndexObject(Object* obj, Reference* ref)
{
    ObjectReferenceMap* map = nullptr;
    if(FoundEntryInIndexOf(obj, &map))
    {
        map->References.push_back(ref);
    }
    else
    {
        std::vector<Reference*> refs = { ref };
        
        ObjectReferenceMap objMap = { obj, refs };
        PROGRAM->ObjectsIndex.push_back(objMap);
    }

}

Reference* CreateReferenceInternal(String name, ObjectClass objClass)
{
    Reference* ref = ReferenceConstructor();
    Object* obj = ObjectConstructor();

    IndexObject(obj, ref);

    ref->Name = name;
    ref->To = obj;

    obj->Class = objClass;
    obj->Value = nullptr;
    
    return ref;
}


Reference* CreateReferenceToArrayObject(String name, ObjectClass objClass, int value){
    Reference* ref = CreateReferenceInternal(name, objClass);

    int* i = new int;
    *i = value;

    // TODO: Make arrays
    // ref->ToObject->Value = i;
    // ref->ToObject->Attributes.reserve(value);


    return ref;
}


Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, int value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    int* i = new int;
    *i = value;
    ObjectOf(ref)->Value = i;

    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, double value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    double* d = new double;
    *d = value;
    ObjectOf(ref)->Value = d;

    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, bool value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    bool* b = new bool;
    *b = value;
    ObjectOf(ref)->Value = b;

    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, const String value)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    std::string* s = new std::string;
    *s = value;
    ObjectOf(ref)->Value = s;
    
    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, void* value, Scope* methodInheritedScope)
{
    Reference* ref = CreateReferenceInternal(name, objClass);
    ObjectOf(ref)->Value = value;
    
    auto method = MethodConstructor(methodInheritedScope);
    ObjectOf(ref)->Action = method;

    return ref;
}

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, void* value){
    if(objClass == StringClass)
    {
        return CreateReferenceToNewObject(name, objClass, *static_cast<String*>(value));
    }
    else if(objClass == DecimalClass)
    {
        return CreateReferenceToNewObject(name, objClass, *static_cast<double*>(value));
    }
    else if(objClass == BooleanClass)
    {
        return CreateReferenceToNewObject(name, objClass, *static_cast<bool*>(value));
    }
    else if(objClass == IntegerClass)
    {
        return CreateReferenceToNewObject(name, objClass, *static_cast<int*>(value));
    }
    else if(objClass == ArrayClass)
    {
        return CreateReferenceToArrayObject(name, objClass, *static_cast<int*>(value));
    }
    else if(objClass == TupleClass)
    {
        return CreateReferenceInternal(name, TupleClass);
    }
    else if(objClass == BaseClass)
    {
        return CreateReferenceInternal(name, BaseClass);
    }
    else 
    {
        return CreateReferenceInternal(name, objClass);
    }
}



Reference* CreateReference(String name, Object* obj)
{
    Reference* ref = ReferenceConstructor();
    IndexObject(obj, ref);

    ref->Name = name;
    ref->To = obj;

    return ref;
}


Object* NullObject()
{
    static Object nullObject;
    static Scope nullScope;
    nullObject.Class = NullClass;
    nullObject.Value = nullptr;
    nullObject.Attributes = &nullScope;

    return &nullObject;
}

Reference* CreateNullReference(String name)
{
    Object* nullObject = NullObject();
    Reference* ref = ReferenceConstructor();
    ref->Name = name;
    ref->To = nullObject;

    IndexObject(nullObject, ref);
    
    return ref;
}

Reference* CreateNullReference()
{
    return CreateNullReference(c_temporaryReferenceName);
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
        return "Nothing";
    }
    else if(obj.Class == BaseClass)
    {
        return "Object";
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

