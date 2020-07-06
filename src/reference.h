#ifndef __REFERENCE_H
#define __REFERENCE_H

#include "main.h"
#include "program.h"
#include "token.h"
#include "scope.h"



// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// these are used as names to references either Referable things (either object or methods)
/// [Name] is the name which is used to call the Reference
/// [To] a Referable (Method/Object) 
struct Reference
{
    std::string Name;
    Referable* To = nullptr;
};


// ---------------------------------------------------------------------------------------------------------------------
// Constants

/// used when an operation returns a temporary reference. will only have lifespan of 1-2 lines of code, often less
/// as it will be dereferenced if it is an operand to a parent operation
inline const std::string c_temporaryReferenceName = "!TemporaryReference";

/// used when returning values by OperationType::Return
inline const std::string c_returnReferenceName = "!ReturnedReference";

/// used for primitive objects resolved during compile time and incorporated as constants in an operation tree
inline const std::string c_operationReferenceName = "!ConstPrimitive";


// ---------------------------------------------------------------------------------------------------------------------
// Dereferencing 

/// removes all mentions of [ref] in the current scope and in the ObjectIndex and destroys [ref]
void Dereference(Reference* ref);

/// dereferences all References* in [referencesList]
void DereferenceAll(std::vector<Reference*> referenceList);


/// changes [ref]->To into [to] and updates all dependencies
void ReassignReference(Reference* ref, Referable* to);

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
Reference* ReferenceFor(String refName, Referable* refable);
Reference* ReferenceFor(String refName);

// for null
Reference* NullReference(String refName = c_temporaryReferenceName);

// reference from tokens
Reference* ReferenceForPrimitive(Token* token, String name);
Reference* ReferenceFor(Token* token, String refName = c_temporaryReferenceName);



// ---------------------------------------------------------------------------------------------------------------------
// ReferenceStubs

/// creates a stub reference (one that has ref->To = nullptr) with [refName]
Reference* ReferenceStub(String refName);

/// true if [ref] is a reference stub
bool IsReferenceStub(Reference* ref);


// ---------------------------------------------------------------------------------------------------------------------
// Reference info

/// true if [ref]->To is a method
bool IsMethod(Reference* ref);

/// true if [ref]->To is an object
bool IsObject(Reference* ref);

/// true if [ref]->To is the NullObject
bool IsNullReference(const Reference* ref);

/// true if [ref]->To is a primitive object
bool IsPrimitiveObject(Reference* ref);


#endif
