// #include "astvm.h"

// #include <iostream>

// #include "main.h"
// #include "diagnostics.h"
// #include "scope.h"
// #include "operation.h"
// #include "reference.h"
// #include "program.h"
// #include "object.h"

// // ---------------------------------------------------------------------------------------------------------------------
// // Diagnostics

// /// logs scope
// void LogDiagnosticsForRuntimeLine(Scope* scope, Operation* op)
// {
//     for(; scope != nullptr; scope=scope->InheritedScope)
//         for(auto ref: scope->ReferencesIndex)
//             LogDiagnostics(ref, Msg("scope before %s line %i", ToString(op->Type), op->LineNumber), "DoOperationOnReferences");
// }



// // ---------------------------------------------------------------------------------------------------------------------
// // Program execution

// Block* AsBlock(Executable* exec)
// {
//     return static_cast<Block*>(exec);
// }

// Operation* AsOperation(Executable* exec)
// {
//     return static_cast<Operation*>(exec);
// }

// /// executes an operation [op] on the ordered list of references [operands] inside [scope]
// /// note: this method should only be called through DoOperation
// Reference* DoOperationOnReferences(Operation* op, std::vector<Reference*> operands)
// {
//     return OperationEvaluators[op->Type](op->Value, operands);
// }

// /// resolve the references return by each operand operation
// std::vector<Reference*> GetOperandReferences(Operation* op)
// {
//     std::vector<Reference*> operandReferences;
    
//     for(Operation* operand: op->Operands)
//     {
//         Reference* operandRef = DoOperation(operand);
//         operandReferences.push_back(operandRef);
//     }
//     return operandReferences;
// }

// /// executes an operation [op] inside [scope]
// Reference* DoOperation(Operation* op)
// {
//     std::vector<Reference*> operandReferences = GetOperandReferences(op);

//     Reference* returnRef = DoOperationOnReferences(op, operandReferences);
//     LogItDebug(Msg("line[%i] operation %s returned a reference", op->LineNumber, ToString(op->Type)), "DoOperation");
//     return returnRef;
// }

// void UpdatePreviousResult(Reference** result, Reference** previousResult)
// {
//     if(*previousResult != nullptr)
//         Dereference(*previousResult);
//     *previousResult = *result;
//     PROGRAM->That = *result;
// }

// /// handles the If operation
// Reference* HandleControlFlowIf(Operation* op, size_t& execLine)
// {
//     Reference* ifExpressionResult = DoOperation(op);
//     bool ifIsTrue = GetBoolValue(ObjectOf(ifExpressionResult));
//     if(op->Type == OperationType::If && !ifIsTrue)
//     {
//         execLine++;
//     }
//     return ifExpressionResult;
// }

// /// handles the while operation
// Reference* HandleControlFlowWhile(Operation* op, size_t& execLine, Block* codeBlock)
// {
//     Reference* whileExpression = DoOperation(op);
//     bool condition = GetBoolValue(ObjectOf(whileExpression));
//     if(op->Type == OperationType::While && !condition)
//     {
//         execLine++;
//     } else if(op->Type == OperationType::While && condition){
//         if(codeBlock->Executables.size() > execLine + 1 && codeBlock->Executables.at(execLine+1)->ExecType == ExecutableType::Block)
//         {
//             auto ref = DoBlock(static_cast<Block *> (codeBlock->Executables.at(execLine+1)));
//             Dereference(ref);
//             execLine -= 1;
//         }
//     }
//     return whileExpression;
// }

// Reference* HandleControlFlowClass(Operation* op, size_t& execLine, Block* block)
// {
//     /// TODO: currently expects a block
//     if(!(execLine + 1 < block->Executables.size() && block->Executables[execLine+1]->ExecType == ExecutableType::Block))
//     {
//         ReportRuntimeMsg(SystemMessageType::Exception, "no block after class");
//         return NullReference();                
//     }

//     Reference* newClass = DoOperation(op);
    
//     auto classBlock = AsBlock(block->Executables[++execLine]);
//     auto classScope = ObjectOf(newClass)->Attributes;
//     DoBlock(classBlock, classScope);

//     return newClass;
// }

// Reference* HandleControlFlowDefineMethod(Operation* op, size_t& execline, Block* block)
// {
//     /// TODO: currently assumes a block
//     if(!(execline + 1 < block->Executables.size() && block->Executables[execline+1]->ExecType == ExecutableType::Block))
//     {
//         ReportRuntimeMsg(SystemMessageType::Exception, "no block after method definition");
//         return NullReference();
//     }

