#ifndef __REFERENCE_H
#define __REFERENCE_H

#include "main.h"
#include "arch.h"
#include "program.h"
#include "token.h"


// ---------------------------------------------------------------------------------------------------------------------
// Scoping

void EnterScope(Scope* newScope);
Scope* CurrentScope();
void ExitScope(); 
void ClearScope();
void AddReferenceToCurrentScope(Reference* ref);

// ---------------------------------------------------------------------------------------------------------------------
// Dereferencing 

void Dereference(Reference* ref);
void DereferenceAll(std::vector<Reference*> referenceList);

void ReassignReference(Reference* ref, Referable* to);
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

Reference* ReferenceStub(String refName);
bool IsReferenceStub(Reference* ref);


// ---------------------------------------------------------------------------------------------------------------------
// Reference info

bool IsMethod(Reference* ref);
bool IsObject(Reference* ref);

#endif
