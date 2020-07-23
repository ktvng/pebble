#include "bytecode.h"

#include "vm.h"
#include "errormsg.h"
#include "flattener.h"

#include "object.h"
#include "main.h"
#include "program.h"
#include "scope.h"
#include "reference.h"
#include "operation.h"
#include "diagnostics.h"


// ---------------------------------------------------------------------------------------------------------------------
// Internal Constructors

Object* InternalObjectConstructor(ObjectClass cls, void* value)
{
    Object* obj = FindExistingObject(cls, value);
    if(obj != nullptr)
    {
        /// TODO: make this more efficient... we don't need to be destroying values because we made them before
        /// we checked that a primitive object with that value/class already existed
        ObjectValueDestructor(cls, value);
        return obj;
    }

    obj = ObjectConstructor(cls, value);

    AddRuntimeObject(obj);
    return obj;
}

Object* InternalBooleanObjectConstructor(bool value)
{
    bool* b = ObjectValueConstructor(value);
    *b = value;
    return InternalObjectConstructor(BooleanClass, b);
}

Reference* InternalReferenceConstructor(String refName, Object* toObject)
{
    Reference* ref = ReferenceConstructor(refName, toObject);

    AddRuntimeReference(ref);
    return ref;
}

// ---------------------------------------------------------------------------------------------------------------------
// Helper methods
int IndexOfNop()
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
    BCI_LoadRefName,
    BCI_LoadPrimitive,
    BCI_Dereference,

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
    BCI_DefType,

    BCI_ResolveDirect,
    BCI_ResolveScoped,

    BCI_DefMethod,
    BCI_EvalHere,
    BCI_Eval,
    BCI_Return,

    BCI_EnterLocal,
    BCI_LeaveLocal,

    BCI_Extend,
    BCI_NOP,
    BCI_Dup,
    BCI_EndLine,

    BCI_Swap
};




// ---------------------------------------------------------------------------------------------------------------------
// Instruction Helpers

/// returns TOS as a Reference*
template <typename T>
inline T* PopTOS()
{
    T* tos = static_cast<T*>(MemoryStack.back());
    MemoryStack.pop_back();
    return tos;
}

template <typename T>
inline void PushTOS(T* entity)
{
    MemoryStack.push_back(static_cast<void*>(entity));
}

template <typename T>
inline T* PeekTOS()
{
    return static_cast<T*>(MemoryStack.back());
}

/// finds and returns a Reference with [refName] in [scope] without checking 
/// the inherited scope
inline Reference* FindInScopeOnlyImmediate(Scope* scope, String& refName)
{
    for(auto ref: scope->ReferencesIndex)
    {
        if(ref->Name == refName)
            return ref;
    }
    return nullptr;
}

/// finds and returns a Reference with [refName] in [scope] and checks
/// the entire chain of inherited scope
inline Reference* FindInScopeChain(Scope* scope, String& refName)
{
    for(auto s = scope; s != nullptr; s = s->InheritedScope)
    {
        for(auto ref: s->ReferencesIndex)
        {
            if(ref->Name == refName)
                return ref;
        }
    }
    return nullptr;
}

/// returns the Scope of [ref]
inline Scope* ScopeOf(Reference* ref)
{
    return ref->To->Attributes;
}

/// returns the Scope of [obj]
inline Scope* ScopeOf(Object* obj)
{
    return obj->Attributes;
}

/// return the size of the MemoryStack
inline size_t MemoryStackSize()
{
    return MemoryStack.size();
}

/// add a new CallFrame to the CallStack with [caller] and [self] the new caller and self
/// objects. changes registers appropriately but does not change the instruction pointer
inline void EnterNewCallFrame(extArg_t callerRefId, Object* caller, Object* self)
{
    std::vector<Scope> localScopeStack;
    CallStack.push_back({ InstructionReg+1, MemoryStackSize(), callerRefId, localScopeStack, LastResultReg });
    PushTOS<Object>(caller);
    PushTOS<Object>(self);

    CallerReg = caller;
    SelfReg = self;
    LocalScopeReg = self->Attributes;
    LastResultReg = nullptr;
}

/// true if both [obj1] and [obj2] are of [cls]
inline bool BothAre(Object* obj1, Object* obj2, ObjectClass cls)
{
    return obj1->Class == cls && obj2->Class == cls;
}

