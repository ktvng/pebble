#include "bytecode.h"

#include "vm.h"
#include "errormsg.h"
#include "flattener.h"

#include "main.h"
#include "program.h"
#include "scope.h"
#include "diagnostics.h"
#include "call.h"
#include "scope.h"
#include "value.h"


// ---------------------------------------------------------------------------------------------------------------------
// Internal constructor wrappers

/// wrapper for the ScopeConstructor used to create a new scope with
/// [inheritedscope]
inline Scope* InternalScopeConstructor(Scope* inheritedScope)
{
    auto scope = ScopeConstructor(inheritedScope);
    AddRuntimeScope(scope);

    return scope;
}

/// wrapper for CopyScope used to copy [scopeToCopy] into a new scope
inline Scope* InternalCopyScope(Scope* scopeToCopy)
{
    Scope* scope;
    if(scopeToCopy == &NothingScope)
    {
        scope = &NothingScope;
    }
    else
    {
        scope = CopyScope(scopeToCopy);
        AddRuntimeScope(scope);
    }
    

    return scope;
}

/// wrapper for CallConstructor to create a new call with [name]
inline Call* InternalCallConstructor(const String* name=nullptr)
{
    Call* call = CallConstructor(name);
    BindScope(call, &NothingScope);
    BindType(call, &NothingType);
    AddRuntimeCall(call);

    return call;
}

inline Call* InternalSharedPrimitiveCallConstructor()
{
    auto call = InternalCallConstructor();
    BindScope(call, &SomethingScope);
    return call;
}

/// wrapper to construct a call for a primitive [value] enforcing the 
/// prior that a given primitive value should only have one constructed call
Call* InternalPrimitiveCallConstructor(int value)
{
    Call* foundCall = nullptr;
    if(ListContainsPrimitiveCall(RuntimeCalls, value, &foundCall))
    {
        return foundCall;
    }
    else if(ListContainsPrimitiveCall(ConstPrimitives, value, &foundCall))
    {
        return foundCall;
    }
    else
    {
        Call* call = InternalSharedPrimitiveCallConstructor();
        BindType(call, &IntegerType);
        AssignValue(call->BoundValue, value);
        return call;
    }
}

/// wrapper to construct a call for a primitive [value] enforcing the 
/// prior that a given primitive value should only have one constructed call
Call* InternalPrimitiveCallConstructor(double value)
{
    Call* foundCall = nullptr;
    if(ListContainsPrimitiveCall(RuntimeCalls, value, &foundCall))
    {
        return foundCall;
    }
    else if(ListContainsPrimitiveCall(ConstPrimitives, value, &foundCall))
    {
        return foundCall;
    }
    else
    {
        Call* call = InternalSharedPrimitiveCallConstructor();
        BindType(call, &DecimalType);
        AssignValue(call->BoundValue, value);

        return call;
    }
}

/// wrapper to construct a call for a primitive [value] enforcing the 
/// prior that a given primitive value should only have one constructed call
Call* InternalPrimitiveCallConstructor(bool value)
{
    Call* foundCall = nullptr;
    if(ListContainsPrimitiveCall(RuntimeCalls, value, &foundCall))
    {
        return foundCall;
    }
    else if(ListContainsPrimitiveCall(ConstPrimitives, value, &foundCall))
    {
        return foundCall;
    }
    else
    {
        Call* call = InternalSharedPrimitiveCallConstructor();
        BindType(call, &BooleanType);
        AssignValue(call->BoundValue, value);

        return call;
    }
}

/// wrapper to construct a call for a primitive [value] enforcing the 
/// prior that a given primitive value should only have one constructed call
Call* InternalPrimitiveCallConstructor(String& value)
{
    Call* foundCall = nullptr;
    if(ListContainsPrimitiveCall(RuntimeCalls, value, &foundCall))
    {
        return foundCall;
    }
    else if(ListContainsPrimitiveCall(ConstPrimitives, value, &foundCall))
    {
        return foundCall;
    }
    else
    {
        Call* call = InternalSharedPrimitiveCallConstructor();
        BindType(call, &StringType);
        AssignValue(call->BoundValue, value);

        return call;
    }
}

Call* InternalCopyCall(const Call* call)
{
    if(call == &NothingCall)
    {
        return &NothingCall;
    }
    
    auto callCopy = InternalCallConstructor();
    
    auto scp = InternalCopyScope(call->BoundScope);
    BindScope(callCopy, scp);
    BindType(callCopy, call->BoundType);
    BindSection(callCopy, call->BoundSection);
    callCopy->NumberOfParameters = call->NumberOfParameters;

    return callCopy;
}

// ---------------------------------------------------------------------------------------------------------------------
// Bytecode id helper methods

// returns the index the NOP instruction in BCI_Instructions array
extArg_t IndexOfNop()
{
    for(size_t i=0; i<BCI_NumberOfInstructions; i++)
    {
        if(BCI_Instructions[i] == BCI_NOP)
        {
            return i;
        }
    }
    return -1;
}

// returns the index of [bci] in BCI_Instructions array
int IndexOfInstruction(BCI_Method bci)
{
    for(size_t i=0; i<BCI_NumberOfInstructions; i++)
    {
        if(BCI_Instructions[i] == bci)
        {
            return i;
        }
    }
    LogIt(LogSeverityType::Sev3_Critical, "IndexOfInstruction", "instruction not found, returned NOP");
    return IndexOfNop();
}


// ---------------------------------------------------------------------------------------------------------------------
// Instructions Array