//     Reference* method = DoOperation(op);

//     auto methodBlock = AsBlock(block->Executables[++execline]);
//     method->To->Action->CodeBlock = methodBlock;

//     return method;
// }
// /// executes [op] in [scope] and updates [execLine] based on the control flow properties
// /// of [op]
// Reference* HandleControlFlow(Operation* op, size_t& execLine, Block* block)
// {
//     switch(op->Type)
//     {
//         case OperationType::If:
//         return HandleControlFlowIf(op, execLine);

//         case OperationType::Class:
//         return HandleControlFlowClass(op, execLine, block);

//         case OperationType::While:
//         return HandleControlFlowWhile(op, execLine, block);

//         case OperationType::DefineMethod:
//         return HandleControlFlowDefineMethod(op, execLine, block);
//         // for any non-control flow operation;
//         default:
//         return DoOperation(op);
//     }
// }

// void HandleRuntimeMessages(int lineNumber)
// {
//     if(RuntimeMsgFlag)
//     {
//         RuntimeMsgPrint(lineNumber);
//         RuntimeMsgFlag = false;
//     }
// }

// bool ProgramFinished()
// {
//     return ScopeStackIsEmpty();
// }


// bool shouldReturn = false;

// /// executes the commands contained in a [codeBlock]
// Reference* DoBlock(Block* codeBlock, Scope* scope)
// {
//     Reference* result = nullptr;
//     Reference* previousResult = nullptr;

//     shouldReturn = false;
//     bool scopeIsLocal = false;
//     if(scope == nullptr)
//     {
//         scopeIsLocal = true;
//         scope = ScopeConstructor(CurrentScope());
//     }
    
//     EnterScope(scope);
//     {
//         for(size_t i=0; i<codeBlock->Executables.size(); i++)
//         {
//             auto exec = codeBlock->Executables.at(i);

//             if(exec->ExecType == ExecutableType::Operation)
//             {
//                 Operation* op = AsOperation(exec); 

//                 // LogDiagnosticsForRuntimeLine(CurrentScope(), op);

//                 LogItDebug(Msg("starting execute line [%i] which is %s", op->LineNumber, ToString(op->Type)), "DoBlock");

//                 result = HandleControlFlow(op, i, codeBlock); 
//                 UpdatePreviousResult(&result, &previousResult);
//                 HandleRuntimeMessages(op->LineNumber);
                
//                 LogItDebug(Msg("finishes execute line [%i]", op->LineNumber), "DoBlock");

//                 if(op->Type == OperationType::Return)
//                 {
//                     shouldReturn = true;
//                     break;
//                 }
//             }
//             else if(exec->ExecType == ExecutableType::Block)
//             {
//                 LogItDebug("discovered child block: starting", "DoBlock");

//                 result = DoBlock(AsBlock(exec));
//                 UpdatePreviousResult(&result, &previousResult);

//                 LogItDebug("exiting child block", "DoBlock");
//                 if(shouldReturn) break;
//             }
//         }
//     }
//     ExitScope();

//     if(ProgramFinished())
//         return nullptr;

//     Reference * returnRef;
//     if(result != nullptr)
//     {
//         returnRef = ReferenceFor(c_returnReferenceName, result->To);
//     }
//     else
//     {
//         returnRef = NullReference();
//     }

//     if(scopeIsLocal)
//         WipeScope(scope);
//     return returnRef;
// }

// /// executes all blocks of [program]
// void DoProgram(Program* program)
// {
//     EnterProgram(program);
//     {
//         DoBlock(program->Main, program->GlobalScope);
//     }
//     ExitProgram();
// }



















// //// TAKEN FROM REFERENCE.CPP


// /// Removes [ref] from [scope]
// void RemoveReferenceFromCurrentScope(Reference* ref)
// {
//     size_t refLoc;
//     for(refLoc = 0; refLoc<CurrentScope()->ReferencesIndex.size() && CurrentScope()->ReferencesIndex.at(refLoc) != ref; refLoc++);
//     if(refLoc != CurrentScope()->ReferencesIndex.size())
//         CurrentScope()->ReferencesIndex.erase(CurrentScope()->ReferencesIndex.begin()+refLoc);
// }

