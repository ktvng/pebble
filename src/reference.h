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

Reference* ReferenceConstructor(String refName, Object* obj);
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
// ReferenceStubs

/// creates a stub reference (one that has ref->To = nullptr) with [refName]
Reference* ReferenceStubConstructor(String refName);

void ReferenceStubDestructor(Reference* ref);

/// true if [ref] is a reference stub
bool IsReferenceStub(Reference* ref);

bool IsTemporaryReference(Reference* ref);

// ---------------------------------------------------------------------------------------------------------------------
// Reference info

/// true if [ref]->To is the NullObject
bool IsNullReference(const Reference* ref);

/// true if [ref]->To is a primitive object
bool IsPrimitiveObject(Reference* ref);

/// true if [obj] is a primitive object
bool IsPrimitiveObject(Object* obj);



#endif
