
#include "reference.h"

#include "main.h"
#include "object.h"
#include "scope.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"

// ---------------------------------------------------------------------------------------------------------------------
// TODO:


void ReferenceDestructor(Reference* ref)
{
    delete ref;
}

/// Removes [ref] from [scope]
void RemoveReferenceFromCurrentScope(Reference* ref)
{
    size_t refLoc;
    for(refLoc = 0; refLoc<CurrentScope()->ReferencesIndex.size() && CurrentScope()->ReferencesIndex.at(refLoc) != ref; refLoc++);
    CurrentScope()->ReferencesIndex.erase(CurrentScope()->ReferencesIndex.begin()+refLoc);
}

/// Remove a reference from ObjectIndex of the global PROGRAM
void RemoveReferenceFromObjectIndex(Reference* ref)
{
    ObjectReferenceMap* map = EntryInIndexOf(ObjectOf(ref));
    if(map == nullptr)
    {
        LogIt(LogSeverityType::Sev3_Critical, "RemoveReferenceFromObjectIndex", "cannot find reference in ObjectIndex");
        return;
    }
    size_t refLoc;
    for(refLoc=0; refLoc<map->References.size() && ref != map->References.at(refLoc); refLoc++);
    map->References.erase(map->References.begin()+refLoc);
}

void RemoveReferenceFromMethodIndex(Reference* ref)
{
    MethodReferenceMap* map = EntryInMethodIndexOf(MethodOf(ref));
    if(map == nullptr)
    {
        LogIt(LogSeverityType::Sev3_Critical, "RemoveReferenceFromMethodIndex", "cannot find reference in ObjectIndex");
        return;
    }
    size_t refLoc;
    for(refLoc=0; refLoc<map->References.size() && ref != map->References.at(refLoc); refLoc++);
    map->References.erase(map->References.begin()+refLoc);
}

/// true if [ref] is a temporary reference
bool IsTemporaryReference(Reference* ref)
{
    return ref->Name == c_temporaryReferenceName;
}

/// removes all dependencies on [ref] and deletes [ref]
void Dereference(Reference* ref)
{
    if(!IsTemporaryReference(ref) && !IsNullReference(ref))
        return;

    LogItDebug(Msg("dereferencing: %s", ref->Name), "Dereference");

    if(ObjectOf(ref) != nullptr)
    {
        RemoveReferenceFromObjectIndex(ref);
    }
    RemoveReferenceFromCurrentScope(ref);
    ReferenceDestructor(ref);
}

/// dereferences each element of [referencesList] from [scope]
void DereferenceAll(std::vector<Reference*> referenceList)
{
    for(Reference* ref: referenceList)
    {
        Dereference(ref);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// Reference matching

/// true if [name] refers to [ref]
bool NameMatchesReference(String name, Reference* ref)
{
    return name == ref->Name;
}

/// find a reference matching [refName] located in [scope]. returns nullptr if none found
Reference* GetReference(String refName)
{
    for(Scope* lookInScope = CurrentScope(); lookInScope != nullptr; lookInScope = lookInScope->InheritedScope)
    {
        for(Reference* ref: lookInScope->ReferencesIndex)
        {
            if(NameMatchesReference(refName, ref))
                return ref;
        }
    }
    return nullptr;
}


// ---------------------------------------------------------------------------------------------------------------------
// Reference Info

/// true if [ref] points to a Method
bool IsMethod(Reference* ref)
{
    if(ref == nullptr)
        return false;
    if(MethodOf(ref) != nullptr)
        return true;
    return false;
}

/// true if [ref] points to an Object
bool IsObject(Reference* ref)
{
    if(ref == nullptr)
        return false;
    if(ObjectOf(ref) != nullptr)
        return true;
    return false;
}

bool IsPrimitiveObject(Reference* ref)
{
    if(!IsObject(ref))
        return false;
    auto objClass = ObjectOf(ref)->Class;
    if(objClass == BooleanClass || objClass == StringClass || objClass == DecimalClass || objClass == IntegerClass)
        return true;
    return false;
}


// ---------------------------------------------------------------------------------------------------------------------
// Get/Create primitive object References (private)

/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(int value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->IndexedObject;
        if(obj->Class == IntegerClass && GetIntValue(*obj) == value)
            return CreateReference(name, obj);
    }
    return CreateReferenceToNewObject(name, IntegerClass, value);
}

/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(double value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->IndexedObject;
        if(obj->Class == DecimalClass && GetDecimalValue(*obj) == value)
            return CreateReference(name, obj);
    }
    return CreateReferenceToNewObject(name, DecimalClass, value);
}

/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(bool value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->IndexedObject;
        if(obj->Class == BooleanClass && GetBoolValue(*obj) == value)
            return CreateReference(name, obj);
    }
    return CreateReferenceToNewObject(name, BooleanClass, value);
}

/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(String value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->IndexedObject;
        if(obj->Class == StringClass && GetStringValue(*obj) == value)
            return CreateReference(name, obj);
    }
    return CreateReferenceToNewObject(name, StringClass, value);
}