// void RemoveReferenceFromObjectIndex(Reference* ref)
// {
//     ObjectReferenceMap* map = nullptr;
//     if(FoundEntryInIndexOf(ObjectOf(ref), &map))
//     {
//         size_t refLoc;
//         for(refLoc=0; refLoc<map->References.size() && ref != map->References.at(refLoc); refLoc++);
//         map->References.erase(map->References.begin()+refLoc);
//     }
//     else
//     {
//         LogIt(LogSeverityType::Sev3_Critical, "RemoveReferenceFromObjectIndex", "cannot find reference in ObjectIndex");
//     }
// }


// /// removes all dependencies on [ref] and deletes [ref]
// void Dereference(Reference* ref)
// {
//     if(!IsTemporaryReference(ref) && !IsNullReference(ref))
//         return;

//     LogItDebug(Msg("dereferencing: %s", ref->Name), "Dereference");

//     if(ObjectOf(ref) != nullptr)
//     {
//         RemoveReferenceFromObjectIndex(ref);
//     }
//     RemoveReferenceFromCurrentScope(ref);
//     ReferenceDestructor(ref);
// }

// /// dereferences each element of [referencesList] from [scope]
// void DereferenceAll(std::vector<Reference*> referenceList)
// {
//     for(Reference* ref: referenceList)
//     {
//         Dereference(ref);
//     }
// }




// /// find a reference matching [refName] located in CurrentScope(). returns nullptr if none found
// Reference* GetReference(String refName)
// {
//     for(Scope* lookInScope = CurrentScope(); lookInScope != nullptr; lookInScope = lookInScope->InheritedScope)
//     {
//         for(Reference* ref: lookInScope->ReferencesIndex)
//         {
//             if(NameMatchesReference(refName, ref))
//                 return ref;
//         }
//     }
//     return nullptr;
// }


















// bool ObjectHasReference(const ObjectReferenceMap& map, const Reference* ref)
// {
//     for(Reference* objRef: map.References)
//     {
//         if(ref == objRef)
//             return true;
//     }
//     return false;
// }

// /// returns the ObjectReferenceMap corresonding to [obj] or nullptr if not found
// bool FoundEntryInIndexOf(const Object* obj, ObjectReferenceMap** foundMap)
// {
//     for(ObjectReferenceMap& map: PROGRAM->ObjectsIndex)
//     {
//         if(map.IndexedObject == obj)
//         {
//             *foundMap = &map;            
//             return true;
//         }
//     }
//     foundMap = nullptr;
//     return false;
// }

// void IndexObject(Object* obj, Reference* ref)
// {
//     ObjectReferenceMap* map = nullptr;
//     if(FoundEntryInIndexOf(obj, &map))
//     {
//         map->References.push_back(ref);
//     }
//     else
//     {
//         std::vector<Reference*> refs = { ref };
        
//         ObjectReferenceMap objMap = { obj, refs };
//         PROGRAM->ObjectsIndex.push_back(objMap);
//     }

// }

// Reference* CreateReferenceInternal(String name, ObjectClass objClass)
// {
//     Reference* ref = ReferenceConstructor();
//     Object* obj = ObjectConstructor();

//     IndexObject(obj, ref);

//     ref->Name = name;
//     ref->To = obj;

//     obj->Class = objClass;
//     obj->Value = nullptr;
    
//     return ref;
// }


// Reference* CreateReferenceToArrayObject(String name, ObjectClass objClass, int value){
//     Reference* ref = CreateReferenceInternal(name, objClass);

//     int* i = new int;
//     *i = value;

//     // TODO: Make arrays
//     // ref->ToObject->Value = i;
//     // ref->ToObject->Attributes.reserve(value);


//     return ref;
// }



// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, int value)
// {
//     Reference* ref = CreateReferenceInternal(name, objClass);
//     ObjectOf(ref)->Value = ObjectValueConstructor(value);

//     return ref;
// }



// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, double value)
// {
//     Reference* ref = CreateReferenceInternal(name, objClass);
//     ObjectOf(ref)->Value = ObjectValueConstructor(value);

//     return ref;
// }


// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, bool value)
// {
//     Reference* ref = CreateReferenceInternal(name, objClass);
//     ObjectOf(ref)->Value = ObjectValueConstructor(value);

//     return ref;
// }

// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, const String value)
// {
//     Reference* ref = CreateReferenceInternal(name, objClass);
//     ObjectOf(ref)->Value = ObjectValueConstructor(value);
    
