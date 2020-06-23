
#include "reference.h"
#include "object.h"


// ---------------------------------------------------------------------------------------------------------------------
// Scoping

static Scope* CurrentScope;

void SetScope(Scope* newScope)
{
    LogItDebug("changed scope", "SetScope");
    CurrentScope = newScope;
}

Scope* GetCurrentScope()
{
    return CurrentScope;
}


/// Remove a reference from ObjectIndex of the global PROGRAM
void RemoveReferenceFromObjectIndex(Reference* ref)
{
    ObjectReferenceMap* map = EntryInIndexOf(ref->ToObject);
    if(map == nullptr)
    {
        LogIt(LogSeverityType::Sev3_Critical, "RemoveReferenceFromObjectIndex", "cannot find reference in ObjectIndex");
        return;
    }
    size_t refLoc;
    for(refLoc=0; refLoc<map->References.size() && ref != map->References.at(refLoc); refLoc++);
    map->References.erase(map->References.begin()+refLoc);
}

/// Removes [ref] from [scope]
void RemoveReferenceFromCurrentScope(Reference* ref)
{
    size_t refLoc;
    for(refLoc = 0; refLoc<CurrentScope->ReferencesIndex.size() && CurrentScope->ReferencesIndex.at(refLoc) != ref; refLoc++);
    CurrentScope->ReferencesIndex.erase(CurrentScope->ReferencesIndex.begin()+refLoc);
}

/// removes all dependencies on [ref] and deletes [ref]
void Dereference(Reference* ref)
{
    LogItDebug(MSG("dereferencing: %s", ref->Name), "Dereference");

    // TODO 
    if(ref->ToObject != nullptr)
    {
        RemoveReferenceFromObjectIndex(ref);
    }
    RemoveReferenceFromCurrentScope(ref);
    delete ref;
}

/// dereferences each element of [referencesList] from [scope]
void DereferenceAll(std::vector<Reference*> referenceList)
{
    for(Reference* ref: referenceList)
    {
        if(ref->Name == c_returnReferenceName)
        {
            Dereference(ref);
        }
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
    for(Scope* lookInScope = CurrentScope; lookInScope != nullptr; lookInScope = lookInScope->InheritedScope)
    {
        for(Reference* ref: lookInScope->ReferencesIndex)
        {
            if(NameMatchesReference(refName, ref))
                return ref;
        }
    }
    ReportCompileMsg(SystemMessageType::Exception, MSG("cannot resolve reference [%s]", refName));
    return nullptr;
}

bool IsMethod(Reference* ref)
{
    if(ref == nullptr)
        return false;
    if(ref->ToMethod != nullptr)
        return true;
    return false;
}




/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(int value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->Object;
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
        Object* obj = map->Object;
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
        Object* obj = map->Object;
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
        Object* obj = map->Object;
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



/// interface to getting, making, and assigning references from tokens
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


Reference* ReferenceFor(String refName, int value)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(value, refName);

    AddReferenceToCurrentScope(ref);

    return ref;
}

Reference* ReferenceFor(String refName, bool value)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(value, refName);

    AddReferenceToCurrentScope(ref);

    return ref;
}

Reference* ReferenceFor(String refName, String value)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(value, refName);

    AddReferenceToCurrentScope(ref);

    return ref;
}

Reference* ReferenceFor(String refName, double value)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(value, refName);

    AddReferenceToCurrentScope(ref);

    return ref;
}

Reference* ReferenceFor(String refName, ObjectClass objClass, void* value)
{
    Reference* ref = CreateReferenceToNewObject(refName, objClass, value);
    AddReferenceToCurrentScope(ref);

    return ref;
}

/// defines a new reference with name attribute [refName] to [obj] inside [scope] 
Reference* ReferenceFor(String refName, Object* obj)
{
    Reference* ref = CreateReference(refName, obj);
    AddReferenceToCurrentScope(ref);
    return ref;
}

Reference* ReferenceFor(String refName, Method* method)
{
    Reference* ref = CreateReference(refName, method);
    AddReferenceToCurrentScope(ref);
    return ref;
}

Reference* NullReference(String refName)
{
    Reference* ref = CreateNullReference(refName);
    AddReferenceToCurrentScope(ref);
    return ref;
}



/// add [ref] to [scope]
void AddReferenceToCurrentScope(Reference* ref)
{
    CurrentScope->ReferencesIndex.push_back(ref);
}

void ReassignReference(Reference* ref, Object* newObj)
{
    RemoveReferenceFromObjectIndex(ref);
    IndexObject(newObj, ref);
    ref->ToObject = newObj;
}

void AssignToNull(Reference* ref)
{
    RemoveReferenceFromObjectIndex(ref);
    IndexObject(NullObject(), ref);
    ref->ToObject = NullObject();
}