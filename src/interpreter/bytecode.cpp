#include "bytecode.h"

#include "vm.h"
#include "errormsg.h"

#include "object.h"
#include "main.h"
#include "program.h"
#include "scope.h"
#include "reference.h"
#include "operation.h"
#include "diagnostics.h"


// ---------------------------------------------------------------------------------------------------------------------
// Internal Constructors

Object* INTERNAL_ObjectConstructor(ObjectClass cls, void* value)
{
    Object* obj = new Object;
    obj->Attributes = ScopeConstructor(nullptr);
    obj->Class = cls;
    obj->Value = value;
    obj->Action = nullptr;
    obj->DefinitionScope = nullptr;

    return obj;
}

Object* INTERNAL_BooleanObjectConstructor(bool value)
{
    bool* b = new bool;
    *b = value;
    return INTERNAL_ObjectConstructor(BooleanClass, b);
}

// ---------------------------------------------------------------------------------------------------------------------
// Helper methods
int IndexOfInstruction(BCI_Method bci)
{
    for(size_t i=0; i<BCI_NumberOfInstructions; i++)
    {
        if(BCI_Instructions[i] == bci)
        {
            return i;
        }
    }
    return -1;
}


// ---------------------------------------------------------------------------------------------------------------------
// Instructions

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

    BCI_Cmp,
    BCI_LoadCmp,

    BCI_JumpFalse,
    BCI_Jump,

    BCI_Copy,

    BCI_ResolveDirect,
    BCI_ResolveScoped,

    BCI_DefMethod,
    BCI_Eval,
    BCI_Return,

    BCI_EnterLocal,
    BCI_LeaveLocal,

    BCI_Extend,
    BCI_NOP,
};

// ---------------------------------------------------------------------------------------------------------------------
// Instruction Helpers

inline Reference* TOS_Ref()
{
    auto ref = static_cast<Reference*>(MemoryStack.back());
    MemoryStack.pop_back();
    return ref;
}

inline Reference* TOSpeek_Ref()
{
    return static_cast<Reference*>(MemoryStack.back());
}

inline Object* TOS_Obj()
{
    auto obj = static_cast<Object*>(MemoryStack.back());
    MemoryStack.pop_back();
    return obj;
}

inline Object* TOSpeek_Obj()
{
    return static_cast<Object*>(MemoryStack.back());
}

inline Scope* TOS_Scope()
{
    auto scope = static_cast<Scope*>(MemoryStack.back());
    MemoryStack.pop_back();
    return scope;
}

inline String* TOS_String()
{
    auto str = static_cast<String*>(MemoryStack.back());
    MemoryStack.pop_back();
    return str;
}

inline void TOS_discard()
{
    MemoryStack.pop_back();
}

inline Reference* FindInScopeOnlyImmediate(Scope* scope, String& refName)
{
    for(auto ref: scope->ReferencesIndex)
    {
        if(ref->Name == refName)
            return ref;
    }
    return nullptr;
}

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

inline Scope* ScopeOf(Reference* ref)
{
    return ref->To->Attributes;
}

template <typename T>
inline void PushToStack(T* entity)
{
    MemoryStack.push_back(static_cast<void*>(entity));
}

inline unsigned int MemoryStackSize()
{
    return MemoryStack.size();
}

inline void EnterNewCallFrame(int callerRefId, Object* caller, Object* self)
{
    std::vector<Scope> localScopeStack;
    CallStack.push_back({ InstructionReg+1, MemoryStackSize(), callerRefId, localScopeStack });
    PushToStack<Object>(caller);
    PushToStack<Object>(self);

    CallerReg = caller;
    SelfReg = self;
    LocalScopeReg = self->Attributes; 
}

inline bool BothAre(Object* obj1, Object* obj2, ObjectClass cls)
{
    return obj1->Class == cls && obj2->Class == cls;
}

inline void AddRefToScope(Reference* ref, Scope* scp)
{
    scp->ReferencesIndex.push_back(ref);
}

