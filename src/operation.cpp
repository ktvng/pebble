#include <iostream>

#include "operation.h"
#include "program.h"
#include "reference.h"
#include "execute.h"
#include "main.h"
#include "token.h"
#include "object.h"
#include "scope.h"
#include "diagnostics.h"


// ---------------------------------------------------------------------------------------------------------------------
// TODO:
// 1. Refactor code to put similar methods in the same section (i.e. DecideProbabilityX, OperationX, DecideOperandsX)
// 2. Implement unimplemented operations
// 3. Revamp the DecideOperandsX system


// ---------------------------------------------------------------------------------------------------------------------
// Operations
OperationEvaluator OperationEvaluators[] = 
{
    OperationAssign,
    OperationIsEqual,
    OperationIsNotEqual,
    OperationIsLessThan,
    OperationIsGreaterThan,
    OperationIsLessThanOrEqualTo,
    OperationIsGreaterThanOrEqualTo,

    OperationAdd,
    OperationSubtract,
    OperationMultiply,
    OperationDivide,

    OperationAnd,
    OperationOr,
    OperationNot,
    OperationEvaluate,
    OperationPrint,
    OperationAsk,

    OperationRef,
    OperationDefineMethod,
    OperationReturn,

    OperationIf,
    OperationElseIf,
    OperationElse,
    OperationWhile,
    OperationEndLabel,
    OperationTuple,

    OperationNew,
    OperationScopeResolution,
    OperationClass,


};


// ---------------------------------------------------------------------------------------------------------------------
// Creating operations

/// constructor for an operation of [type], with [operands], and value [value]. only operations
/// of type Ref::Operation should have non-nullptr value.
Operation* OperationConstructor(
    OperationType type, 
    OperationsList operands, 
    Reference* value)
{
    Operation* op = new Operation;
    op->LineNumber = -1;
    op->Operands = operands;
    op->Type = type;
    op->Value = value;

    op->ExecType = ExecutableType::Operation;

    return op;
}

/// constructor for an operation of [type], with [operands], and value [value]. only operations
/// of type Ref::Operation should have non-nullptr value. For memcheck purposes, cannot call the
/// other constructor
Operation* OperationConstructor(
    OperationType type, 
    Reference* value,
    OperationsList operands)
{
    Operation* op = new Operation;
    op->LineNumber = -1;
    op->Operands = operands;
    op->Type = type;
    op->Value = value;

    op->ExecType = ExecutableType::Operation;

    return op;
}

void OperationDestructor(Operation* op)
{
    delete op;
}

void DeleteOperationRecursive(Operation* op)
{
    for(auto operand: op->Operands)
        DeleteOperationRecursive(operand);
    
    if(op->Type == OperationType::Ref && IsReferenceStub(op->Value))
    {
        ReferenceStubDestructor(op->Value);
    }
    OperationDestructor(op);
}

Reference* ResolveStub(Reference* ref) 
{
    if(!IsReferenceStub(ref))
        return ref;

    if(ref->Name == "that" || ref->Name == "it")
    {
        if(PROGRAM->That == nullptr)
        {
            ReportRuntimeMsg(SystemMessageType::Exception, "no previous result to use with 'that'/'it' keyword");
            return NullReference(c_temporaryReferenceName);
        }
        return ReferenceFor(c_temporaryReferenceName, PROGRAM->That->To);
    }
    
    auto lookupRef = ReferenceFor(ref->Name);
    if(lookupRef == nullptr)
        return NullReference(ref->Name);
    else
        return lookupRef;
}

inline Reference* ResolveNthOf(std::vector<Reference*>& operands, size_t n)
{
    if(operands.size() >= n)
        return ResolveStub(operands.at(n-1));
    return nullptr;
}

inline Reference* ResolveFirst(std::vector<Reference*>& operands)
{
    return ResolveNthOf(operands, 1);
}

inline Reference* ResolveSecond(std::vector<Reference*>& operands)
{
    return ResolveNthOf(operands, 2);
}

bool ReferencesAreEqual(Reference* lhs, Reference* rhs)
{
    return lhs->To == rhs->To;
}

/// TODO: allow intra primitive comparisions
Reference* OperationIsEqual(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lhs = ResolveFirst(operands);
    Reference* rhs = ResolveSecond(operands);
    bool areEqual = ReferencesAreEqual(lhs, rhs);
    
    return ReferenceFor(c_temporaryReferenceName, areEqual);
}