/// adds [ref] to [scp]
inline void AddRefToScope(Reference* ref, Scope* scp)
{
    scp->ReferencesIndex.push_back(ref);
}

/// add a list of Objects [paramsList] to the scope of [methodObj] in reverse order
/// used when evaluating a method
inline void AddParamsToMethodScope(Object* methodObj, std::vector<Object*> paramsList)
{
    if(paramsList.size() != methodObj->ByteCodeParamsAsMethod.size())
    {
        ReportFatalError(SystemMessageType::Exception, 2, 
            Msg("expected %i arguments but got %i", methodObj->ByteCodeParamsAsMethod.size(), paramsList.size()));
    }

    if(paramsList.size() == 0)
    {
        return;
    }

    for(size_t i =0; i<paramsList.size() && i<methodObj->ByteCodeParamsAsMethod.size(); i++)
    {
        auto ref = InternalReferenceConstructor(methodObj->ByteCodeParamsAsMethod[i], paramsList[paramsList.size()-1-i]);
        AddRefToScope(ref, ScopeOf(methodObj));
    }
}

/// pops [numParams] objects from the MemoryStack and returns a list of these elements
/// (which is the reverse order due to the push/pop algorithm)
inline std::vector<Object*> GetParameters(extArg_t numParams)
{
    std::vector<Object*> params;
    params.reserve(numParams);
    for(extArg_t i=0; i<numParams; i++)
    {
        params.push_back(PopTOS<Object>());
    }
    
    return params;
}

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
inline void CompareIntegers(Object* lhs, Object* rhs)
{
    int lVal = GetIntValue(*lhs);
    int rVal = GetIntValue(*rhs);

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
inline void CompareDecimals(Object* lhs, Object* rhs)
{
    double lVal = GetDecimalValue(*lhs);
    double rVal = GetDecimalValue(*rhs);

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

/// changes the LocalScopeReg whenever a LocalScope change occurs. the base scope
/// is either the program scope or the self object scope
void AdjustLocalScopeReg()
{
    if(LocalScopeStack().size() == 0)
    {
        if(SelfReg != &NothingObject)
        {
            LocalScopeReg = SelfReg->Attributes;
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

/// resolves [refName] which is a keyword to the appropriate object which is 
/// pushed to TOS
inline void ResolveReferenceKeyword(const String& refName)
{
    Object* obj = &NothingObject;
    if(refName == "caller")
    {
        obj = CallerReg;
    }
    else if(refName == "self")
    {
        obj = SelfReg;
    }
    else
    {
        if(LastResultReg == nullptr)
        {
            obj = &NothingObject;
        }
        else
        {
            obj = LastResultReg;
        }
    }

    PushTOS<Object>(obj);
}




// ---------------------------------------------------------------------------------------------------------------------
// Instruction Defintions

/// no assumptions
/// leaves the String refName as TOS
void BCI_LoadRefName(extArg_t arg)
{
    PushTOS<String>(&ReferenceNames[arg]);
}

/// no asumptions
/// leaves the primitive specified by arg as TOS
void BCI_LoadPrimitive(extArg_t arg)
{
    if(arg >= ConstPrimitives.size())
        return; 

    PushTOS<Object>(ConstPrimitives[arg]);
}

/// assumes TOS is a ref
/// leaves object of that references as TOS
void BCI_Dereference(extArg_t arg)
{
    auto ref = PopTOS<Reference>();
    PushTOS<Object>(ref->To);
}

/// assumes TOS is the new object and TOS1 is the reference to reassign
/// leaves the object
void BCI_Assign(extArg_t arg)
{
    auto obj = PopTOS<Object>();
    auto ref = PopTOS<Reference>();

    ref->To = obj;
    PushTOS<Object>(obj);
}

/// assumes TOS1 and TOS2 are rhs object and lhs object respectively
/// leaves the result as TOS
void BCI_Add(extArg_t arg)
{
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();

    Object* obj = nullptr;
    if(BothAre(lObj, rObj, IntegerClass))
    {
        int i = GetIntValue(*lObj) + GetIntValue(*rObj);
        int* ans = ObjectValueConstructor(i);
        obj = InternalObjectConstructor(IntegerClass, ans);
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        double d = GetIntValue(*lObj) + GetIntValue(*rObj);
        double* ans = ObjectValueConstructor(d);
        obj = InternalObjectConstructor(DecimalClass, ans);
    }
    else if(BothAre(lObj, rObj, StringClass))
    {
        String s = GetStringValue(*lObj) + GetStringValue(*rObj);
        String* ans = ObjectValueConstructor(s);
        obj = InternalObjectConstructor(StringClass, ans);
    }
    else
    {
        obj = &NothingObject;
        ReportError(SystemMessageType::Warning, 0, Msg("cannot add types %s and %s", lObj->Class, rObj->Class));
    }

    PushTOS<Object>(obj);
}

void BCI_Subtract(extArg_t arg)
{
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();

    Object* obj = nullptr;
    if(BothAre(lObj, rObj, IntegerClass))
    {
        int i = GetIntValue(*lObj) - GetIntValue(*rObj);
        int* ans = ObjectValueConstructor(i);
        obj = InternalObjectConstructor(IntegerClass, ans);
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        double d = GetIntValue(*lObj) - GetIntValue(*rObj);
        double* ans = ObjectValueConstructor(d);
        obj = InternalObjectConstructor(DecimalClass, ans);
    }
    else
    {
        obj = &NothingObject;
        ReportError(SystemMessageType::Warning, 0, Msg("cannot subtract types %s and %s", lObj->Class, rObj->Class));
    }

    PushTOS<Object>(obj);
}

void BCI_Multiply(extArg_t arg)
{
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();

    Object* obj = nullptr;
    if(BothAre(lObj, rObj, IntegerClass))
    {
        int i = GetIntValue(*lObj) * GetIntValue(*rObj);
        int* ans = ObjectValueConstructor(i);
        obj = InternalObjectConstructor(IntegerClass, ans);
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        double d = GetIntValue(*lObj) * GetIntValue(*rObj);
        double* ans = ObjectValueConstructor(d);
        obj = InternalObjectConstructor(DecimalClass, ans);
    }
    else
    {
        obj = &NothingObject;
        ReportError(SystemMessageType::Warning, 0, Msg("cannot multiply types %s and %s", lObj->Class, rObj->Class));
    }

    PushTOS<Object>(obj);
}

void BCI_Divide(extArg_t arg)
{
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();

    Object* obj = nullptr;
    if(BothAre(lObj, rObj, IntegerClass))
    {
        int i = GetIntValue(*lObj) / GetIntValue(*rObj);
        int* ans = ObjectValueConstructor(i);
        obj = InternalObjectConstructor(IntegerClass, ans);
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        double d = GetIntValue(*lObj) / GetIntValue(*rObj);
        double* ans = ObjectValueConstructor(d);
        obj = InternalObjectConstructor(DecimalClass, ans);
    }
    else
    {
        obj = &NothingObject;
        ReportError(SystemMessageType::Warning, 0, Msg("cannot divide types %s and %s", lObj->Class, rObj->Class));
    }

    PushTOS<Object>(obj);
}

/// 0 = print
/// 1 = ask
void BCI_SysCall(extArg_t arg)
{
    switch(arg)
    {
        case 0:
        {
            String msg = GetStringValue(*PeekTOS<Object>()) + "\n";
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
            String* str = ObjectValueConstructor(s);
            auto obj = InternalObjectConstructor(StringClass, str);
            PushTOS<Object>(obj);
        }
        break;
        
        default:
        break;
    }
}

void BCI_And(extArg_t arg)
{
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();

    bool b = GetBoolValue(*lObj) && GetBoolValue(*rObj);
    bool* ans = ObjectValueConstructor(b);
    Object* obj = InternalObjectConstructor(BooleanClass, ans);

    PushTOS<Object>(obj);
}

void BCI_Or(extArg_t arg)
{
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();

    bool b = GetBoolValue(*lObj) || GetBoolValue(*rObj);
    bool* ans = ObjectValueConstructor(b);
    Object* obj = InternalObjectConstructor(BooleanClass, ans);

    PushTOS<Object>(obj);
}

void BCI_Not(extArg_t arg)
{
    auto obj = PopTOS<Object>();

    bool b =!GetBoolValue(*obj);
    bool* ans = ObjectValueConstructor(b);
    Object* newObj = InternalObjectConstructor(BooleanClass, ans);

    PushTOS<Object>(newObj);
}

/// assumes TOS1 1 and TOS2 are rhs object and lhs object respectively
/// leaves obj (bool compare result) as TOS
void BCI_NotEquals(extArg_t arg)
{
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();

    auto boolObj = InternalBooleanObjectConstructor(rObj != lObj);
    PushTOS<Object>(boolObj);
}

/// assumes TOS1 1 and TOS2 are rhs object and lhs object respectively
/// leaves obj (bool compare result) as TOS
void BCI_Equals(extArg_t arg)
{
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();

    auto boolObj = InternalBooleanObjectConstructor(rObj == lObj);
    PushTOS<Object>(boolObj);
}

/// assumes TOS1 1 and TOS2 are rhs object and lhs object respectively
/// sets CmpReg to appropriate value
void BCI_Cmp(extArg_t arg)
{
    CmpReg = CmpRegDefaultValue;
    auto rObj = PopTOS<Object>();
    auto lObj = PopTOS<Object>();
    

    if(BothAre(lObj, rObj, IntegerClass))
    {
        CompareIntegers(lObj, rObj);
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        CompareDecimals(lObj, rObj);
    }
    else
    {
        ReportError(SystemMessageType::Warning, 0, Msg("cannot compare types %s and %s", lObj->Class, rObj->Class));
    }
}

/// assumes CmpReg is valued
/// leaves a new Boolean object as TOS 
void BCI_LoadCmp(extArg_t arg)
{
    Object* boolObj = nullptr;
    switch(arg)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: 
        case 5:
        case 6:
        boolObj = InternalBooleanObjectConstructor(NthBit(CmpReg, arg));
        break;

        default:
        return;
    }

    PushTOS<Object>(boolObj);
}

void BCI_JumpFalse(extArg_t arg)
{
    auto obj = PopTOS<Object>();
    if(!GetBoolValue(*obj))
    {
        InstructionReg = arg;
        JumpStatusReg = 1;
    }
}

void BCI_Jump(extArg_t arg)
{
    InstructionReg = arg;
    JumpStatusReg = 1;
}

/// assumes TOS is an object
/// leaves object copy as TOS
void BCI_Copy(extArg_t arg)
{
    auto obj = PopTOS<Object>();
    auto objCopy = InternalObjectConstructor(obj->Class, obj->Value);
    objCopy->BlockStartInstructionId = obj->BlockStartInstructionId;
    
    /// TODO: maybe take this out?
    for(auto ref: obj->Attributes->ReferencesIndex)
    {
        auto refcopy = InternalReferenceConstructor(ref->Name, ref->To);
        AddRefToScope(refcopy, objCopy->Attributes);
    }

    for(auto str: obj->ByteCodeParamsAsMethod)
    {
        objCopy->ByteCodeParamsAsMethod.push_back(str);
    }

    PushTOS<Object>(objCopy);
}

/// assumes TOS is a string (typeName)
/// leaves object of type as TOS
void BCI_DefType(extArg_t arg)
{
    auto typeName = *PopTOS<String>();
    auto obj = InternalObjectConstructor(typeName, nullptr);

    /// TODO: make this a new method to explain/add documentation
    obj->BlockStartInstructionId = InstructionReg + NOPSafetyDomainSize() + 3;

    PushTOS<Object>(obj);
}

/// assumes TOS is a String
/// leaves resolved reference as TOS
void BCI_ResolveDirect(extArg_t arg)
{
    auto refName = PopTOS<String>();

    if(RefNameIsKeyword(*refName))
    {
        ResolveReferenceKeyword(*refName);
        return;
    }

    auto resolvedRef = FindInScopeOnlyImmediate(LocalScopeReg, *refName);
    if(resolvedRef == nullptr)
    {
        resolvedRef = FindInScopeChain(SelfReg->Attributes, *refName);
    }

    if(resolvedRef == nullptr)
    {
        resolvedRef = FindInScopeOnlyImmediate(CallerReg->Attributes, *refName);
    }

    if(resolvedRef == nullptr)
    {
        resolvedRef = FindInScopeOnlyImmediate(ProgramReg, *refName);
    }

    if(resolvedRef == nullptr)
    {
        auto newRef = InternalReferenceConstructor(*refName, &NothingObject);
        AddRefToScope(newRef, LocalScopeReg);
        PushTOS<Reference>(newRef);
    }
    else
    {
        PushTOS<Reference>(resolvedRef);
    }

}

/// assumes TOS is a string and TOS1 is an obj
/// leaves resolved reference as TOS
void BCI_ResolveScoped(extArg_t arg)
{
    auto refName = PopTOS<String>();

    if(PeekTOS<Object>() == &NothingObject || PeekTOS<Object>() == &NothingObject)
    {
        ReportError(SystemMessageType::Warning, 1, Msg(" %s refers to the object <%s> which cannot have attributes", PeekTOS<Reference>()->Name, PeekTOS<Reference>()->To->Class));
        return;
    }
    auto callerObj = PopTOS<Object>();
    auto resolvedRef = FindInScopeOnlyImmediate(ScopeOf(callerObj), *refName);

    if(resolvedRef == nullptr)
    {
        auto newRef = InternalReferenceConstructor(*refName, &NothingObject);
        AddRefToScope(newRef, ScopeOf(callerObj));
        PushTOS<Reference>(newRef);
    }
    else
    {
        PushTOS<Reference>(resolvedRef);
    }
}

/// assumes the top [arg] entities on the stack are parameter names (strings)
/// leaves a new method object as TOS
void BCI_DefMethod(extArg_t arg)
{
    String* argList;

    if(arg != 0)
    {
        argList = new String[arg];
        for(extArg_t i = 1; i<=arg; i++)
        {
            auto str = *PopTOS<String>();
            argList[arg-i] = str;
        }
    }

    auto obj = InternalObjectConstructor(MethodClass, nullptr);

    if(arg != 0)
    {
        for(extArg_t i=0; i<arg; i++)
        {
            obj->ByteCodeParamsAsMethod.push_back(argList[i]);
        }
        
        delete[] argList;
    }


    /// expects a block
    /// TODO: verify block
    /// TODO: currently assumes NOPS (AND) and Assign after
    obj->BlockStartInstructionId = InstructionReg + NOPSafetyDomainSize() + 3;

    PushTOS<Object>(obj);
}

/// arg is number of parameters
/// assumes TOS[arg] are objs (params), TOS[arg+1] is an object (method), TOS[arg+2] an obj (caller)
/// adds caller and self to TOS (in that order)
void BCI_Eval(extArg_t arg)
{
    auto paramsList = GetParameters(arg);
    auto methodObj = PopTOS<Object>();
    auto callerObj = PopTOS<Object>();

    LogDiagnostics(methodObj);
    LogDiagnostics(callerObj);

    extArg_t jumpTo = methodObj->BlockStartInstructionId;
    AddParamsToMethodScope(methodObj, paramsList);

    /// TODO: figure out caller id
    EnterNewCallFrame(0, callerObj, methodObj);
    
    InstructionReg = jumpTo;
    JumpStatusReg = 1;
}

/// arg is number of parameters
/// assumes TOS[arg] are objs (params), TOS[arg+1] is an object (method)
/// adds caller and self to TOS (in that order)
void BCI_EvalHere(extArg_t arg)
{
    auto paramsList = GetParameters(arg);
    auto methodObj = PopTOS<Object>();

    extArg_t jumpTo = methodObj->BlockStartInstructionId;
    AddParamsToMethodScope(SelfReg, paramsList);

    /// TODO: figure out caller id
    EnterNewCallFrame(0, CallerReg, SelfReg);
    
    InstructionReg = jumpTo;
    JumpStatusReg = 1;
}

/// arg is bool flag on whether or not to return a specific object
void BCI_Return(extArg_t arg)
{
    extArg_t jumpBackTo = CallStack.back().ReturnToInstructionId;
    extArg_t stackStart = CallStack.back().MemoryStackStart;

    Object* returnObj = nullptr;
    if(arg == 1)
    {
        returnObj = PopTOS<Object>();
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
        PushTOS(SelfReg);
    }

    InstructionReg = jumpBackTo;

    LastResultReg = CallStack.back().LastResult;
    CallStack.pop_back();

    extArg_t returnMemStart = CallStack.back().MemoryStackStart;

    CallerReg = static_cast<Object*>(MemoryStack[returnMemStart]);
    SelfReg = static_cast<Object*>(MemoryStack[returnMemStart+1]);

    LocalScopeStack() = CallStack.back().LocalScopeStack;
    AdjustLocalScopeReg();

    JumpStatusReg = 1;
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
    LastResultReg = PopTOS<Object>();
}

/// swaps TOS with TOS1
void BCI_Swap(extArg_t arg)
{
    void* TOS = PopTOS<void*>();
    void* TOS1 = PopTOS<void*>();

    PushTOS<void>(TOS);
    PushTOS<void>(TOS1);
}