BCI_Method BCI_Instructions[] = {
    BCI_LoadCallName,
    BCI_LoadPrimitive,
    BCI_Assign,
    BCI_Add,
    BCI_Subtract,

    BCI_Multiply,
    BCI_Divide,
    BCI_SysCall,
    BCI_And,
    BCI_Or,
    
    BCI_Not,
    BCI_NotEquals,
    BCI_Equals,
    BCI_Cmp,
    BCI_LoadCmp,

    BCI_JumpFalse,
    BCI_Jump,
    BCI_Copy,
    BCI_BindType,
    BCI_ResolveDirect,

    BCI_ResolveScoped,
    BCI_BindScope,
    BCI_BindSection,
    BCI_EvalHere,
    BCI_Eval,

    BCI_Return,
    BCI_Array,
    BCI_EnterLocal,
    BCI_LeaveLocal,
    BCI_Extend,

    BCI_NOP,
    BCI_Dup,
    BCI_EndLine,
    BCI_DropTOS,
    BCI_Is,
};



// ---------------------------------------------------------------------------------------------------------------------
// Memory stack helpers

/// pop a pointer of <T> from the top of the MemoryStack
template <typename T>
inline T* PopTOS()
{
    T* tos = static_cast<T*>(MemoryStack.back());
    MemoryStack.pop_back();
    return tos;
}

/// push a pointer of <T> to [entity] onto the MemoryStack
template <typename T>
inline void PushTOS(T* entity)
{
    MemoryStack.push_back(static_cast<void*>(entity));
}

/// peek a pointer of <T> to the top of the MemoryStack
template <typename T>
inline T* PeekTOS()
{
    return static_cast<T*>(MemoryStack.back());
}

/// return the size of the MemoryStack
inline size_t MemoryStackSize()
{
    return MemoryStack.size();
}


// ---------------------------------------------------------------------------------------------------------------------
// Scope helpers

/// finds and returns a Reference with [callName] in [scope] without checking 
/// the inherited scope
inline Call* FindInScopeOnlyImmediate(Scope* scope, const String* callName)
{
    for(auto call: scope->CallsIndex)
    {
        if(call->Name == callName)
            return call;
    }
    return nullptr;
}

/// finds and returns a Reference with [callName] in [scope] and checks
/// the entire chain of inherited scope
inline Call* FindInScopeChain(Scope* scope, const String* callName)
{
    for(auto s = scope; s != nullptr; s = s->InheritedScope)
    {
        for(auto call: s->CallsIndex)
        {
            if(call->Name == callName)
                return call;
        }
    }
    return nullptr;
}

/// returns the Scope bound to [call]
inline Scope* ScopeOf(Call* call)
{
    return call->BoundScope;
}



// ---------------------------------------------------------------------------------------------------------------------
// Instruction jump helpers

/// sets the jump location to [ins] and notifies the vm to jump
inline void InternalJumpTo(extArg_t ins)
{
    InstructionReg = ins;
    JumpStatusReg = 1;
}

/// add a new CallFrame to the CallStack with [caller] and [self] the new caller and self
/// objects. changes registers appropriately but does not change the instruction pointer
inline void EnterNewCallFrame(extArg_t callerRefId, Call* caller, Call* self)
{
    std::vector<LabeledScope> localScopeStack;
    CallStack.push_back({ InstructionReg+1, MemoryStackSize(), callerRefId, localScopeStack, LastResultReg });
    PushTOS<Call>(caller);
    PushTOS<Call>(self);

    CallerReg = caller;
    SelfReg = self;
    LocalScopeReg = self->BoundScope;
    LastResultReg = nullptr;
    LocalScopeIsDetachedReg = false;
}

