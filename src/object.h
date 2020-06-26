#ifndef __OBJECT_H
#define __OBJECT_H

#include "main.h"
#include "arch.h"

// ---------------------------------------------------------------------------------------------------------------------
// Global Object Index

ObjectReferenceMap* EntryInIndexOf(const Object* obj);
void IndexObject(Object* obj, Reference* ref);


// ---------------------------------------------------------------------------------------------------------------------
// Access referenced objects/methods

Object* ObjectOf(const Reference* ref);
Method* MethodOf(const Reference* ref);


// ---------------------------------------------------------------------------------------------------------------------
// Create references and associated (primitive) object

Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, bool value);
Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, int value);
Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, const String value);
Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, double value);
Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, void* value);


// ---------------------------------------------------------------------------------------------------------------------
// Create references to existing Referable (object/method)

Reference* CreateReference(String name, Referable* refable);

// ---------------------------------------------------------------------------------------------------------------------
// Create reference to NullObject (Nothing)

Reference* CreateNullReference();
Reference* CreateNullReference(String name);
Object* NullObject();

// ---------------------------------------------------------------------------------------------------------------------
// Create reference from Token 

Reference* CreateReferenceToNewObject(String name, Token* valueToken);
Reference* CreateReferenceToNewObject(Token* nameToken, Token* valueToken);


// ---------------------------------------------------------------------------------------------------------------------
// Object type handling

ObjectClass GetPrecedenceClass(const Object& obj1, const Object& obj2);
bool IsNumeric(const Reference* ref);
bool IsString(const Reference* ref);


// ---------------------------------------------------------------------------------------------------------------------
// Value for primitive objects

std::string GetStringValue(const Object& obj);
int GetIntValue(const Object& obj);
double GetDecimalValue(const Object& obj);
bool GetBoolValue(const Object& obj);

#endif