//     return ref;
// }

// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, void* value){
//     if(objClass == StringClass)
//     {
//         return CreateReferenceToNewObject(name, objClass, *static_cast<String*>(value));
//     }
//     else if(objClass == DecimalClass)
//     {
//         return CreateReferenceToNewObject(name, objClass, *static_cast<double*>(value));
//     }
//     else if(objClass == BooleanClass)
//     {
//         return CreateReferenceToNewObject(name, objClass, *static_cast<bool*>(value));
//     }
//     else if(objClass == IntegerClass)
//     {
//         return CreateReferenceToNewObject(name, objClass, *static_cast<int*>(value));
//     }
//     else if(objClass == ArrayClass)
//     {
//         return CreateReferenceToArrayObject(name, objClass, *static_cast<int*>(value));
//     }
//     else if(objClass == TupleClass)
//     {
//         return CreateReferenceInternal(name, TupleClass);
//     }
//     else if(objClass == BaseClass)
//     {
//         return CreateReferenceInternal(name, BaseClass);
//     }
//     else 
//     {
//         return CreateReferenceInternal(name, objClass);
//     }
// }



// Reference* CreateReference(String name, Object* obj)
// {
//     Reference* ref = ReferenceConstructor();
//     IndexObject(obj, ref);

//     ref->Name = name;
//     ref->To = obj;

//     return ref;
// }


// Object* NullObject()
// {
//     static Object nullObject;
//     static Scope nullScope;
//     nullObject.Class = NullClass;
//     nullObject.Value = nullptr;
//     nullObject.Attributes = &nullScope;

//     return &nullObject;
// }

// Reference* CreateNullReference(String name)
// {
//     Object* nullObject = NullObject();
//     Reference* ref = ReferenceConstructor();
//     ref->Name = name;
//     ref->To = nullObject;

//     IndexObject(nullObject, ref);
    
//     return ref;
// }

// Reference* CreateNullReference()
// {
//     return CreateNullReference(c_temporaryReferenceName);
// }


// Reference* CreateReferenceToNewObject(String name, Token* valueToken)
// {
//     String value = valueToken->Content;
//     bool b;

//     switch(valueToken->Type)
//     {
//         case TokenType::Integer:
//         return CreateReferenceToNewObject(name, IntegerClass, std::stoi(value));

//         case TokenType::Boolean:
//         b = value == "true" ? true : false;
//         return CreateReferenceToNewObject(name, BooleanClass, b);

//         case TokenType::String:
//         return CreateReferenceToNewObject(name, StringClass, value);

//         case TokenType::Decimal:
//         return CreateReferenceToNewObject(name, DecimalClass, std::stod(value));

//         case TokenType::Reference:
//         default:
//         LogIt(LogSeverityType::Sev1_Notify, "CreateReferenceToNewObject", "unimplemented in this case (generic References/Simple)");
//         return CreateNullReference();
//     }
// }

// Reference* CreateReferenceToNewObject(Token* nameToken, Token* valueToken)
// {
//     return CreateReferenceToNewObject(nameToken->Content, valueToken);
// }





// // ---------------------------------------------------------------------------------------------------------------------
// // Get/Create generic object References

// /// interface for getting References from tokens. if [token] refers to a primitive object, then a reference to a primitive
// /// will be returned that points to the existing primitive if possible. Otherwise the reference is looked up in the
// /// current scope. Failure to find the reference returns a Null reference
// Reference* ReferenceFor(Token* token, String refName)
// {
//     if(TokenMatchesType(token, PrimitiveTokenTypes))
//     {
//         return ReferenceForPrimitive(token, refName);
//     }
//     else if(TokenMatchesType(token, TokenType::Reference))
//     {
//         return GetReference(token->Content);
//     }
//     LogIt(Sev3_Critical, "ReferenceFor", "unknown error");
//     return CreateNullReference();
// }

// /// gets a reference by name by looking in the current scope
// Reference* ReferenceFor(String refName)
// {
//     return GetReference(refName);
// }

// Reference* ReferenceForInImmediateScope(String refName, Scope* scope)
// {
//     for(auto ref: scope->ReferencesIndex)
//     {
//         if(NameMatchesReference(refName, ref))
//             return ref;
//     }
//     return nullptr;
// }

// /// gets a reference for a Integer primitive
// Reference* ReferenceFor(String refName, int value)
// {
//     Reference* ref = ReferenceForPrimitive(value, refName);
//     AddReferenceToCurrentScope(ref);