/// changes the LocalScopeReg whenever a LocalScope change occurs. the base scope
/// is either the program scope or the self object scope
void AdjustLocalScopeReg()
{
    if(LocalScopeStack().size() == 0)
    {
        if(SelfReg->BoundScope != &NothingScope)
        {
            LocalScopeReg = SelfReg->BoundScope;
        }
        else
        {
            LocalScopeReg = ProgramReg;
        }

        LocalScopeIsDetachedReg = false;
    }
    else
    {
        LocalScopeReg = LocalScopeStack().back().Value;
        LocalScopeIsDetachedReg = LocalScopeStack().back().IsDetached;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Type system helpers

inline bool CanBe(const BindingType type, const Call* call)
{
    return call->BoundType == type 
        || call->BoundType == &NothingType;
}

inline bool Loosely(const BindingType type, const Call* call)
{
    return call->BoundType == type;
}

inline bool Strictly(const BindingType type, const Call* call)
{
    return call->BoundType == type 
        && call->BoundScope != &NothingScope;
}

inline bool IsNotNothing(const Call* call)
{
    return call->BoundScope != &NothingScope;
}

/// true if [call] corresponds to the general definition of Nothing
bool IsNothing(const Call* call)
{
    return call->BoundScope == &NothingScope;
}

/// true if [call] corresponds to the strict definition of Nothing
bool IsPureNothing(const Call* call)
{
    return call->BoundType == &NothingType;
}

typedef bool (*TypeClassifier)(const BindingType, const Call*);
typedef bool (*NothingTypeClassifier)(const Call*);

inline bool Either(TypeClassifier classifiedAs, const BindingType type, const Call* call1, const Call* call2)
{
    return classifiedAs(type, call1) || classifiedAs(type, call2);
}

inline bool Either(NothingTypeClassifier isNothing, const Call* call1, const Call* call2)
{
    return isNothing(call1) || isNothing(call2);
}

inline bool Both(TypeClassifier classifiedAs, const BindingType type, const Call* call1, const Call* call2)
{
    return classifiedAs(type, call1) && classifiedAs(type, call2);
}

inline Call* TheNothingOf(Call* call1, Call* call2)
{
    if(IsNothing(call1) && !IsPureNothing(call1))
    {
        return call1;   
    }
    else if(IsNothing(call2) && !IsPureNothing(call2))
    {
        return call2;
    }

    return &NothingCall;
}




// ---------------------------------------------------------------------------------------------------------------------
// Comparison instruction helpers

/// returns the [n]th bit of [data]
inline bool NthBit(uint8_t data, int n)
{
    return (data & (BitFlag << n)) >> n;
}

/// fills in <= >= using the comparisions already made
inline void FillInRestOfComparisons()
{
    uint8_t geq = NthBit(CmpReg, 2) | NthBit(CmpReg, 4);
    uint8_t leq = NthBit(CmpReg, 2) | NthBit(CmpReg, 3);

    CmpReg = CmpReg | (geq << 6) | (leq << 5);
}

/// sets CmpReg to the result of comparing [lhs] and [rhs] as integers
inline void CompareIntegers(const Call* lhs, const Call* rhs)
{
    int lVal = IntegerValueOf(lhs);
    int rVal = IntegerValueOf(rhs);

    if(lVal < rVal)
    {
        CmpReg = CmpReg | (BitFlag << 3);
    }
    else if(lVal > rVal)
    {
        CmpReg = CmpReg | (BitFlag << 4);
    }
    else
    {
        CmpReg = CmpReg | (BitFlag << 2);
    }

    FillInRestOfComparisons();
}

/// sets CmpReg to the result of comparing [lhs] and [rhs] as decimals
inline void CompareDecimals(const Call* lhs, const Call* rhs)
{
    double lVal = DecimalValueOf(lhs);
    double rVal = DecimalValueOf(rhs);

    if(lVal < rVal)
    {
        CmpReg = CmpReg | (BitFlag << 3);
    }
    else if(lVal > rVal)
    {
        CmpReg = CmpReg | (BitFlag << 4);
    }
    else
    {
        CmpReg = CmpReg | (BitFlag << 2);
    }

    FillInRestOfComparisons();
}




// ---------------------------------------------------------------------------------------------------------------------
// Binary operation helpers

typedef Call* (*InternalFunction)(size_t i, const Call* lhs, const Call* rhs);

Call* ApplyFunction(
    InternalFunction func, 
    Call* lhs, 
    Call* rhs, 
    BindingType* types, 
    size_t nTypes)
{
    Call* call = &NothingCall;
    for(size_t i=0; i<nTypes; i++)
    {
        if(Both(CanBe, types[i], lhs, rhs))
        {
            if(Both(Strictly, types[i], lhs, rhs))
            {
                call = func(i, lhs, rhs);
            }
            else
            {
                call = TheNothingOf(lhs, rhs);
            }

            return call;
        }
    }

    return nullptr;
}



// ---------------------------------------------------------------------------------------------------------------------
// Comparison instruction helpers


BindingType AddTypes[] = {
    &IntegerType, 
    &DecimalType, 
    &StringType
};

size_t nAddTypes = 3;

Call* AddFunction(size_t i, const Call* lhs, const Call* rhs)
{
    Call* call = &NothingCall;
    switch(i)
    {
        case 0:
        {
            int ans = IntegerValueOf(lhs) + IntegerValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans); 
            break;
        }

        case 1:
        {
            double ans = DecimalValueOf(lhs) + DecimalValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans);
            break;
        }

        case 2:
        {
            String ans = StringValueOf(lhs) + StringValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans);
            break;
        }

        default:
        break;
    }
    
    return call;
}

BindingType GenericMathTypes[] = 
{
    &IntegerType,
    &DecimalType
};

size_t nGenericMathTypes = 2;

Call* SubtractFunction(size_t i, const Call* lhs, const Call* rhs)
{
    Call* call = &NothingCall;
    switch(i)
    {
        case 0:
        {
            int ans = IntegerValueOf(lhs) - IntegerValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans); 
            break;
        }

        case 1:
        {
            double ans = DecimalValueOf(lhs) - DecimalValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans);
            break;
        }

        default:
        break;
    }
    
    return call;
}

Call* MultiplyFunction(size_t i, const Call* lhs, const Call* rhs)
{
    Call* call = &NothingCall;
    switch(i)
    {
        case 0:
        {
            int ans = IntegerValueOf(lhs) * IntegerValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans); 
            break;
        }

        case 1:
        {
            double ans = DecimalValueOf(lhs) * DecimalValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans);
            break;
        }

        default:
        break;
    }
    
    return call;
}

Call* DivideFunction(size_t i, const Call* lhs, const Call* rhs)
{
    Call* call = &NothingCall;
    switch(i)
    {
        case 0:
        {
            int ans = IntegerValueOf(lhs) / IntegerValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans); 
            break;
        }

        case 1:
        {
            double ans = DecimalValueOf(lhs) / DecimalValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans);
            break;
        }

        default:
        break;
    }
    
    return call;
}

BindingType GenericBooleanTypes[] = 
{
    &BooleanType,
};

size_t nGenericBooleanTypes = 1;

Call* AndFunction(size_t i, const Call* lhs, const Call* rhs)
{
    Call* call = &NothingCall;
    switch(i)
    {
        case 0:
        {
            bool ans = BooleanValueOf(lhs) && BooleanValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans); 
            break;
        }

        default:
        break;
    }
    
    return call;
}

Call* OrFunction(size_t i, const Call* lhs, const Call* rhs)
{
    Call* call = &NothingCall;
    switch(i)
    {
        case 0:
        {
            bool ans = BooleanValueOf(lhs) && BooleanValueOf(rhs);
            call = InternalPrimitiveCallConstructor(ans); 
            break;
        }

        default:
        break;
    }
    
    return call;
}

Call* CmpFunction(size_t i, const Call* lhs, const Call* rhs)
{
    Call* call = &NothingCall;
    switch(i)
    {
        case 0:
        {
            CompareIntegers(lhs, rhs);
            break;
        }

        case 1:
        {
            CompareDecimals(lhs, rhs);
            break;
        }

        default:
        break;
    }
    
    return call;
}





// ---------------------------------------------------------------------------------------------------------------------
// Assignment helers

