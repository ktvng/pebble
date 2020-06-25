#ifndef __REFERENCE_H
#define __REFERENCE_H

#include "main.h"
#include "arch.h"
#include "program.h"
#include "token.h"

void EnterScope(Scope* newScope);
Scope* GetCurrentScope();
void ExitScope(); 
void ClearScope();

void Dereference(Reference* ref);
void DereferenceAll(std::vector<Reference*> referenceList);

void ReassignReference(Reference* ref, Object* newObj);

// for arrays and general objects
Reference* ReferenceFor(String refName, ObjectClass objClass, void* value);

// for primtiives
Reference* ReferenceFor(String refName, int value);
Reference* ReferenceFor(String refName, bool value);
Reference* ReferenceFor(String refName, double value);
Reference* ReferenceFor(String refName, String value);

Reference* ReferenceFor(String refName, Object* obj);
Reference* ReferenceFor(String refName, Method* method);

// for null
Reference* NullReference(String refName = c_returnReferenceName);

void AddReferenceToCurrentScope(Reference* ref);


Reference* ReferenceFor(Token* token, String refName = c_returnReferenceName);
Reference* ReferenceFor(String refName);
void AssignToNull(Reference* ref);

// reference for primitives only
Reference* ReferenceForPrimitive(Token* token, String name);

// creates a stub that refers to nullptrs
Reference* ReferenceStub(String refName);
bool IsReferenceStub(Reference* ref);

#endif