Reference* OperationIsNotEqual(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lhs = ResolveFirst(operands);
    Reference* rhs = ResolveSecond(operands);
    bool areEqual = ReferencesAreEqual(lhs, rhs);
    
    return ReferenceFor(c_temporaryReferenceName, !areEqual);
}

Reference* OperationIsLessThan(Reference* value, std::vector<Reference*>& operands)
{
    auto lRef = ResolveFirst(operands);
    auto rRef = ResolveSecond(operands);
    bool comparisonIs = false;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {
        comparisonIs = GetDecimalValue(*ObjectOf(lRef)) < GetDecimalValue(*ObjectOf(rRef));
    }

    return ReferenceFor(c_temporaryReferenceName, comparisonIs);
}

Reference* OperationIsGreaterThan(Reference* value, std::vector<Reference*>& operands)
{
    auto lRef = ResolveFirst(operands);
    auto rRef = ResolveSecond(operands);
    bool comparisonIs = false;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {
        comparisonIs = GetDecimalValue(*ObjectOf(lRef)) > GetDecimalValue(*ObjectOf(rRef));
    }

    return ReferenceFor(c_temporaryReferenceName, comparisonIs);
}

Reference* OperationIsLessThanOrEqualTo(Reference* value, std::vector<Reference*>& operands)
{
    auto lRef = ResolveFirst(operands);
    auto rRef = ResolveSecond(operands);
    bool comparisonIs = false;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {
        comparisonIs = GetDecimalValue(*ObjectOf(lRef)) <= GetDecimalValue(*ObjectOf(rRef));
    }

    return ReferenceFor(c_temporaryReferenceName, comparisonIs);
}

Reference* OperationIsGreaterThanOrEqualTo(Reference* value, std::vector<Reference*>& operands)
{
    auto lRef = ResolveFirst(operands);
    auto rRef = ResolveSecond(operands);
    bool comparisonIs = false;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {
        comparisonIs = GetDecimalValue(*ObjectOf(lRef)) >= GetDecimalValue(*ObjectOf(rRef));
    }
    
    return ReferenceFor(c_temporaryReferenceName, comparisonIs);
}


Reference* OperationOr(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = ResolveFirst(operands);
    Reference* rRef = ResolveSecond(operands);

    bool b = GetBoolValue(*ObjectOf(lRef)) || GetBoolValue(*ObjectOf(rRef));
    return ReferenceFor(c_temporaryReferenceName, b);
}

Reference* OperationNot(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = ResolveFirst(operands);

    bool b = !GetBoolValue(*ObjectOf(ref));
    return ReferenceFor(c_temporaryReferenceName, b);
}

Reference* OperationEndLabel(Reference* value, std::vector<Reference*>& operands)
{
    LogIt(LogSeverityType::Sev1_Notify, "OperationEndLabel", "unimplemented");
    return NullReference();
}


// ---------------------------------------------------------------------------------------------------------------------
// Atomic Operations

/// handles OperationType::Ref which returns a reference
Reference* OperationRef(Reference* value, std::vector<Reference*>& operands)
{
    return value;
}

/// handles OperationType::Assign which assigns a reference [lRef] to the Object of [rRef]
/// returns a temporary reference to the assigned Object
Reference* OperationAssign(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = ResolveFirst(operands);
    Reference* rRef = ResolveSecond(operands);

    if(IsNullReference(rRef))
    {
        ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot assign %s to Nothing", lRef));
        Dereference(lRef);
        return ReferenceFor(c_temporaryReferenceName, lRef->To);
    }

    ReassignReference(lRef, rRef->To);
    return ReferenceFor(c_temporaryReferenceName, lRef->To);
}

/// handles OperationType::Print which prints the string value of [ref]
/// returns a temporary reference to the printed Object
Reference* OperationPrint(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = ResolveFirst(operands);

    if(g_outputOn)
        std::cout << GetStringValue(*ObjectOf(ref)) << "\n";
    ProgramOutput.append(GetStringValue(*ObjectOf(ref)) + "\n");

    auto returnRef = ReferenceFor(c_temporaryReferenceName, ObjectOf(ref));
    if(IsNullReference(ref))
        Dereference(ref);
    
    return returnRef;
}

