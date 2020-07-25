#ifndef __ASTVM_H
#define __ASTVM_H

#include "abstract.h"

/// used to keep track of objects and their references
struct ObjectReferenceMap
{
    Object* IndexedObject;
    std::vector<Reference*> References;
};

// extern std::vector<ObjectReferenceMap> ObjectsIndex;




// typedef Reference* (*OperationEvaluator)(Reference*, std::vector<Reference*>&);
// extern OperationEvaluator OperationEvaluators[];

// Reference* DoOperation(Operation* op);
// Reference* DoBlock(Block* codeBlock, Scope* scope=nullptr);
// void DoProgram(Program* program);

// // ---------------------------------------------------------------------------------------------------------------------
// // Dereferencing 

// void RemoveReferenceFromObjectIndex(Reference* ref);

// /// removes all mentions of [ref] in the current scope and in the ObjectIndex and destroys [ref]
// void Dereference(Reference* ref);

// /// dereferences all References* in [referencesList]
// void DereferenceAll(std::vector<Reference*> referenceList);


// /// changes [ref]->To into [to] and updates all dependencies
// void ReassignReference(Reference* ref, Object* to);

// /// changes [ref->To] into the NullObject and updates all dependencies
// void AssignToNull(Reference* ref);


// // ---------------------------------------------------------------------------------------------------------------------
// // Getting references

// // for arrays and general objects
// Reference* ReferenceFor(String refName, ObjectClass objClass, void* value);

// // for primtiives
// Reference* ReferenceFor(String refName, int value);
// Reference* ReferenceFor(String refName, bool value);
// Reference* ReferenceFor(String refName, double value);
// Reference* ReferenceFor(String refName, String value);

// // for existing object
// Reference* ReferenceFor(String refName, Object* refable);
// Reference* ReferenceFor(String refName);
// Reference* ReferenceForInImmediateScope(String refName, Scope* scope);

// // for null
// Reference* NullReference(String refName = c_temporaryReferenceName);

// // reference from tokens
// Reference* ReferenceFor(Token* token, String refName = c_temporaryReferenceName);

// Reference* GetReference(String refName);




// /// moved from OBJ

// // ---------------------------------------------------------------------------------------------------------------------
// // Create references and associated (primitive) object

// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, bool value);
// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, int value);
// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, const String value);
// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, double value);
// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, void* value);

// // ---------------------------------------------------------------------------------------------------------------------
// // Create references to existing Object

// Reference* CreateReference(String name, Object* obj);


// // ---------------------------------------------------------------------------------------------------------------------
// // Create reference to NullObject (Nothing)

// Reference* CreateNullReference();
// Reference* CreateNullReference(String name);
// Object* NullObject();


// // ---------------------------------------------------------------------------------------------------------------------
// // Create reference from Token 

// Reference* CreateReferenceToNewObject(String name, Token* valueToken);
// Reference* CreateReferenceToNewObject(Token* nameToken, Token* valueToken);


// bool ScopeStackIsEmpty();


// void WipeScope(Scope* scope);

// /// add [newscope] to the top of the scope stack so all reference matching is done inside [newscope] 
// void EnterScope(Scope* newScope);

// /// returns the active scope which is the top of the scope stack
// Scope* CurrentScope();

// /// pops the top of the scope stack to return to the previous scope
// void ExitScope(bool andDestroy = false); 

// /// adds [ref] to the ReferencesIndex of the current scope
// void AddReferenceToCurrentScope(Reference* ref);

#endif