//     return ref;
// }

// /// gets a reference for a Boolean primitive
// Reference* ReferenceFor(String refName, bool value)
// {
//     Reference* ref = ReferenceForPrimitive(value, refName);
//     AddReferenceToCurrentScope(ref);

//     return ref;
// }

// /// gets a reference for a String primitive
// Reference* ReferenceFor(String refName, String value)
// {
//     Reference* ref = ReferenceForPrimitive(value, refName);
//     AddReferenceToCurrentScope(ref);

//     return ref;
// }

// /// gets a reference for a Decimal primitive
// Reference* ReferenceFor(String refName, double value)
// {
//     Reference* ref = ReferenceForPrimitive(value, refName);
//     AddReferenceToCurrentScope(ref);

//     return ref;
// }

// /// gets new reference named [refName] for [objClass] with [value]. This will also create
// /// a corresponding new object
// Reference* ReferenceFor(String refName, ObjectClass objClass, void* value)
// {
//     if(objClass == IntegerClass)
//         return ReferenceFor(c_returnReferenceName, *static_cast<int*>(value));
//     else if(objClass == DecimalClass)
//         return ReferenceFor(c_returnReferenceName, *static_cast<double*>(value));
//     else if(objClass == BooleanClass)
//         return ReferenceFor(c_returnReferenceName, *static_cast<bool*>(value));
//     else if(objClass == StringClass)
//         return ReferenceFor(c_returnReferenceName, *static_cast<String*>(value));

//     Reference* ref = CreateReferenceToNewObject(refName, objClass, value);
//     AddReferenceToCurrentScope(ref);

//     return ref;
// }

// /// defines a new reference named [refName] to [Object]
// Reference* ReferenceFor(String refName, Object* obj)
// {
//     Reference* ref = CreateReference(refName, obj);
//     AddReferenceToCurrentScope(ref);
//     return ref;
// }

// /// reassign an existing reference [ref] to [to]
// void ReassignReference(Reference* ref, Object* to)
// {
//     RemoveReferenceFromObjectIndex(ref);
//     IndexObject(to, ref);
    
//     ref->To = to;
// }



// // ---------------------------------------------------------------------------------------------------------------------
// // Get/Create null references

// /// create a new reference named [refName] to the NullObject
// Reference* NullReference(String refName)
// {
//     Reference* ref = CreateNullReference(refName);
//     AddReferenceToCurrentScope(ref);
//     return ref;
// }

// /// true if [ref] is a null reference
// bool IsNullReference(const Reference* ref)
// {
//     return (ObjectOf(ref) != nullptr && ObjectOf(ref) == NullObject());
// }

// /// reassign an existing reference [ref] to NullObject
// void AssignToNull(Reference* ref)
// {
//     RemoveReferenceFromObjectIndex(ref);
//     IndexObject(NullObject(), ref);
//     ref->To = NullObject();
// }


// Reference* CreateReferenceInternal(String name, ObjectClass objClass)
// {
//     Reference* ref = ReferenceConstructor();
//     Object* obj = ObjectConstructor();

//     IndexObject(obj, ref);

//     ref->Name = name;
//     ref->To = obj;

//     obj->Class = objClass;
//     obj->Value = nullptr;
    
//     return ref;
// }


// Reference* CreateReferenceToArrayObject(String name, ObjectClass objClass, int value){
//     Reference* ref = CreateReferenceInternal(name, objClass);

//     int* i = new int;
//     *i = value;

//     // TODO: Make arrays
//     // ref->ToObject->Value = i;
//     // ref->ToObject->Attributes.reserve(value);


//     return ref;
// }

// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, int value)
// {
//     Reference* ref = CreateReferenceInternal(name, objClass);
//     ObjectOf(ref)->Value = ObjectValueConstructor(value);

//     return ref;
// }

// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, double value)
// {
//     Reference* ref = CreateReferenceInternal(name, objClass);
//     ObjectOf(ref)->Value = ObjectValueConstructor(value);

//     return ref;
// }

// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, bool value)
// {
//     Reference* ref = CreateReferenceInternal(name, objClass);
//     ObjectOf(ref)->Value = ObjectValueConstructor(value);

//     return ref;
// }


// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, const String value)
// {
//     Reference* ref = CreateReferenceInternal(name, objClass);
//     ObjectOf(ref)->Value = ObjectValueConstructor(value);
    