inline void AddParamsToMethodScope(Object* methodObj, Object* paramsObj)
{
    if(paramsObj == &NothingObject)
    {
        return;
    }

    auto scope = methodObj->Attributes;
    /// TODO: tuples are unimplemented
    if(paramsObj->Class == TupleClass)
    {
        for(auto ref: paramsObj->Attributes->ReferencesIndex)
        {
            AddRefToScope(ref, scope);
        }
    }
    else
    {
        auto ref = ReferenceConstructor(methodObj->Action->ParameterNames[0], paramsObj);
        AddRefToScope(ref, scope);
    }
}

inline bool NthBit(uint8_t data, int n)
{
    return (data & (BitFlag << n)) >> n;
}

inline void FillInRestOfComparisons()
{
    uint8_t geq = NthBit(CmpReg, 2) | NthBit(CmpReg, 4);
    uint8_t leq = NthBit(CmpReg, 2) | NthBit(CmpReg, 3);

    CmpReg = CmpReg | (geq << 6) | (leq << 5);
}

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

// ---------------------------------------------------------------------------------------------------------------------
// Instruction Defintions

/// no assumptions
/// leaves the String refName as TOS
void BCI_LoadRefName(extArg_t arg)
{
    PushToStack<String>(&ReferenceNames[arg]);
}

/// no asumptions
/// leaves the primitive specified by arg as TOS
void BCI_LoadPrimitive(extArg_t arg)
{
    if(arg >= ConstPrimitives.size())
        return; 

    PushToStack<Object>(ConstPrimitives[arg]);
}

/// assumes TOS is a ref
/// leaves object of that references as TOS
void BCI_Dereference(extArg_t arg)
{
    auto ref = TOS_Ref();
    PushToStack<Object>(ref->To);
}

/// assumes TOS is the new object and TOS1 is the reference to reassign
/// leaves the object
void BCI_Assign(extArg_t arg)
{
    auto obj = TOS_Obj();
    auto ref = TOS_Ref();

    ref->To = obj;
    PushToStack<Object>(obj);
}

/// assumes TOS1 and TOS2 are rhs object and lhs object respectively
/// leaves the result as TOS
void BCI_Add(extArg_t arg)
{
    auto rObj = TOS_Obj();
    auto lObj = TOS_Obj();

    Object* obj = nullptr;
    if(BothAre(lObj, rObj, IntegerClass))
    {
        int* ans = new int;
        *ans = GetIntValue(*lObj) + GetIntValue(*rObj);
        obj = INTERNAL_ObjectConstructor(IntegerClass, ans);
        
        obj->Value = ans;
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        double* ans = new double;
        *ans = GetDecimalValue(*lObj) + GetDecimalValue(*rObj);
        obj = INTERNAL_ObjectConstructor(DecimalClass, ans);
        obj->Value = ans;
    }
    else if(BothAre(lObj, rObj, StringClass))
    {
        String* ans = new String;
        *ans = GetStringValue(*lObj) + GetStringValue(*rObj);
        obj = INTERNAL_ObjectConstructor(StringClass, ans);
        obj->Value = ans;
    }
    else
    {
        obj = &NothingObject;
        ReportError(SystemMessageType::Warning, 0, Msg("cannot add types %s and %s", lObj->Class, rObj->Class));
    }

    PushToStack<Object>(obj);
}

void BCI_Subtract(extArg_t arg)
{
    auto rObj = TOS_Obj();
    auto lObj = TOS_Obj();

    Object* obj = nullptr;
    if(BothAre(lObj, rObj, IntegerClass))
    {
        int* ans = new int;
        *ans = GetIntValue(*lObj) - GetIntValue(*rObj);
        obj = INTERNAL_ObjectConstructor(IntegerClass, ans);
        
        obj->Value = ans;
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        double* ans = new double;
        *ans = GetDecimalValue(*lObj) - GetDecimalValue(*rObj);
        obj = INTERNAL_ObjectConstructor(DecimalClass, ans);
        obj->Value = ans;
    }
    else
    {
        obj = &NothingObject;
        ReportError(SystemMessageType::Warning, 0, Msg("cannot subtract types %s and %s", lObj->Class, rObj->Class));
    }

    PushToStack<Object>(obj);
}

