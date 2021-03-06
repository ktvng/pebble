#ifndef __OBJECT_H
#define __OBJECT_H

#include "abstract.h"

// ---------------------------------------------------------------------------------------------------------------------
// Objects/Methods

/// defines the class of an object which imbues type properties
typedef std::string ObjectClass;
/// emulated object in Pebble
/// [Class] refers to the Object Class which governs type properties
/// [Attributes] are references to other Objects/Methods
/// [Value] is used by primitive objects for basic operations.
///         there are currently only int*, double*, bool*, std::string* value types
class Object
{
    public:
    ObjectClass Class;
    Scope* Attributes;
    void* Value;
    Method* Action;
    Scope* DefinitionScope;

    /// used for bytecode representation
    extArg_t BlockStartInstructionId;
    ParameterList ByteCodeParamsAsMethod;
};

/// emulated method in Pebble
/// [CodeBlock] is the code associated with the method
/// [Parameters] are the parameters input to the method
class Method
{
    public:
    Block* CodeBlock;
    ParameterList ParameterNames;
};


Object* ObjectConstructor(ObjectClass cls, void* value);
Object* ObjectConstructor();
void ObjectDestructor(Object* obj);
void DeleteObject(Object* obj);

Object* NullObject();

int* ObjectValueConstructor(int value);
double* ObjectValueConstructor(double value);
bool* ObjectValueConstructor(bool value);
String* ObjectValueConstructor(String value);

void ObjectValueDestructor(ObjectClass klass, void* val);


// ---------------------------------------------------------------------------------------------------------------------
// ObjectClasses

inline const ObjectClass BaseClass = "Object";
inline const ObjectClass IntegerClass = "Integer";
inline const ObjectClass DecimalClass = "Decimal";
inline const ObjectClass StringClass = "String";
inline const ObjectClass BooleanClass = "Boolean";
inline const ObjectClass NullClass = "Nothing";
inline const ObjectClass ArrayClass = "Array";
inline const ObjectClass TupleClass = "Tuple";
inline const ObjectClass SomethingClass = "Something";
inline const ObjectClass MethodClass = "Method";

// ---------------------------------------------------------------------------------------------------------------------
// Global Object Index

bool FoundEntryInIndexOf(const Object* obj, ObjectReferenceMap** foundMap);
void IndexObject(Object* obj, Reference* ref);

void MethodDestructor(Method* m);
Method* MethodConstructor();
// ---------------------------------------------------------------------------------------------------------------------
// Access referenced objects/methods

Object* ObjectOf(const Reference* ref);


// ---------------------------------------------------------------------------------------------------------------------
// Object type handling

ObjectClass GetPrecedenceClass(const Object* obj1, const Object* obj2);
bool IsNumeric(const Reference* ref);
bool IsString(const Reference* ref);
bool IsCallable(const Reference* ref);
bool IsCallable(const Object* obj);


// ---------------------------------------------------------------------------------------------------------------------
// Value for primitive objects

std::string GetStringValue(const Object* obj);
int GetIntValue(const Object* obj);
double GetDecimalValue(const Object* obj);
bool GetBoolValue(const Object* obj);

bool ListContainsPrimitiveObject(std::vector<Object*> list, double value, Object** foundObject);
bool ListContainsPrimitiveObject(std::vector<Object*> list, int value, Object** foundObject);
bool ListContainsPrimitiveObject(std::vector<Object*> list, bool value, Object** foundObject);
bool ListContainsPrimitiveObject(std::vector<Object*> list, String& value, Object** foundObject);

#endif