/// handles OperationType::Add which adds the objects of [lRef] and [rRef]
/// only supports adding objects of numeric type and Strings (by concatenation)
/// returns a temporary reference to the addition result, which is null on failure
Reference* OperationAdd(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = ResolveFirst(operands);
    Reference* rRef = ResolveSecond(operands);
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*ObjectOf(lRef), *ObjectOf(rRef));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*ObjectOf(lRef)) + GetIntValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*ObjectOf(lRef)) + GetDecimalValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Add", "unimplemented");
            resultRef = NullReference();
        }
        
        return resultRef;
    }
    // allow for string addition by concatenation. this promotes all other objects to strings
    else if(IsString(lRef) || IsString(rRef))
    {
        String s = GetStringValue(*ObjectOf(lRef)) + GetStringValue(*ObjectOf(rRef));
        resultRef = ReferenceFor(c_temporaryReferenceName, s);

        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot add types %s and %s", 
        ObjectOf(lRef)->Class, 
        ObjectOf(rRef)->Class));

    return resultRef;
}

/// handles OperationType::And which returns the && of the boolean value for [lRef] and [rRef]
/// returns a temporary reference to the result
Reference* OperationAnd(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = ResolveFirst(operands);
    Reference* rRef = ResolveSecond(operands);

    bool b = GetBoolValue(*ObjectOf(lRef)) && GetBoolValue(*ObjectOf(rRef));
    return ReferenceFor(c_temporaryReferenceName, b);
}

/// handles OperationType::Subtract which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationSubtract(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = ResolveFirst(operands);
    Reference* rRef = ResolveSecond(operands);
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*ObjectOf(lRef), *ObjectOf(rRef));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*ObjectOf(lRef)) - GetIntValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*ObjectOf(lRef)) - GetDecimalValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Subtract", "unimplemented");
            resultRef = NullReference();
        }
        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot subtract %s from %s", ObjectOf(rRef)->Class, ObjectOf(lRef)->Class));
    return resultRef;
}

/// handles OperationType::If 
/// returns a temporary reference to an object representing the evaluated if-expression
Reference* OperationIf(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = ResolveFirst(operands);
    return ReferenceFor(c_temporaryReferenceName, ObjectOf(ref));
}

Reference* OperationWhile(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = ResolveFirst(operands);
    return ReferenceFor(c_temporaryReferenceName, ObjectOf(ref));
}


/// handles OperationType::Multiply which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationMultiply(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = ResolveFirst(operands);
    Reference* rRef = ResolveSecond(operands);
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*ObjectOf(lRef), *ObjectOf(rRef));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*ObjectOf(lRef)) * GetIntValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*ObjectOf(lRef)) * GetDecimalValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Multiply", "unimplemented");
            resultRef = NullReference();
        }
        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot multiply %s and %s", ObjectOf(lRef)->Class, ObjectOf(rRef)->Class));
    return resultRef;
}

// ---------------------------------------------------------------------------------------------------------------------
// DivideOperation

/// handles OperationType::Divide which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationDivide(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = ResolveFirst(operands);
    Reference* rRef = ResolveSecond(operands);
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*ObjectOf(lRef), *ObjectOf(rRef));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*ObjectOf(lRef)) / GetIntValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*ObjectOf(lRef)) / GetDecimalValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Divide", "unimplemented");
            resultRef = NullReference();
        }
        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot divide %s by %s", ObjectOf(lRef)->Class, ObjectOf(rRef)->Class));
    return resultRef;
}

/// handles OperationType::Return to exit a method
/// returns a persistant reference to the return value
Reference* OperationReturn(Reference* value, std::vector<Reference*>& operands)
{
    Reference* returnRef = ResolveFirst(operands);
    return ReferenceFor(c_returnReferenceName, returnRef->To);
}





// ---------------------------------------------------------------------------------------------------------------------
// OperationType::DefineMethod

/// handle the operation which adds a Method [ref] to the scope
Reference* OperationDefineMethod(Reference* value, std::vector<Reference*>& operands)
{
    Reference* methodRef = ResolveFirst(operands);
    if(IsNullReference(methodRef))
    {
        auto refToNewObj = ReferenceFor(c_temporaryReferenceName, BaseClass, nullptr);
        
        /// needed because method scope is destroyed after each call
        if(CurrentScope()->IsDurable)
            refToNewObj->To->DefinitionScope = CurrentScope();
        
        refToNewObj->To->Action = MethodConstructor();
        ReassignReference(methodRef, refToNewObj->To);
        Dereference(refToNewObj);
    }
    else
    {
        if(methodRef->To->Action == nullptr)
            methodRef->To->Action = MethodConstructor();
    }

    ParameterList params;
    if(operands.size() > 1)
    {
        auto argsRef = ResolveSecond(operands);
        if(IsNullReference(argsRef) || argsRef->To->Class != TupleClass)
        {
            params.push_back(argsRef->Name);
        }
        else
        {
            for(auto paramRef: argsRef->To->Attributes->ReferencesIndex)
            {
                params.push_back(paramRef->Name);
            }
        }
    }

    methodRef->To->Action->ParameterNames = params;

    return methodRef;
}