void BCI_Multiply(extArg_t arg)
{
    auto rObj = TOS_Obj();
    auto lObj = TOS_Obj();

    Object* obj = nullptr;
    if(BothAre(lObj, rObj, IntegerClass))
    {
        int* ans = new int;
        *ans = GetIntValue(*lObj) * GetIntValue(*rObj);
        obj = INTERNAL_ObjectConstructor(IntegerClass, ans);
        
        obj->Value = ans;
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        double* ans = new double;
        *ans = GetDecimalValue(*lObj) * GetDecimalValue(*rObj);
        obj = INTERNAL_ObjectConstructor(DecimalClass, ans);
        obj->Value = ans;
    }
    else
    {
        obj = &NothingObject;
        ReportError(SystemMessageType::Warning, 0, Msg("cannot multiply types %s and %s", lObj->Class, rObj->Class));
    }

    PushToStack<Object>(obj);
}

void BCI_Divide(extArg_t arg)
{
    auto rObj = TOS_Obj();
    auto lObj = TOS_Obj();

    Object* obj = nullptr;
    if(BothAre(lObj, rObj, IntegerClass))
    {
        int* ans = new int;
        *ans = GetIntValue(*lObj) / GetIntValue(*rObj);
        obj = INTERNAL_ObjectConstructor(IntegerClass, ans);
        
        obj->Value = ans;
    }
    else if(BothAre(lObj, rObj, DecimalClass))
    {
        double* ans = new double;
        *ans = GetDecimalValue(*lObj) / GetDecimalValue(*rObj);
        obj = INTERNAL_ObjectConstructor(DecimalClass, ans);
        obj->Value = ans;
    }
    else
    {
        obj = &NothingObject;
        ReportError(SystemMessageType::Warning, 0, Msg("cannot divide types %s and %s", lObj->Class, rObj->Class));
    }

    PushToStack<Object>(obj);
}

/// 0 = print
void BCI_SysCall(extArg_t arg)
{
    switch(arg)
    {
        case 0:
        std::cout << GetStringValue(*TOSpeek_Obj()) << std::endl;
        break;

        default:
        break;
    }
}

void BCI_And(extArg_t arg)
{
    auto rObj = TOS_Obj();
    auto lObj = TOS_Obj();

    bool* ans = new bool;
    *ans = GetBoolValue(*lObj) && GetBoolValue(*rObj);
    Object* obj = INTERNAL_ObjectConstructor(BooleanClass, ans);

    PushToStack<Object>(obj);
}

void BCI_Or(extArg_t arg)
{
    auto rObj = TOS_Obj();
    auto lObj = TOS_Obj();

    bool* ans = new bool;
    *ans = GetBoolValue(*lObj) || GetBoolValue(*rObj);
    Object* obj = INTERNAL_ObjectConstructor(BooleanClass, ans);

    PushToStack<Object>(obj);
}

void BCI_Not(extArg_t arg)
{
    auto obj = TOS_Obj();

    bool* ans = new bool;
    *ans = !GetBoolValue(*obj);
    Object* newObj = INTERNAL_ObjectConstructor(BooleanClass, ans);

    PushToStack<Object>(newObj);
}

/// assumes TOS1 1 and TOS2 are rhs object and lhs object respectively
/// sets CmpReg to appropriate value
void BCI_Cmp(extArg_t arg)
{
    CmpReg = CmpRegDefaultValue;
    auto rObj = TOS_Obj();
    auto lObj = TOS_Obj();
    

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
        boolObj = INTERNAL_BooleanObjectConstructor(NthBit(CmpReg, arg));
        break;

        default:
        return;
    }

    PushToStack<Object>(boolObj);
}

