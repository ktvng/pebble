
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

Reference* ReferenceConstructor()
{
    Reference* ref = new Reference; 
    ref->Name = "";
    ref->To = nullptr;

    return ref;
}

Reference* ReferenceConstructor(String refName, Object* obj)
{
    Reference* ref = new Reference; 
    ref->Name = refName;
    ref->To = obj;

    return ref;
}

void ReferenceDestructor(Reference* ref)
{
    delete ref;
}



/// Remove a reference from ObjectIndex of the global PROGRAM


/// true if [ref] is a temporary reference
bool IsTemporaryReference(Reference* ref)
{
    return ref->Name == c_temporaryReferenceName;
}



// ---------------------------------------------------------------------------------------------------------------------
// Reference matching

/// true if [name] refers to [ref]
bool NameMatchesReference(String name, Reference* ref)
{
    return name == ref->Name;
}


// ---------------------------------------------------------------------------------------------------------------------
// Reference Info

bool IsPrimitiveObject(Reference* ref)
{
    if(ref->To == nullptr)
    {
        return false;
    }
    
    return IsPrimitiveObject(ref->To);
}

bool IsPrimitiveObject(Object* obj)
{
    auto objClass = obj->Class;
    if(objClass == BooleanClass || objClass == StringClass || objClass == DecimalClass || objClass == IntegerClass)
    {
        return true;
    }

    return false;
}



// ---------------------------------------------------------------------------------------------------------------------
// ReferenceStub used for parsing

/// create a reference stub used in parsing. This is a stand-in unscoped Reference object
/// that must be resolved into a proper reference 
Reference* ReferenceStubConstructor(String refName)
{
    Reference* ref = new Reference;
    ref->Name = refName;
    ref->To = nullptr;

    return ref;
}

void ReferenceStubDestructor(Reference* ref)
{
    delete ref;
}

/// true if [ref] is a stub
bool IsReferenceStub(Reference* ref)
{
    return ObjectOf(ref) == nullptr;
}