//     return ref;
// }

// Reference* CreateReferenceToNewObject(String name, ObjectClass objClass, void* value){
//     if(objClass == StringClass)
//     {
//         return CreateReferenceToNewObject(name, objClass, *static_cast<String*>(value));
//     }
//     else if(objClass == DecimalClass)
//     {
//         return CreateReferenceToNewObject(name, objClass, *static_cast<double*>(value));
//     }
//     else if(objClass == BooleanClass)
//     {
//         return CreateReferenceToNewObject(name, objClass, *static_cast<bool*>(value));
//     }
//     else if(objClass == IntegerClass)
//     {
//         return CreateReferenceToNewObject(name, objClass, *static_cast<int*>(value));
//     }
//     else if(objClass == ArrayClass)
//     {
//         return CreateReferenceToArrayObject(name, objClass, *static_cast<int*>(value));
//     }
//     else if(objClass == TupleClass)
//     {
//         return CreateReferenceInternal(name, TupleClass);
//     }
//     else if(objClass == BaseClass)
//     {
//         return CreateReferenceInternal(name, BaseClass);
//     }
//     else 
//     {
//         return CreateReferenceInternal(name, objClass);
//     }
// }



// Reference* CreateReference(String name, Object* obj)
// {
//     Reference* ref = ReferenceConstructor();
//     IndexObject(obj, ref);

//     ref->Name = name;
//     ref->To = obj;

//     return ref;
// }

// Reference* CreateNullReference(String name)
// {
//     Object* nullObject = NullObject();
//     Reference* ref = ReferenceConstructor();
//     ref->Name = name;
//     ref->To = nullObject;

//     IndexObject(nullObject, ref);
    
//     return ref;
// }

// Reference* CreateNullReference()
// {
//     return CreateNullReference(c_temporaryReferenceName);
// }


// Reference* CreateReferenceToNewObject(String name, Token* valueToken)
// {
//     String value = valueToken->Content;
//     bool b;

//     switch(valueToken->Type)
//     {
//         case TokenType::Integer:
//         return CreateReferenceToNewObject(name, IntegerClass, std::stoi(value));

//         case TokenType::Boolean:
//         b = value == "true" ? true : false;
//         return CreateReferenceToNewObject(name, BooleanClass, b);

//         case TokenType::String:
//         return CreateReferenceToNewObject(name, StringClass, value);

//         case TokenType::Decimal:
//         return CreateReferenceToNewObject(name, DecimalClass, std::stod(value));

//         case TokenType::Reference:
//         default:
//         LogIt(LogSeverityType::Sev1_Notify, "CreateReferenceToNewObject", "unimplemented in this case (generic References/Simple)");
//         return CreateNullReference();
//     }
// }

// Reference* CreateReferenceToNewObject(Token* nameToken, Token* valueToken)
// {
//     return CreateReferenceToNewObject(nameToken->Content, valueToken);
// }


// // ---------------------------------------------------------------------------------------------------------------------
// // Scopestack

// using namespace utils;

// /// stack which contains the hierarchy of scopes building up to the current Scope of execution
// static Stack<Scope*> ScopeStack;

// /// returns the current scope
// Scope* CurrentScope()
// {
//     return ScopeStack.Peek();
// }

// /// enters a new scope
// void EnterScope(Scope* newScope)
// {
//     ScopeStack.Push(newScope);
// }

// /// exit the current scope and return to the previous scope (i.e. the scope before entering this one)
// void ExitScope(bool andDestroy)
// {
//     auto scope = ScopeStack.Pop();
//     if(andDestroy)
//         ScopeDestructor(scope);
// }

// /// add [ref] to current scope
// void AddReferenceToCurrentScope(Reference* ref)
// {
//     if(CurrentScope() == nullptr)
//     {
//         LogIt(LogSeverityType::Sev2_Important, "AddReferenceToCurrentScope", "current scope is not set");
//         return;
//     }
//     CurrentScope()->ReferencesIndex.push_back(ref);
// }

// bool ScopeStackIsEmpty()
// {
//     return ScopeStack.Size() == 0;
// }

// void WipeScope(Scope* scope)
// {
//     for(auto ref: scope->ReferencesIndex)
//     {
//         RemoveReferenceFromObjectIndex(ref);
//         ReferenceDestructor(ref);
//     }
//     ScopeDestructor(scope);
// }