void BCI_JumpFalse(extArg_t arg)
{
    auto obj = TOS_Obj();
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
    auto obj = TOS_Obj();
    auto objCopy = INTERNAL_ObjectConstructor(obj->Class, obj->Value);
    objCopy->BlockStartInstructionId = obj->BlockStartInstructionId;
    PushToStack<Object>(objCopy);
}

/// assumes TOS is a String
/// leaves resolved reference as TOS
void BCI_ResolveDirect(extArg_t arg)
{
    auto refName = TOS_String();
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
        auto newRef = ReferenceConstructor(*refName, &SomethingObject);
        AddRefToScope(newRef, LocalScopeReg);
        PushToStack<Reference>(newRef);
    }
    else
    {
        PushToStack<Reference>(resolvedRef);
    }

}

/// assumes TOS is a string and TOS1 is a ref
/// leaves resolved reference as TOS
void BCI_ResolveScoped(extArg_t arg)
{
    auto refName = TOS_String();

    if(TOSpeek_Ref() == &NothingReference || TOSpeek_Ref()->To == &SomethingObject)
    {
        ReportError(SystemMessageType::Warning, 1, Msg(" %s refers to the object <%s> which cannot have attributes", TOSpeek_Ref()->Name, TOSpeek_Ref()->To->Class));
        return;
    }
    auto callerRef = TOS_Ref();
    auto resolvedRef = FindInScopeOnlyImmediate(ScopeOf(callerRef), *refName);

    if(resolvedRef == nullptr)
    {
        auto newRef = ReferenceConstructor(*refName, &SomethingObject);
        AddRefToScope(newRef, callerRef->To->Attributes);
        PushToStack<Reference>(newRef);
    }
    else
    {
        PushToStack<Reference>(resolvedRef);
    }
}

/// assumes the top [arg] entities on the stack are parameter names (strings)
/// leaves a new method object as TOS
void BCI_DefMethod(extArg_t arg)
{
    ParameterList list;
    list.reserve(arg+1);

    for(int i = arg; i > 0; i--)
    {
        list[i-1] = *TOS_String(); 
    }

    auto obj = INTERNAL_ObjectConstructor(MethodClass, nullptr);
    obj->ByteCodeParamsAsMethod = list;
    /// expects a block
    /// TODO: verify block
    obj->BlockStartInstructionId = InstructionReg + 1;

    PushToStack<Object>(obj);
}

/// assumes TOS an obj (params), TOS1 is an object (method), TOS2 an object (caller)
/// adds caller and self to TOS (in that order)
void BCI_Eval(extArg_t arg)
{
    auto paramsObj = TOS_Obj();
    auto methodObj = TOS_Obj();
    auto callerObj = TOS_Obj();

    int jumpTo = methodObj->BlockStartInstructionId;
    AddParamsToMethodScope(methodObj, paramsObj);
    /// TODO: figure out caller id
    EnterNewCallFrame(0, callerObj, methodObj);
    InstructionReg = jumpTo;
    
}

void BCI_Return(extArg_t arg)
{
    int jumpBackTo = CallStack.back().ReturnToInstructionId;
    int stackStart = CallStack.back().MemoryStackStart;
    while(MemoryStack.size() >= static_cast<size_t>(stackStart))
    {
        TOS_discard();
    }
    InstructionReg = jumpBackTo;
    CallStack.pop_back();

    LocalScopeStack() = CallStack.back().LocalScopeStack;
    LocalScopeReg = &LocalScopeStack().back();
}

/// no assumptions
/// does not change TOS. will update LocalScopeReg
void BCI_EnterLocal(extArg_t arg)
{
    LocalScopeStack().push_back( {{}, LocalScopeReg, false} );
    LocalScopeReg = &LocalScopeStack().back();
}

void BCI_LeaveLocal(extArg_t arg)
{
    /// TODO: destroy andy local references
    LocalScopeStack().pop_back();
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

void BCI_NOP(extArg_t arg)
{
    // does nothing, used for optimizations
}
