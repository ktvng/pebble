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
    if(scopeToCopy == &NothingScope || scopeToCopy == nullptr)
    {
       scope = ScopeConstructor(nullptr);
    }
    else
    {
        scope = CopyScope(scopeToCopy);
    }
    
    AddRuntimeScope(scope);

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
    BCI_DefMethod,
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
    BCI_Swap,
    BCI_JumpNothing,

    BCI_DropTOS,
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

/// adds [call] to [scp]
inline void AddCallToScope(Call* call, Scope* scp)
{
    scp->CallsIndex.push_back(call);
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
    std::vector<Scope> localScopeStack;
    CallStack.push_back({ InstructionReg+1, MemoryStackSize(), callerRefId, localScopeStack, LastResultReg });
    PushTOS<Call>(caller);
    PushTOS<Call>(self);

    CallerReg = caller;
    SelfReg = self;
    LocalScopeReg = self->BoundScope;
    LastResultReg = nullptr;
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
    }
    else
    {
        LocalScopeReg = &LocalScopeStack().back();
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Type system helpers

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

/// true if both [call1] and [call2] are [type]
inline bool BothAre(const Call* call1, const Call* call2, const BindingType type)
{
    return call1->BoundType == type && call2->BoundType == type;
}

/// true if either [call1] or [call2] are Nothing
inline bool EitherIsNothing(const Call* call1, const Call* call2)
{
    return IsNothing(call1) || IsNothing(call2);
}

/// true if [call] is [type] and is bound to a realized scope (not NothingScope)
inline bool IsActually(const BindingType type, const Call* call)
{
    return call->BoundScope != &NothingScope && call->BoundType == type;
}



// ---------------------------------------------------------------------------------------------------------------------
// Eval instruction helpers

/// add a list of Calls [paramsList] to the scope of [methodCall] in reverse order
/// used when evaluating a method
inline void AddParamsToMethodScope(Call* methodCall, std::vector<Call*> paramsList)
{
    if(paramsList.size() != methodCall->BoundScope->CallParameters.size())
    {
        ReportFatalError(SystemMessageType::Exception, 2, 
            Msg("expected %i arguments but got %i", methodCall->BoundScope->CallParameters.size(), paramsList.size()));
    }

    if(paramsList.size() == 0)
    {
        return;
    }

    for(size_t i =0; i<paramsList.size() && i<methodCall->BoundScope->CallParameters.size(); i++)
    {
        auto call = InternalCallConstructor(methodCall->BoundScope->CallParameters[i]);
        auto callParam = paramsList[paramsList.size()-1-i];
        BindScope(call, callParam->BoundScope);
        BindSection(call, callParam->BoundSection);
        /// TODO: type checking here
        BindType(call, callParam->BoundType);
        BindValue(call, callParam->BoundValue);

        AddCallToScope(call, ScopeOf(methodCall));
    }
}

/// pops [numParams] objects from the MemoryStack and returns a list of these elements
/// (which is the reverse order due to the push/pop algorithm)
inline std::vector<Call*> GetParameters(extArg_t numParams)
{
    std::vector<Call*> params;
    params.reserve(numParams);
    for(extArg_t i=0; i<numParams; i++)
    {
        params.push_back(PopTOS<Call>());
    }
    
    return params;
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
inline void CompareIntegers(Call* lhs, Call* rhs)
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
inline void CompareDecimals(Call* lhs, Call* rhs)
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
// Equality operators

/// true if [call1] and [call2] have the same value. assumes that neither are
/// Nothing
inline bool CallsHaveEqualValue(const Call* call1, const Call* call2)
{
    if(BothAre(call1, call2, &IntegerType))
    {
        return call1->BoundValue.i == call2->BoundValue.i;
    }
    else if(BothAre(call1, call2, &DecimalType))
    {
        return call1->BoundValue.d == call2->BoundValue.d;
    }
    else if(BothAre(call1, call2, &StringType))
    {
        return call1->BoundValue.s == call2->BoundValue.s;
    }
    else if(BothAre(call1, call2, &BooleanType))
    {
        return call1->BoundValue.b == call2->BoundValue.b;
    }
    else
    {
        return true;
    }
}

/// true if Calls are equal (i.e. defines the definition of equality in Pebble)
inline bool CallsAreEqual(const Call* call1, const Call* call2)
{
    if(call1->BoundType == &NothingType || call2->BoundType == &NothingType)
    {
        return call1->BoundScope == call2->BoundScope;
    }

    return call1->BoundType == call2->BoundType &&
        call1->BoundScope == call2->BoundScope &&
        call1->BoundSection == call2->BoundSection &&
        CallsHaveEqualValue(call1, call2);
}



// ---------------------------------------------------------------------------------------------------------------------
// Scope resolution helpers

/// resolves [callName] which is a keyword to the appropriate object which is 
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
    return type == &ArrayType || type == &ObjectType; 
}

void HandlePrimitiveTypeInstantiation(Call* primitiveCall, std::vector<Call*>& paramsList)
{
    if(primitiveCall->BoundType == &ArrayType)
    {
        HandleArrayInitialization(primitiveCall, paramsList[0]);
    }
    else if(primitiveCall->BoundType == &ObjectType)
    {
        HandleObjectInitialization(primitiveCall);
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Print/Say helpers

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
// Instruction Defintions

/// no assumptions
/// leaves the String callName as TOS
void BCI_LoadCallName(extArg_t arg)
{
    PushTOS<String>(&CallNames[arg]);
}

/// no asumptions
/// leaves the primitive specified by arg as TOS
void BCI_LoadPrimitive(extArg_t arg)
{
    if(arg >= ConstPrimitives.size())
        return; 

    PushTOS<Call>(ConstPrimitives[arg]);
}

/// assumes TOS is the new object and TOS1 is the reference to reassign
/// leaves the object
void BCI_Assign(extArg_t arg)
{
    auto rhs = PopTOS<Call>();
    auto lhs = PopTOS<Call>();

    LogDiagnostics(rhs);
    LogDiagnostics(lhs);

    if(lhs == &NothingCall)
    {
        ReportFatalError(SystemMessageType::Exception, 7, 
            Msg("cannot assign %s to %s", *lhs->BoundType, *rhs->BoundType));
    }

    if(lhs->BoundType == &NothingType)
    {
        if(lhs->BoundScope == &NothingScope)
        {
            BindType(lhs, rhs->BoundType);
            BindSection(lhs, rhs->BoundSection);
            BindScope(lhs, rhs->BoundScope);
            lhs->BoundValue = rhs->BoundValue;
        }
    }
    else
    {
        if(lhs->BoundType == rhs->BoundType)
        {
            BindType(lhs, rhs->BoundType);
            BindSection(lhs, rhs->BoundSection);
            BindScope(lhs, rhs->BoundScope);
            lhs->BoundValue = rhs->BoundValue;
        }
        else
        {
            ReportFatalError(SystemMessageType::Exception, 0,
                Msg("cannot assign %s to %s", *lhs->BoundType, *rhs->BoundType));
            return;
        }
    }


    PushTOS<Call>(lhs);
}

/// assumes TOS1 and TOS2 are rhs object and lhs object respectively
/// leaves the result as TOS
void BCI_Add(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    Call* call = nullptr;
    if(EitherIsNothing(lCall, rCall))
    {
        call = &NothingCall;
    }
    else if(BothAre(lCall, rCall, &IntegerType))
    {
        int ans = IntegerValueOf(lCall) + IntegerValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else if(BothAre(lCall, rCall, &DecimalType))
    {
        double ans = DecimalValueOf(lCall) + DecimalValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else if(BothAre(lCall, rCall, &StringType))
    {
        String ans = StringValueOf(lCall) + StringValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else
    {
        call = &NothingCall;
        // ReportError(SystemMessageType::Warning, 0, Msg("cannot add types %s and %s", lCall->BoundType, rCall->BoundType));
    }

    PushTOS<Call>(call);
}

void BCI_Subtract(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    Call* call = nullptr;
    if(EitherIsNothing(lCall, rCall))
    {
        call = &NothingCall;
    }
    else if(BothAre(lCall, rCall, &IntegerType))
    {
        int ans = IntegerValueOf(lCall) - IntegerValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else if(BothAre(lCall, rCall, &DecimalType))
    {
        double ans = DecimalValueOf(lCall) - DecimalValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else
    {
        call = &NothingCall;
        // ReportError(SystemMessageType::Warning, 0, Msg("cannot add types %s from %s", rCall->BoundType, lCall->BoundType));
    }

    PushTOS<Call>(call);
}

void BCI_Multiply(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    Call* call = nullptr;
    if(EitherIsNothing(lCall, rCall))
    {
        call = &NothingCall;
    }
    else if(BothAre(lCall, rCall, &IntegerType))
    {
        int ans = IntegerValueOf(lCall) * IntegerValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else if(BothAre(lCall, rCall, &DecimalType))
    {
        double ans = DecimalValueOf(lCall) * DecimalValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else
    {
        call = &NothingCall;
        // ReportError(SystemMessageType::Warning, 0, Msg("cannot multiply types %s from %s", lCall->BoundType, rCall->BoundType));
    }

    PushTOS<Call>(call);
}

void BCI_Divide(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    Call* call = nullptr;
    if(EitherIsNothing(lCall, rCall))
    {
        call = &NothingCall;
    }
    else if(BothAre(lCall, rCall, &IntegerType))
    {
        int ans = IntegerValueOf(lCall) / IntegerValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else if(BothAre(lCall, rCall, &DecimalType))
    {
        double ans = DecimalValueOf(lCall) / DecimalValueOf(rCall);
        call = InternalPrimitiveCallConstructor(ans);
    }
    else
    {
        call = &NothingCall;
        // ReportError(SystemMessageType::Warning, 0, Msg("cannot divide types %s by %s", lCall->BoundType, rCall->BoundType));
    }

    PushTOS<Call>(call);
}

/// 0 = print
/// 1 = ask
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
        }
        break;

        case 1:
        {
            String s;
            std::getline(std::cin, s);
            std::cout << s;
            auto call = InternalPrimitiveCallConstructor(s);
            PushTOS<Call>(call);
        }
        break;
        
        default:
        break;
    }
}

/// TODO: nothing proof this
/// TODO: type proof this
void BCI_And(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    if(EitherIsNothing(lCall, rCall))
    {
        PushTOS<Call>(&NothingCall);    
        return;
    }

    bool b = 0;
    if(IsActually(&BooleanType, lCall) && IsActually(&BooleanType, rCall))
    {
        b = BooleanValueOf(lCall) && BooleanValueOf(rCall);
    }
    else
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot 'or' %s with %s", CallTypeToString(lCall), CallTypeToString(rCall)));
    }

    Call* call = InternalPrimitiveCallConstructor(b);
    PushTOS<Call>(call);
}

void BCI_Or(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    if(EitherIsNothing(lCall, rCall))
    {
        PushTOS<Call>(&NothingCall);    
        return;
    }

    bool b = 0;
    if(IsActually(&BooleanType, lCall) && IsActually(&BooleanType, rCall))
    {
        b = BooleanValueOf(lCall) || BooleanValueOf(rCall);
    }
    else
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot 'or' %s with %s", CallTypeToString(lCall), CallTypeToString(rCall)));
    }
    Call* call = InternalPrimitiveCallConstructor(b);
    PushTOS<Call>(call);
}

void BCI_Not(extArg_t arg)
{
    if(IsPureNothing(PeekTOS<Call>()))
    {
        return;
    }
     
    auto call = PopTOS<Call>();

    if(!IsActually(&BooleanType, call))
    {
        ReportFatalError(SystemMessageType::Exception, 0, 
            Msg("cannot 'not' %s", *call->BoundType));
    }

    bool b = !BooleanValueOf(call);
    Call* newCall = InternalPrimitiveCallConstructor(b);
    PushTOS<Call>(newCall);
}

/// assumes TOS1 1 and TOS2 are rhs object and lhs object respectively
/// leaves obj (bool compare result) as TOS
void BCI_NotEquals(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    bool b = !CallsAreEqual(rCall, lCall);
    Call* call = InternalPrimitiveCallConstructor(b);

    PushTOS<Call>(call);
}

/// assumes TOS1 1 and TOS2 are rhs object and lhs object respectively
/// leaves obj (bool compare result) as TOS
void BCI_Equals(extArg_t arg)
{
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();

    bool b = CallsAreEqual(rCall, lCall);
    Call* call = InternalPrimitiveCallConstructor(b);

    PushTOS<Call>(call);
}

/// assumes TOS1 1 and TOS2 are rhs object and lhs object respectively
/// sets CmpReg to appropriate value
void BCI_Cmp(extArg_t arg)
{
    CmpReg = CmpRegDefaultValue;
    auto rCall = PopTOS<Call>();
    auto lCall = PopTOS<Call>();
    

    if(BothAre(lCall, rCall, &IntegerType))
    {
        CompareIntegers(lCall, rCall);
    }
    else if(BothAre(lCall, rCall, &DecimalType))
    {
        CompareDecimals(lCall, rCall);
    }
    else
    {
        ReportError(SystemMessageType::Warning, 0, Msg("cannot compare types %s and %s", lCall->BoundType, rCall->BoundType));
    }
}

/// assumes CmpReg is valued
/// leaves a new Boolean object as TOS 
void BCI_LoadCmp(extArg_t arg)
{
    Call* boolCall = nullptr;
    switch(arg)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: 
        case 5:
        case 6:
        boolCall = InternalPrimitiveCallConstructor(NthBit(CmpReg, arg));
        break;

        default:
        return;
    }

    PushTOS<Call>(boolCall);
}

void BCI_JumpFalse(extArg_t arg)
{
    auto call = PopTOS<Call>();

    if(IsNothing(call))
    {
        InternalJumpTo(arg);
    }

    if(IsActually(&BooleanType, call))
    {
        if(!BooleanValueOf(call))
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

void BCI_Jump(extArg_t arg)
{
    InternalJumpTo(arg);
}

/// assumes TOS is an object
/// TODO: may not work with primitives
/// TODO: should deep copy scope
/// TODO: maybe depreciated
/// leaves object copy as TOS
void BCI_Copy(extArg_t arg)
{
    auto call = PopTOS<Call>();
    auto callCopy = InternalCallConstructor();
    
    auto scp = InternalCopyScope(call->BoundScope);
    BindScope(callCopy, scp);
    BindType(callCopy, call->BoundType);
    BindSection(callCopy, call->BoundSection);

    PushTOS<Call>(callCopy);
}

/// assumes TOS is a Call
/// leaves object of type as TOS
void BCI_BindType(extArg_t arg)
{
    auto originalCall = PeekTOS<Call>();

    if(originalCall->Name != nullptr)
    {
        if(originalCall->BoundScope != &NothingScope)
        {
            PopTOS<Call>();
            auto call = InternalCallConstructor();
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

/// assumes TOS is a String
/// leaves resolved reference as TOS
void BCI_ResolveDirect(extArg_t arg)
{
    auto callName = PopTOS<String>();

    if(CallNameIsKeyword(callName))
    {
        ResolveKeywordCall(callName);
        return;
    }

    auto resolvedCall = FindInScopeOnlyImmediate(LocalScopeReg, callName);
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

/// assumes TOS is a string and TOS1 is an obj
/// leaves resolved reference as TOS
void BCI_ResolveScoped(extArg_t arg)
{
    auto callName = PopTOS<String>();
    auto callerCall = PopTOS<Call>();

    // all attributes of Nothing resolve to Nothing
    if(callerCall->BoundScope == &NothingScope)
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

/// assumes the top [arg] entities on the stack are parameter names (strings)
/// leaves a new method object as TOS
void BCI_DefMethod(extArg_t arg)
{
    String** argList;

    if(arg != 0)
    {
        argList = new String*[arg];
        for(extArg_t i = 1; i<=arg; i++)
        {
            auto str = PopTOS<String>();
            argList[arg-i] = str;
        }
    }

    auto call = InternalCallConstructor();
    auto scope =  InternalScopeConstructor(nullptr);
    BindScope(call, scope);
    BindType(call, &MethodType);

    if(arg != 0)
    {
        for(extArg_t i=0; i<arg; i++)
        {
            call->BoundScope->CallParameters.push_back(argList[i]);
        }
        
        delete[] argList;
    }

    PushTOS<Call>(call);
}

/// assumes TOS is a Call
/// binds arg as the section for that call
void BCI_BindSection(extArg_t arg)
{
    auto call = PeekTOS<Call>();
    BindSection(call, arg);
}

/// arg is number of parameters
/// assumes TOS[arg] are objs (params), TOS[arg+1] is an object (method), TOS[arg+2] an obj (caller)
/// adds caller and self to TOS (in that order)
void BCI_Eval(extArg_t arg)
{
    auto paramsList = GetParameters(arg);
    auto methodCall = PopTOS<Call>();
    auto caller = PopTOS<Call>();

    if(methodCall->BoundScope == &NothingScope)
    {
        PushTOS<Call>(methodCall);
        return;
    }

    if(IsPrimitiveMethodType(methodCall->BoundType))
    {
        HandlePrimitiveTypeInstantiation(methodCall, paramsList);
        return;
    }

    if(methodCall->BoundSection == 0)
    {
        auto methodName = (methodCall->Name == nullptr ? "anonymous variable" : *methodCall->Name);
        ReportFatalError(SystemMessageType::Exception, 3, Msg("%s cannot be called", methodName));
        return;
    }

    extArg_t jumpIns= methodCall->BoundSection;
    AddParamsToMethodScope(methodCall, paramsList);

    /// TODO: figure out caller id
    EnterNewCallFrame(0, caller, methodCall);
    
    InternalJumpTo(jumpIns);
}

/// arg is number of parameters
/// assumes TOS[arg] are objs (params), TOS[arg+1] is an object (method)
/// adds caller and self to TOS (in that order)
void BCI_EvalHere(extArg_t arg)
{
    auto paramsList = GetParameters(arg);
    auto methodCall = PopTOS<Call>();

    extArg_t jumpIns= methodCall->BoundSection;
    AddParamsToMethodScope(SelfReg, paramsList);

    /// TODO: figure out caller id
    EnterNewCallFrame(0, CallerReg, SelfReg);
    
    InternalJumpTo(jumpIns);
}

/// arg is bool flag on whether or not to return a specific object
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

/// assumes TOS is index and TOS1 is the parent call
/// leaves the Call indexed as TOS
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

/// no assumptions
/// does not change TOS. will update LocalScopeReg
void BCI_EnterLocal(extArg_t arg)
{
    LocalScopeStack().push_back( {{}, LocalScopeReg, false} );
    LocalScopeReg = &LocalScopeStack().back();
    LastResultReg = nullptr;
}

/// no assumptions
/// does not change TOS. will update LocalScopeReg and LastResultReg
void BCI_LeaveLocal(extArg_t arg)
{
    /// TODO: destroy andy local references
    LocalScopeStack().pop_back();
    AdjustLocalScopeReg();
    LastResultReg = nullptr;
}

/// no assumptions
/// adds [arg] as the (ExtensionExp + 1)th bit of ExtendedArg and increments ExtensionExp
void BCI_Extend(extArg_t arg)
{
    switch (ExtensionExp)
    {
        case 0:
        case 1:
        case 2: 
        case 3:
        case 4:
        case 5:
        case 6:
        {
            ExtensionExp++;
            extArg_t shiftedExpr = ((extArg_t)arg) << (8 * ExtensionExp);
            ExtendedArg = ExtendedArg ^ shiftedExpr;
            break;
        }


        default:
        break;
    }
}

/// no assumptions
/// no actions, used for optimizations
void BCI_NOP(extArg_t arg)
{
    // does nothing, used for optimizations
}

/// no assumptions
/// pushes a pointer to the old TOS to the TOS
void BCI_Dup(extArg_t arg)
{
    auto entity = PeekTOS<void>();
    PushTOS<void>(entity);
}

/// assumes TOS is an object
/// changes LastResultReg to this object
void BCI_EndLine(extArg_t arg)
{
    LastResultReg = PopTOS<Call>();
}

/// swaps TOS with TOS1
void BCI_Swap(extArg_t arg)
{
    void* TOS = PopTOS<void>();
    void* TOS1 = PopTOS<void>();

    PushTOS<void>(TOS);
    PushTOS<void>(TOS1);
}

/// assumes TOS is an object
/// pops TOS and jumps if obj == Nothing
/// TODO: rename to JumpUndefined
void BCI_JumpNothing(extArg_t arg)
{
    auto TOS = PopTOS<Call>();
    if(TOS->BoundType == &NothingType)
    {
        InternalJumpTo(arg);
    }
}

/// assumes TOS exists
/// pops TOS 
void BCI_DropTOS(extArg_t arg)
{
    PopTOS<void>();
}