// ---------------------------------------------------------------------------------------------------------------------
// Tuple operation
Reference* OperationTuple(Reference* value, std::vector<Reference*>& operands)
{
    LogItDebug("called", "OperationTuple");
    Reference* tupleRef = ReferenceFor(c_temporaryReferenceName, TupleClass, nullptr);

    for(auto ref: operands)
    {
        EnterScope(ObjectOf(tupleRef)->Attributes);
        {
            auto copyRef = NullReference(ref->Name);
            ReassignReference(copyRef, ref->To);
        }
        ExitScope();

    }

    return tupleRef;
}


// ---------------------------------------------------------------------------------------------------------------------
// Evaluate operation

std::vector<Reference*> ResolveParamters(Reference* ref)
{
    if(IsNullReference(ref))
        return {};

    auto obj = ObjectOf(ref);
    if(obj != nullptr && obj->Class == TupleClass)
    {
        std::vector<Reference*> params;
        for(auto ref: obj->Attributes->ReferencesIndex)
        {
            params.push_back(ref);
        }
        return params;
    }
    else
    {
        return { ref };
    }
    
}

/// methods are called by a caller, and if no caller is specified, CurrentScope takes on the role of 
/// caller. the order of operands should be [caller] [methodname] [arguments]
/// handles OperationType::Evaluate which takes in a Method reference [ref] and [parameters], the first
/// of which is a reference to the method, and evaluates the method on these parameters
/// returns a persistant reference to the returned result if a return statement was called
/// or a temporary reference if no return statement was called
Reference* OperationEvaluate(Reference* value, std::vector<Reference*>& operands)
{
    // operands guarenteed to be at size 3
    String callerName = operands[0]->Name;
    Reference* caller = nullptr;

    Scope* methodResolutionScope = nullptr;
    Scope* methodCallerScope = nullptr;

    if(callerName == c_nullStubName)
    {
        methodResolutionScope = CurrentScope();
        methodCallerScope = nullptr;
    }
    else
    {
        caller = ResolveFirst(operands);
        if(IsNullReference(caller))
        {
            ReportRuntimeMsg(SystemMessageType::Exception, "cannot resolve method caller");
            return caller;
        }

        methodResolutionScope = caller->To->Attributes;
        methodCallerScope = caller->To->Attributes;
    }

    Reference* method;
    EnterScope(methodResolutionScope);
    {
        /// TODO: may want to stay exlusively in caller scope
        method = ResolveSecond(operands);        
    }
    ExitScope();
    
    /// TODO: was option to look in local scope even if there is a caller

    if(IsNullReference(method))
    {
        ReportRuntimeMsg(SystemMessageType::Exception, Msg("cannot resolve %s into a method", method->Name));
        return method;
    }
    if(method->To->Action == nullptr)
    {
        ReportRuntimeMsg(SystemMessageType::Exception, Msg("%s is not callable", method->Name));
        return NullReference();
    }

    Reference* params = ResolveNthOf(operands, 3);
    
    // resolve param refs from params
    std::vector<Reference*> givenParamsList = ResolveParamters(params);
    auto givenNum = givenParamsList.size();
    auto requiredNum = ObjectOf(method)->Action->ParameterNames.size();
    if(givenNum != requiredNum)
    {
        ReportRuntimeMsg(SystemMessageType::Exception, Msg("wrong number of parameters: given %i, but expected %i", givenNum, requiredNum));
        return NullReference();
    }

    auto methodParamNames = ObjectOf(method)->Action->ParameterNames;
    
    /// if no caller, then method scope should be the one it was defined in
    if(caller == nullptr)
    {
        // prefer DefinitionScope if it is durable and has been set. otherwise default to current scope
        methodCallerScope = method->To->DefinitionScope;
        if(methodCallerScope == nullptr)
        {
            methodCallerScope = CurrentScope();
        }
    }

    // add parameters to method scope
    auto methodSelfScope = method->To->Attributes;
    methodSelfScope->InheritedScope = methodCallerScope;

    auto methodBodyScope = ScopeConstructor(methodSelfScope);
    EnterScope(methodBodyScope);
    {
        for(size_t i=0; i<methodParamNames.size(); i++)
        {
            ReferenceFor(methodParamNames[i], givenParamsList[i]->To);
        }
        if(caller == nullptr)
            NullReference("caller");
        else
            ReferenceFor("caller", caller->To);

        ReferenceFor("self", method->To);
    }
    ExitScope();

    Reference* result;
    auto methodBlock = ObjectOf(method)->Action->CodeBlock;
    result = DoBlock(methodBlock, methodBodyScope);

    WipeScope(methodBodyScope);

    LogItDebug("finished evaluate", "OperationEvaluate");
    return result;
}