/// true if [call] cannot be rebound to anything
inline bool IsUnassignable(const Call* call)
{
    return call == &NothingCall
        || call == &ObjectCall
        || call == &IntegerCall
        || call == &DecimalCall
        || call == &StringCall
        || call == &BooleanCall
        || call == &ArrayCall;
}

inline void InternalAssign(Call* lhs, const Call* rhs)
{
    if(IsUnassignable(lhs))
    {
        ReportFatalError(SystemMessageType::Exception, 7, 
            Msg("cannot reassign the global call %s", *lhs->Name));
    }

    if(lhs->BoundType == &NothingType)
    {
        if(lhs->BoundScope == &NothingScope)
        {
            BindType(lhs, rhs->BoundType);
            BindSection(lhs, rhs->BoundSection);
            BindScope(lhs, rhs->BoundScope);
            BindValue(lhs, rhs->BoundValue);
            lhs->NumberOfParameters = rhs->NumberOfParameters;
        }
    }
    else
    {
        if(lhs->BoundType == rhs->BoundType)
        {
            BindType(lhs, rhs->BoundType);
            BindSection(lhs, rhs->BoundSection);
            BindScope(lhs, rhs->BoundScope);
            BindValue(lhs, rhs->BoundValue);
            lhs->NumberOfParameters = rhs->NumberOfParameters;
        }
        else if(IsNothing(lhs) && IsPureNothing(rhs))
        {
            return;
        }
        else
        {
            ReportFatalError(SystemMessageType::Exception, 0,
                Msg("cannot assign %s to %s", *lhs->BoundType, *rhs->BoundType));
            return;
        }
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Equality operators

/// true if [call1] and [call2] have the same value.
inline bool CallsHaveEqualValue(const Call* call1, const Call* call2)
{
    if(Either(IsNothing, call1, call2))
    {
        return true;
    }
    else if(Both(Strictly, &IntegerType, call1, call2))
    {
        return call1->BoundValue.i == call2->BoundValue.i;
    }
    else if(Both(Strictly, &DecimalType, call1, call2))
    {
        return call1->BoundValue.d == call2->BoundValue.d;
    }
    else if(Both(Strictly, &StringType, call1, call2))
    {
        return call1->BoundValue.s == call2->BoundValue.s;
    }
    else if(Both(Strictly, &BooleanType, call1, call2))
    {
        return call1->BoundValue.b == call2->BoundValue.b;
    }
    else
    {
        return false;
    }
}

/// true if Calls are equal (i.e. defines the definition of equality in Pebble)
inline bool CallsAreEqual(const Call* call1, const Call* call2)
{
    return call1->BoundType == call2->BoundType
        && call1->BoundScope == call2->BoundScope 
        && call1->BoundSection == call2->BoundSection 
        && CallsHaveEqualValue(call1, call2);
}



// ---------------------------------------------------------------------------------------------------------------------
// Scope resolution helpers

/// resolves [callName] which is a keyword to the appropriate call which is 
/// pushed to TOS
inline void ResolveKeywordCall(const String* callName)
{
    Call* call = &NothingCall;
    if(*callName == "caller")
    {
        call = CallerReg;
    }
    else if(*callName == "self")
    {
        call = SelfReg;
    }
    else
    {
        if(LastResultReg == nullptr)
        {
            call = &NothingCall;
        }
        else
        {
            call = LastResultReg;
        }
    }

    PushTOS<Call>(call);
}


// ---------------------------------------------------------------------------------------------------------------------
// Array helpers

/// built in method to initialize a new array and push it onto TOS
inline void HandleArrayInitialization(Call* caller, Call* size)
{
    int arraySize = IntegerValueOf(size);
    IfNeededAddArrayIndexCalls(arraySize);
    for(int i=0; i<arraySize; i++)
    {
        auto callatIndex = InternalCallConstructor(CallNamePointerFor(i));
        AddCallToScope(callatIndex, ScopeOf(caller));
    }

    auto sizeCall = InternalCallConstructor(&SizeCallName);
    BindType(sizeCall, &IntegerType);
    AssignValue(sizeCall->BoundValue, arraySize);
    AddCallToScope(sizeCall, ScopeOf(caller));

    PushTOS<Call>(caller);
}

/// build in method to initialize a new object and push it onto TOS
inline void HandleObjectInitialization(Call* call)
{
    auto newObjectCall = InternalCallConstructor();
    BindType(newObjectCall, &ObjectType);
    BindScope(newObjectCall, InternalScopeConstructor(nullptr));
    
    PushTOS<Call>(newObjectCall);
}

/// true if a type corresponds to a primitive, predefined method
inline bool IsPrimitiveMethodType(const BindingType type)
{
    return type == &ArrayType 
        || type == &ObjectType 
        || type == &AbstractArrayType 
        || type == &AbstractObjectType; 
}

void HandlePrimitiveTypeInstantiation(Call* simpleCall, Call** paramsList, bool inPlace)
{
    Call* callToInitialize = nullptr;
    if(inPlace)
    {
        callToInitialize = SelfReg;
    }
    else
    {
        callToInitialize = simpleCall;
    }

    if(Loosely(&ArrayType, simpleCall) || Loosely(&AbstractArrayType, simpleCall))
    {
        HandleArrayInitialization(callToInitialize, paramsList[0]);
    }
    else if(Loosely(&ObjectType, simpleCall) || Loosely(&AbstractObjectType, simpleCall))
    {
        HandleObjectInitialization(callToInitialize);
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Eval instruction helpers

/// add a list of Calls [paramsList] to the scope of [methodCall] in reverse order
/// used when evaluating a method
inline void AddParamsToMethodScope(Call* methodCall, Call** paramsList, extArg_t nParams)
{
    for(size_t i =0; i<methodCall->NumberOfParameters; i++)
    {
        auto methodParam = methodCall->BoundScope->CallsIndex[i];
        auto paramInput = paramsList[nParams-1-i];
        InternalAssign(methodParam, paramInput);
    }
}

/// pops [numParams] objects from the MemoryStack and returns a list of these elements
/// (which is the reverse order due to the push/pop algorithm)
inline Call** GetParameters(extArg_t numParams)
{
    Call** params = new Call*[numParams];
    for(extArg_t i=0; i<numParams; i++)
    {
        params[i] = PopTOS<Call>();
    }
    
    return params;
}

/// defines the logic to evaluate the section bound to a given call
inline void InternalEval(Call* methodCall, Call* caller, Call** paramsList, extArg_t nParams, bool inPlace)
{
    if(IsPureNothing(methodCall))
    {
        PushTOS<Call>(&NothingCall);
        return;
    }

    if(nParams != methodCall->NumberOfParameters)
    {
        ReportFatalError(SystemMessageType::Exception, 2, 
            Msg("expected %i arguments but got %i", methodCall->NumberOfParameters, nParams));
        return;
    }

    if(IsPrimitiveMethodType(methodCall->BoundType))
    {
        HandlePrimitiveTypeInstantiation(methodCall, paramsList, inPlace);
        return;
    }

    if(methodCall->BoundSection == 0)
    {
        auto methodName = (methodCall->Name == nullptr ? "anonymous variable" : *methodCall->Name);
        ReportFatalError(SystemMessageType::Exception, 3, Msg("%s cannot be called", methodName));
        return;
    }

    extArg_t jumpIns = methodCall->BoundSection;

    /// TODO: figure out caller id
    /// inPlace evaluation will not change caller or self
    if(inPlace)
    {
        if(nParams != 0)
        {
            AddParamsToMethodScope(SelfReg, paramsList, nParams);
        }

        EnterNewCallFrame(0, CallerReg, SelfReg);
    }
    else
    {
        if(nParams != 0)
        {
            AddParamsToMethodScope(methodCall, paramsList, nParams);
        }

        EnterNewCallFrame(0, caller, methodCall);
    }

    InternalJumpTo(jumpIns);
}


// ---------------------------------------------------------------------------------------------------------------------
// Display (print/say) helpers

/// returns a string representation of [call]'s BoundType
String CallTypeToString(const Call* call)
{
    if(IsPureNothing(call))
    {
        return "<Nothing>";
    }

    if(IsNothing(call))
    {
        return "<" + *call->BoundType + "?>";
    }

    return "<" + *call->BoundType + ">";
}

/// returns a string representation of [call]
String CallToString(const Call* call)
{
    if(IsPureNothing(call) || IsNothing(call))
    {
        return CallTypeToString(call) + "\n";
    }

    return StringValueOf(call) + "\n";
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
// Instruction Defintions

/// bytecode instruction
/// consumption: 0
/// assumptions: none
/// argument:    index id of call name with lookup either in SimpleCallNames
///              or CallNames
/// description: leaves the <String*> representing the Call's name as TOS
/// stack state: <String*>
void BCI_LoadCallName(extArg_t arg)
{
    if(arg < SIMPLE_CALLS)
    {
        PushTOS<String>(SimpleCallNames[arg]);
    }
    else
    {
        PushTOS<String>(&CallNames[arg-SIMPLE_CALLS]);
    }
}

/// bytecode instruction
/// consumption: 0
/// assumptions: none 
/// argument:    index of primitive in ConstPrimitives
/// description: leaves the primitive call specified by arg as TOS
/// stack state: <Call>
void BCI_LoadPrimitive(extArg_t arg)
{
    if(arg >= ConstPrimitives.size())
        return; 

    PushTOS<Call>(ConstPrimitives[arg]);
}

/// bytecode instruction
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: reassigns TOS[1] to the bindings on TOS[0] which is retained
///              as TOS
/// stack state: <Call> 
void BCI_Assign(extArg_t arg)
{
    auto rhs = PopTOS<Call>();
    auto lhs = PopTOS<Call>();

    InternalAssign(lhs, rhs);

    PushTOS<Call>(lhs);
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves the addition result as TOS
/// stack state: <Call>
void BCI_Add(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    auto call = ApplyFunction(AddFunction, lCall, rCall, AddTypes, nAddTypes);

    if(call == nullptr)
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot add types %s from %s", CallTypeToString(lCall), CallTypeToString(rCall)));
        return;
    }

    PushTOS(call);
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves the subtraction result as TOS
/// stack state: <Call>
void BCI_Subtract(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    auto call = ApplyFunction(SubtractFunction, lCall, rCall, GenericMathTypes, nGenericMathTypes);

    if(call == nullptr)
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot subtract types %s from %s", CallTypeToString(lCall), CallTypeToString(rCall)));
        return;
    }

    PushTOS(call);
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves the multiplication result as TOS
/// stack state: <Call>
void BCI_Multiply(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    auto call = ApplyFunction(MultiplyFunction, lCall, rCall, GenericMathTypes, nGenericMathTypes);

    if(call == nullptr)
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot multiply types %s from %s", CallTypeToString(lCall), CallTypeToString(rCall)));
        return;
    }

    PushTOS(call);
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves the division result as TOS
/// stack state: <Call>
void BCI_Divide(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    auto call = ApplyFunction(DivideFunction, lCall, rCall, GenericMathTypes, nGenericMathTypes);

    if(call == nullptr)
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot divide types %s from %s", CallTypeToString(lCall), CallTypeToString(rCall)));
        return;
    }

    PushTOS(call);
}

/// bytecode instruction
/// consumption: varies
/// assumptions: varies
/// argument:    0 = print
///              1 = ask
/// description: applies the system function with TOS[0]
/// stack state: <Call>
void BCI_SysCall(extArg_t arg)
{
    switch(arg)
    {
        case 0:
        {
            String msg = CallToString(PeekTOS<Call>());
            ProgramOutput += msg;
            if(g_outputOn)
            {
                std::cout << msg;
            }

            break;
        }

        case 1:
        {
            String s;
            std::getline(std::cin, s);
            std::cout << s;
            auto call = InternalPrimitiveCallConstructor(s);
            PushTOS<Call>(call);
            break;
        }
        
        default:
        break;
    }
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves the and result as TOS
/// stack state: <Call> 
void BCI_And(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    auto call = ApplyFunction(AndFunction, lCall, rCall, GenericBooleanTypes, nGenericBooleanTypes);

    if(call == nullptr)
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot 'and' types %s from %s", CallTypeToString(lCall), CallTypeToString(rCall)));
        return;
    }

    PushTOS(call);
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves the or result as TOS
/// stack state: <Call>  
void BCI_Or(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    auto call = ApplyFunction(OrFunction, lCall, rCall, GenericBooleanTypes, nGenericBooleanTypes);

    if(call == nullptr)
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot 'or' types %s from %s", CallTypeToString(lCall), CallTypeToString(rCall)));
        return;
    }

    PushTOS(call);
}

/// bytecode instruction
/// consumption: 1
/// assumptions: TOS[0] is <Call>
/// description: leaves the not result as TOS
/// stack state: <Call> 
void BCI_Not(extArg_t arg)
{
    if(IsPureNothing(PeekTOS<Call>()))
    {
        return;
    }
     
    auto call = PopTOS<Call>();

    if(!CanBe(&BooleanType, call))
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot 'not' %s", CallTypeToString(call)));
    }

    if(Strictly(&BooleanType, call))
    {
        bool b = !BooleanValueOf(call);
        Call* newCall = InternalPrimitiveCallConstructor(b);
        PushTOS(newCall);
    }
    else
    {
        PushTOS(call);
    }
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves the resulting boolean typed Call as TOS
/// stack state: <Call> 
void BCI_NotEquals(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    bool b = !CallsAreEqual(rCall, lCall);
    Call* call = InternalPrimitiveCallConstructor(b);

    PushTOS(call);
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves the resulting boolean typed Call as TOS
/// stack state: <Call> 
void BCI_Equals(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    bool b = CallsAreEqual(rCall, lCall);
    Call* call = InternalPrimitiveCallConstructor(b);

    PushTOS(call);
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: fills the CmpReg with the result of all numeric comparisons
/// stack state: none 
void BCI_Cmp(extArg_t arg)
{
    CmpReg = CmpRegDefaultValue;
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    auto call = ApplyFunction(CmpFunction, lCall, rCall, GenericMathTypes, nGenericMathTypes);

    if(call == nullptr)
    {
        ReportFatalError(SystemMessageType::Warning, 0, 
            Msg("cannot compare types %s and %s", lCall->BoundType, rCall->BoundType));
        return;
    }

    if(Either(IsNothing, lCall, rCall))
    {
        CmpReg = CmpReg | (BitFlag << 7);
    }
}

/// bytecode instruction
/// consumption: 0
/// assumptions: CmpReg is properly valued
/// argument:    determines the specific comparison to use
/// description: leaves the boolean call for a specific comparison as TOS
/// stack state: <Call>
void BCI_LoadCmp(extArg_t arg)
{
    Call* boolCall = nullptr;
    if(arg < 6 && NthBit(CmpReg, 7) == 0)
    {
        boolCall = InternalPrimitiveCallConstructor(NthBit(CmpReg, arg));
        PushTOS<Call>(boolCall);
    }
    else
    {
        PushTOS(&NothingCall);
    }
}

/// bytecode instruction
/// consumption: 1
/// assumptions: TOS[0] is <Call>
/// argument:    the new instruction id to jump to
/// description: jump to [arg] if TOS[0] is boolean false
/// stack state: none
void BCI_JumpFalse(extArg_t arg)
{
    auto call = PopTOS<Call>();

    if(IsPureNothing(call))
    {
        InternalJumpTo(arg);
    }

    if(Loosely(&BooleanType, call))
    {
        if(Strictly(&BooleanType, call))
        {
            if(!BooleanValueOf(call))
            {
                InternalJumpTo(arg);
            }
        }
        else
        {
            InternalJumpTo(arg);
        }
    }
    else
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot conditionally jump on %s", CallTypeToString(call)));
    }
}

/// bytecode instruction
/// consumption: 0
/// assumptions: none
/// description: unconditionally jumps to [arg]
/// stack state: none
void BCI_Jump(extArg_t arg)
{
    InternalJumpTo(arg);
}

/// TODO: may not work with primitives
/// bytecode instruction
/// consumption: 1
/// assumptions: TOS[0] is <Call>
/// description: creates a deep copy of TOS[0] which is left as the new TOS
/// stack state: <Call>
void BCI_Copy(extArg_t arg)
{
    auto call = PopTOS<Call>();
    auto callCopy = InternalCopyCall(call);
    PushTOS<Call>(callCopy);
}

/// bytecode instruction
/// consumption: 1
/// assumptions: TOS[0] is <Call>
/// description: binds type to TOS[0] or to a shallow copy of it
/// stack state: <Call>
void BCI_BindType(extArg_t arg)
{
    auto originalCall = PeekTOS<Call>();

    if(originalCall->Name != nullptr)
    {
        if(originalCall->BoundScope != &NothingScope)
        {
            PopTOS<Call>();
            auto call = InternalCallConstructor();
            call->NumberOfParameters = originalCall->NumberOfParameters;
            BindType(call, originalCall->Name);
            BindSection(call, originalCall->BoundSection);
            BindValue(call, originalCall->BoundValue);
            
            PushTOS(call);
        }
        else
        {
            BindType(originalCall, originalCall->Name);
        }
    }
    
}

/// bytecode instruction
/// consumption: 1
/// assumptions: TOS[0] is <String>
/// description: resolve TOS[0] into a valid <Call> inside contextual scope
/// stack state: <Call>
void BCI_ResolveDirect(extArg_t arg)
{
    auto callName = PopTOS<String>();

    if(CallNameIsKeyword(callName))
    {
        ResolveKeywordCall(callName);
        return;
    }

    auto resolvedCall = FindInScopeOnlyImmediate(LocalScopeReg, callName);

    if(LocalScopeIsDetachedReg == false)
    {
        if(resolvedCall == nullptr)
        {
            resolvedCall = FindInScopeChain(SelfReg->BoundScope, callName);
        }

        if(resolvedCall == nullptr)
        {
            resolvedCall = FindInScopeOnlyImmediate(CallerReg->BoundScope, callName);
        }

        if(resolvedCall == nullptr)
        {
            resolvedCall = FindInScopeOnlyImmediate(ProgramReg, callName);
        }
    }

    if(resolvedCall == nullptr)
    {
        auto newCall = InternalCallConstructor(callName);
        AddCallToScope(newCall, LocalScopeReg);
        PushTOS<Call>(newCall);
    }
    else
    {
        PushTOS<Call>(resolvedCall);
    }

}

/// bytecode instruction
/// consumption: 1
/// assumptions: TOS[0] is <String>, TOS[1] is <Call>
/// description: resolves TOS[0] into a valid <Call> in TOS[1] scope
/// stack state: <Call>
void BCI_ResolveScoped(extArg_t arg)
{
    auto callName = PopTOS<String>();
    auto callerCall = PopTOS<Call>();

    // all attributes of Nothing resolve to Nothing
    if(IsNothing(callerCall))
    {
        PushTOS<Call>(&NothingCall);
        return;
    }

    auto resolvedCall = FindInScopeOnlyImmediate(ScopeOf(callerCall), callName);

    if(resolvedCall == nullptr)
    {
        auto newCall = InternalCallConstructor(callName);
        AddCallToScope(newCall, ScopeOf(callerCall));
        PushTOS<Call>(newCall);
    }
    else
    {
        PushTOS<Call>(resolvedCall);
    }
}

/// bytecode instruction
/// consumption: 1
/// assumptions: TOS[0] is <Scope>
/// description: creates a new <Call> which is bound to TOS[0] and left as TOS
/// stack state: <Call>
void BCI_BindScope(extArg_t arg)
{
    auto call = InternalCallConstructor();
    auto scope = PopTOS<Scope>();

    call->NumberOfParameters = arg;
    BindScope(call, scope);

    /// TODO: make method types more robust
    BindType(call, &MethodType);

    PushTOS(call);
}

/// bytecode instruction
/// consumption: 0
/// assumptions: TOS[0] is <Call>
/// description: binds [arg] as the section of TOS, 
/// stack state: none
void BCI_BindSection(extArg_t arg)
{
    auto call = PeekTOS<Call>();
    BindSection(call, arg);
}

/// bytecode instruction
/// consumption: 2 + [arg]
/// assumptions: assumes TOS[0 : arg + 2] are <Call>
/// argmument:   the number of parameters
/// description: uses TOS[0 : arg] as parameters, TOS[arg + 1] as the section
///              to evaluate, and TOS[arg + 2] as the caller
///
///              depending on TOS[arg + 1]              
///                 general:
///                 jumps to the section of TOS[arg + 1] and enters a new 
///                 <CallFrame>
///
///                 pure nothing:
///                 leaves NothingCall as TOS
///
///                 primitive type:
///                 executes the built-in primitive constructor method and
///                 leaves the constructed <Call> as TOS. does not jump
///              
/// stack state: varies
void BCI_Eval(extArg_t arg)
{
    auto paramsList = GetParameters(arg);
    auto methodCall = PopTOS<Call>();
    auto caller = PopTOS<Call>();

    InternalEval(methodCall, caller, paramsList, arg, /*inPlace*/ false);
    delete[] paramsList;
}

/// bytecode instruction
/// consumption: 1 + [arg]
/// assumptions: TOS[0 : 1 + arg] are <Call>
/// argmument:   the number of parameters
/// description: TOS[1 + arg] is the <Call> whose section is evaluated (see
///              BCI_Eval)
/// stack state: none
void BCI_EvalHere(extArg_t arg)
{
    auto paramsList = GetParameters(arg);
    auto methodCall = PopTOS<Call>();

    InternalEval(methodCall, &NothingCall, paramsList, arg, /*inPlace*/ true);
    delete[] paramsList;
}

/// bytecode instruction
/// consumption: varies
/// assumptions: varies
/// argument:    arg = 0: depends on CallerReg and SelfReg. returns CallerReg
///                       provided it is not NothingCall. otherwise will return
///                       SelfReg
///              arg = 1: use TOS[0] (is <Call>) as return value. consumes 1.
/// description: pushes the approriate <Call> to TOS
/// stack state: <Call>
void BCI_Return(extArg_t arg)
{
    extArg_t jumpBackTo = CallStack.back().ReturnToInstructionId;
    extArg_t stackStart = CallStack.back().MemoryStackStart;

    Call* returnObj = nullptr;
    if(arg == 1)
    {
        returnObj = PopTOS<Call>();
    }

    while(MemoryStack.size() > stackStart)
    {
        PopTOS<void>();
    }

    if(arg == 1)
    {
        PushTOS(returnObj);
    }
    else
    {
        if(CallerReg->BoundType != &NothingType)
        {
            PushTOS(CallerReg);
        }
        else
        {
            PushTOS(SelfReg);
        }
    }

    InstructionReg = jumpBackTo;

    LastResultReg = CallStack.back().LastResult;
    CallStack.pop_back();

    extArg_t returnMemStart = CallStack.back().MemoryStackStart;

    CallerReg = static_cast<Call*>(MemoryStack[returnMemStart]);
    SelfReg = static_cast<Call*>(MemoryStack[returnMemStart+1]);

    LocalScopeStack() = CallStack.back().LocalScopeStack;
    AdjustLocalScopeReg();

    JumpStatusReg = 1;
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: indexes TOS[1] as an <Call> of <ArrayType> and leaves the
///              <Call> at index TOS[0] as the new TOS.
///
///              enforces that TOS[0] must a <Call> of <IntegerType>
/// stack state: <Call>
void BCI_Array(extArg_t arg)
{
    auto indexCall = PopTOS<Call>();
    auto arrayCall = PopTOS<Call>();

    auto sizeCall = FindInScopeOnlyImmediate(ScopeOf(arrayCall), &SizeCallName);

    if(indexCall->BoundType != &IntegerType)
    {
        ReportFatalError(SystemMessageType::Exception, 5, Msg("%s is not an integer", *indexCall->Name));
        return;
    }

    if(sizeCall == nullptr)
    {
        ReportFatalError(SystemMessageType::Exception, 4, Msg("%s is not an array", *arrayCall->Name));
        return;
    }

    auto indexInt = IntegerValueOf(indexCall);
    auto sizeInt = IntegerValueOf(sizeCall);

    if(indexInt >= sizeInt)
    {
        ReportFatalError(SystemMessageType::Exception, 6, Msg("%i is out of bounds in %s of size %i", indexInt, *arrayCall->Name, sizeInt));
        return;
    }

    auto indexedCall = FindInScopeOnlyImmediate(ScopeOf(arrayCall), CallNamePointerFor(indexInt));
    if(indexCall == nullptr)
    {
        LogIt(LogSeverityType::Sev3_Critical, "BCI_Array", "cannot find indexed call");
    }

    PushTOS(indexedCall);
}


/// bytecode instruction
/// consumption: 0
/// assumptions: none
/// argument:    arg = 0: local contextual scope
/// argument:    arg = 1: detached scope
/// description: creates a new scope which is either local or detached.
///              local scope:
///                 call resolution includes CallerReg, SelfReg and ProgramReg
///                 and is used for control flow statements/method bodies
///
///              detached scope:
///                 call resolution only occurs in the detached scope. 
/// externality: LocalScopeReg, 
///              LocalScopeStack(), 
///              LocalScopeIsDetachedReg, 
///              LastResultReg
/// stack state: none
void BCI_EnterLocal(extArg_t arg)
{
    auto newScope = InternalScopeConstructor(nullptr);
    bool isDetached = (arg == 1 ? true : false);
    LocalScopeStack().push_back({newScope, isDetached});
    LocalScopeReg = LocalScopeStack().back().Value;
    LocalScopeIsDetachedReg = isDetached;
    LastResultReg = nullptr;
}

/// bytecode instruction
/// assumptions: 
/// description: 
/// stack state: 
/// no assumptions
/// leaves the scope as TOS. will update LocalScopeReg and LastResultReg
void BCI_LeaveLocal(extArg_t arg)
{
    PushTOS<Scope>(LocalScopeStack().back().Value);
    LocalScopeStack().pop_back();
    AdjustLocalScopeReg();
    LastResultReg = nullptr;
}

/// bytecode instruction
/// consumption: 0
/// assumptions: none
/// description: adds [arg] as the next uninitialized byte of ExtendedArg
///              (stored by ExtensionExp, which is incremented)
/// externality: ExtensionExp
///              ExtendedArg
/// stack state: none
void BCI_Extend(extArg_t arg)
{
    if(ExtensionExp < 7)
    {
        ExtensionExp++;
        extArg_t shiftedExpr = ((extArg_t)arg) << (8 * ExtensionExp);
        ExtendedArg = ExtendedArg ^ shiftedExpr;
    }
}

/// bytecode instruction
/// consumption: 0
/// assumptions: none
/// description: no operation
/// stack state: none
void BCI_NOP(extArg_t arg)
{
    // does nothing, used for optimizations
}

/// bytecode instruction
/// consumption: 0
/// assumptions: none
/// description: duplicates TOS[0] and leaves this pointer as TOS
/// stack state: same as <TOS[0]>
void BCI_Dup(extArg_t arg)
{
    auto entity = PeekTOS<void>();
    PushTOS<void>(entity);
}

/// bytecode instruction
/// consumption: 0
/// assumptions: TOS[0] is <Call>
/// description: sets LastResultReg to TOS[0]
/// externality: LastResultReg
/// stack state: none
void BCI_EndLine(extArg_t arg)
{
    LastResultReg = PopTOS<Call>();
}

/// bytecode instruction
/// consumption: 1
/// assumptions: none
/// description: remove and discard TOS[0]
/// stack state: none
void BCI_DropTOS(extArg_t arg)
{
    PopTOS<void>();
}

/// bytecode instruction
/// consumption: 2
/// assumptions: TOS[0] is <Call>, TOS[1] is <Call>
/// description: leaves a <Call> of <BooleanType> as TOS which signifies whether
///              TOS[1] 'is' TOS[0]. defines the notion of 'being of'
/// stack state: <Call>
void BCI_Is(extArg_t)
{
    auto rhs = PopTOS<Call>();
    auto lhs = PopTOS<Call>();

    Call* call = nullptr;
    if(IsPureNothing(lhs))
    {
        InternalAssign(lhs, rhs);
        call = InternalPrimitiveCallConstructor(true);   
    }
    else
    {
        bool is = false;
        if(IsNothing(lhs) && !IsNothing(rhs))
        {
            is = lhs->BoundType == rhs->BoundType
                 && lhs->BoundSection == rhs->BoundSection
                 && CallsHaveEqualValue(lhs, rhs); 

        }
        else if(!IsNothing(lhs) && IsNothing(rhs))
        {
            is = lhs->BoundType == rhs->BoundType
                 && lhs->BoundSection == rhs->BoundSection;
        }
        else
        {
            is = CallsAreEqual(lhs, rhs);
        }

        call = InternalPrimitiveCallConstructor(is);
    }

    PushTOS(call);
}