/// gets a reference for a primitive [token] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(Token* token, String name)
{
    if(!TokenMatchesType(token, PrimitiveTokenTypes))
        return nullptr;

    String value = token->Content;
    bool b;
    Reference* ref = nullptr;

    switch(token->Type)
    {
        case TokenType::Integer:
        ref = ReferenceForPrimitive(std::stoi(value), name);
        break;

        case TokenType::Boolean:
        b = value == "true" ? true : false;
        ref = ReferenceForPrimitive(b, name);
        break;

        case TokenType::String:
        ref = ReferenceForPrimitive(value, name);
        break;

        case TokenType::Decimal:
        ref = ReferenceForPrimitive(std::stod(value), name);
        break;

        default:
        LogIt(LogSeverityType::Sev3_Critical, "ReferenceForPrimitive", "unknown error");
    }

    AddReferenceToCurrentScope(ref);
    return ref;
}


// ---------------------------------------------------------------------------------------------------------------------
// Get/Create generic object References

/// interface for getting References from tokens. if [token] refers to a primitive object, then a reference to a primitive
/// will be returned that points to the existing primitive if possible. Otherwise the reference is looked up in the
/// current scope. Failure to find the reference returns a Null reference
Reference* ReferenceFor(Token* token, String refName)
{
    if(TokenMatchesType(token, PrimitiveTokenTypes))
    {
        return ReferenceForPrimitive(token, refName);
    }
    else if(TokenMatchesType(token, TokenType::Reference))
    {
        return GetReference(token->Content);
    }
    LogIt(Sev3_Critical, "ReferenceFor", "unknown error");
    return CreateNullReference();
}

/// gets a reference by name by looking in the current scope
Reference* ReferenceFor(String refName)
{
    return GetReference(refName);
}

/// gets a reference for a Integer primitive
Reference* ReferenceFor(String refName, int value)
{
    Reference* ref = ReferenceForPrimitive(value, refName);
    AddReferenceToCurrentScope(ref);

    return ref;
}

/// gets a reference for a Boolean primitive
Reference* ReferenceFor(String refName, bool value)
{
    Reference* ref = ReferenceForPrimitive(value, refName);
    AddReferenceToCurrentScope(ref);

    return ref;
}

/// gets a reference for a String primitive
Reference* ReferenceFor(String refName, String value)
{
    Reference* ref = ReferenceForPrimitive(value, refName);
    AddReferenceToCurrentScope(ref);

    return ref;
}

/// gets a reference for a Decimal primitive
Reference* ReferenceFor(String refName, double value)
{
    Reference* ref = ReferenceForPrimitive(value, refName);
    AddReferenceToCurrentScope(ref);

    return ref;
}

/// gets new reference named [refName] for [objClass] with [value]. This will also create
/// a corresponding new object
Reference* ReferenceFor(String refName, ObjectClass objClass, void* value)
{
    if(objClass == IntegerClass)
        return ReferenceFor(c_returnReferenceName, *static_cast<int*>(value));
    else if(objClass == DecimalClass)
        return ReferenceFor(c_returnReferenceName, *static_cast<double*>(value));
    else if(objClass == BooleanClass)
        return ReferenceFor(c_returnReferenceName, *static_cast<bool*>(value));
    else if(objClass == StringClass)
        return ReferenceFor(c_returnReferenceName, *static_cast<String*>(value));

    Reference* ref = CreateReferenceToNewObject(refName, objClass, value);
    AddReferenceToCurrentScope(ref);

    return ref;
}

/// defines a new reference named [refName] to [referable]
Reference* ReferenceFor(String refName, Referable* refable)
{
    Reference* ref = CreateReference(refName, refable);
    AddReferenceToCurrentScope(ref);
    return ref;
}

/// reassign an existing reference [ref] to [to]
void ReassignReference(Reference* ref, Referable* to)
{
    if(ObjectOf(ref) != nullptr)
    {
        RemoveReferenceFromObjectIndex(ref);
    }
    else
    {
        RemoveReferenceFromMethodIndex(ref);
    }

    if(to->Type == ReferableType::Object)
    {
        IndexObject(static_cast<Object*>(to), ref);
    }
    else
    {
        IndexMethod(static_cast<Method*>(to), ref);
    }

    ref->To = to;
}

// ---------------------------------------------------------------------------------------------------------------------
// ReferenceStub used for parsing

/// create a reference stub used in parsing. This is a stand-in unscoped Reference object
/// that must be resolved into a proper reference 
Reference* ReferenceStub(String refName)
{
    Reference* ref = new Reference;
    ref->Name = refName;
    ref->To = nullptr;

    return ref;
}

/// true if [ref] is a stub
bool IsReferenceStub(Reference* ref)
{
    return ObjectOf(ref) == nullptr && MethodOf(ref) == nullptr;
}


// ---------------------------------------------------------------------------------------------------------------------
// Get/Create null references

/// create a new reference named [refName] to the NullObject
Reference* NullReference(String refName)
{
    Reference* ref = CreateNullReference(refName);
    AddReferenceToCurrentScope(ref);
    return ref;
}

/// true if [ref] is a null reference
bool IsNullReference(const Reference* ref)
{
    return (ObjectOf(ref) != nullptr && ObjectOf(ref) == NullObject());
}

/// reassign an existing reference [ref] to NullObject
void AssignToNull(Reference* ref)
{
    RemoveReferenceFromObjectIndex(ref);
    IndexObject(NullObject(), ref);
    ref->To = NullObject();
}