// ---------------------------------------------------------------------------------------------------------------------
// New Operation

/// creates a new object and copies all attributes
Reference* OperationNew(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = ResolveFirst(operands);

    if(IsNullReference(ref))
    {
        ReportRuntimeMsg(SystemMessageType::Exception, Msg("%s is Nothing, it must be defined before use", ref->Name));
        return NullReference();
    }

    Reference* returnRef = ReferenceFor(c_temporaryReferenceName, ObjectOf(ref)->Class, ObjectOf(ref)->Value);
    
    // needed because method scope is destroyed after each call
    if(CurrentScope()->IsDurable)
        returnRef->To->DefinitionScope = CurrentScope();
    for(auto attributeRef: ObjectOf(ref)->Attributes->ReferencesIndex)
    {
        EnterScope(ObjectOf(returnRef)->Attributes);
        {
            ReferenceFor(attributeRef->Name, attributeRef->To);
        }
        ExitScope();
    }
    ObjectOf(returnRef)->Attributes->InheritedScope = ObjectOf(ref)->Attributes->InheritedScope;
    returnRef->To->Action = ref->To->Action;

    return returnRef;
}

// ---------------------------------------------------------------------------------------------------------------------
// Scope Resolution Operation

inline bool HasNoCaller(std::vector<Reference*>& operands)
{
    return operands.size() < 2;
}

/// resolves one link in a scope chain
Reference* OperationScopeResolution(Reference* value, std::vector<Reference*>& operands)
{
    if(HasNoCaller(operands))
    {
        Reference* ref = ResolveFirst(operands);
        if(IsTemporaryReference(ref))
            return ReferenceFor(c_temporaryReferenceName, ref->To);
        else
            return ref;
    }
    else
    {
        Reference* caller = ResolveFirst(operands);

        if(IsNullReference(caller))
        {
            ReportRuntimeMsg(SystemMessageType::Exception, Msg("%s is Nothing and has no attributes", caller->Name));
            return NullReference();
        }

        String attributeName = operands.at(1)->Name;
        Scope* callerScope = caller->To->Attributes;
        Reference* attribute = ReferenceForInImmediateScope(attributeName, callerScope);

        if(attribute == nullptr)
        {
            EnterScope(callerScope);
            {
                attribute = NullReference(attributeName);
            }
            ExitScope();
        }

        return attribute;
    }
}


// ---------------------------------------------------------------------------------------------------------------------
// Class Operation

Reference* OperationClass(Reference* value, std::vector<Reference*>& operands)
{
    /// TODO: clean up and handle inheritance
    if(operands.size() == 0)
    {
        ReportRuntimeMsg(SystemMessageType::Exception, "no class name specified");
        return NullReference();
    }

    String className = operands.at(0)->Name;
    Reference* klass = ReferenceFor(className, className, nullptr);

    EnterScope(klass->To->Attributes);
    {
        ReferenceFor("self", klass->To);
    }
    ExitScope();

    klass->To->Attributes->InheritedScope = PROGRAM->GlobalScope;

    return klass;
}


// ---------------------------------------------------------------------------------------------------------------------
// Unimplemented

Reference* OperationAsk(Reference* value, std::vector<Reference*>& operands)
{
    LogIt(LogSeverityType::Sev3_Critical, "OperationAsk", "unimplemented");
    return NullReference();
}
Reference* OperationElseIf(Reference* value, std::vector<Reference*>& operands)
{
    LogIt(LogSeverityType::Sev3_Critical, "OperationAsk", "unimplemented");
    return NullReference();
}
Reference* OperationElse(Reference* value, std::vector<Reference*>& operands)
{
    LogIt(LogSeverityType::Sev3_Critical, "OperationAsk", "unimplemented");
    return NullReference();
}