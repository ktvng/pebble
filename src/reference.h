#ifndef __REFERENCE_H
#define __REFERENCE_H

#include "abstract.h"




// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// these are used as names to references either  things (either object or methods)
/// [Name] is the name which is used to call the Reference
/// [To] a Object (Method/Object) 
struct Reference
{
    std::string Name;
    Object* To = nullptr;
};

void ReferenceDestructor(Reference* ref);

// ---------------------------------------------------------------------------------------------------------------------
// Constants

/// used when an operation returns a temporary reference. will only have lifespan of 1-2 lines of code, often less
/// as it will be dereferenced if it is an operand to a parent operation
inline const std::string c_temporaryReferenceName = "!TemporaryReference";

/// used when returning values by OperationType::Return
inline const std::string c_returnReferenceName = "!ReturnedReference";

/// used for primitive objects resolved during compile time and incorporated as constants in an operation tree
inline const std::string c_operationReferenceName = "!primitive:";

inline const std::string c_nullStubName = "Nothing";


// ---------------------------------------------------------------------------------------------------------------------
// Dereferencing 

void RemoveReferenceFromObjectIndex(Reference* ref);

/// removes all mentions of [ref] in the current scope and in the ObjectIndex and destroys [ref]
void Dereference(Reference* ref);

/// dereferences all References* in [referencesList]
void DereferenceAll(std::vector<Reference*> referenceList);


/// changes [ref]->To into [to] and updates all dependencies
void ReassignReference(Reference* ref, Object* to);

/// changes [ref->To] into the NullObject and updates all dependencies
void AssignToNull(Reference* ref);


// ---------------------------------------------------------------------------------------------------------------------
// Getting references

// for arrays and general objects
Reference* ReferenceFor(String refName, ObjectClass objClass, void* value);

// for primtiives
Reference* ReferenceFor(String refName, int value);
Reference* ReferenceFor(String refName, bool value);
Reference* ReferenceFor(String refName, double value);
Reference* ReferenceFor(String refName, String value);

// for existing object
Reference* ReferenceFor(String refName, Object* refable);
Reference* ReferenceFor(String refName);
Reference* ReferenceForInImmediateScope(String refName, Scope* scope);

// for null
Reference* NullReference(String refName = c_temporaryReferenceName);

// reference from tokens
Reference* ReferenceForPrimitive(Token* token, String name);
Reference* ReferenceFor(Token* token, String refName = c_temporaryReferenceName);

Reference* GetReference(String refName);

// ---------------------------------------------------------------------------------------------------------------------
// ReferenceStubs

/// creates a stub reference (one that has ref->To = nullptr) with [refName]
Reference* ReferenceStubConstructor(String refName);

void ReferenceStubDestructor(Reference* ref);

/// true if [ref] is a reference stub
bool IsReferenceStub(Reference* ref);

bool IsTemporaryReference(Reference* ref);

// ---------------------------------------------------------------------------------------------------------------------
// Reference info

/// true if [ref]->To is an object
bool IsObject(Reference* ref);

/// true if [ref]->To is the NullObject
bool IsNullReference(const Reference* ref);

/// true if [ref]->To is a primitive object
bool IsPrimitiveObject(Reference* ref);


#endif
