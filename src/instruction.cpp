#include <cstdint>

#include "instruction.h"
#include "object.h"
#include "main.h"
#include "program.h"
#include "scope.h"
#include "reference.h"
#include "operation.h"
#include "cctype" //remove
#include "diagnostics.h"


typedef unsigned long long extArg_t;

// ---------------------------------------------------------------------------------------------------------------------
// Scope Registers

/// stores the program global scope
Scope* ProgramReg = nullptr;

/// stores the caller scope
Object* CallerReg = nullptr;

/// stores the topmost callframe scope
Object* SelfReg = nullptr;

/// stores the local (possibly anonymous) scope
/// in general this is equivalent to SelfReg
Scope* LocalScopeReg = nullptr;


// ---------------------------------------------------------------------------------------------------------------------
// Special Registers
String* ReferenceNameReg;

extArg_t ExtendedArg;

int ExtensionExp;

extArg_t InstructionReg;

/// 1 if jump occured, 0 otherwise
int JumpStatusReg;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

std::vector<void*> MemoryStack;


// ---------------------------------------------------------------------------------------------------------------------
// Entity Arrays
std::vector<String> ReferenceNames;
std::vector<Object*> ConstPrimitives;


typedef void (*BCISystemCall)();
std::vector<BCISystemCall> SystemFunctions;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack
struct CallFrame
{
    extArg_t ReturnToInstructionId;
    extArg_t MemoryStackStart;
    int Owner;
    std::vector<Scope> LocalScopeStack;
};

std::vector<CallFrame> CallStack;

// ---------------------------------------------------------------------------------------------------------------------
// Anonymous scope stack
// used for scopes that don't have owners (if/while/indent-block) which don't affect
// self or caller
inline std::vector<Scope>& LocalScopeStack()
{
    return CallStack.back().LocalScopeStack;
}

// ---------------------------------------------------------------------------------------------------------------------
// Bytecode program
struct ByteCodeInstruction
{
    uint8_t Op;
    uint8_t Arg;
};

std::vector<ByteCodeInstruction> ByteCodeProgram;


String ToString(ByteCodeInstruction& bci);

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

// ---------------------------------------------------------------------------------------------------------------------
// Statics
Object GodObject
{
    BaseClass,
    ScopeConstructor(nullptr),
    nullptr,
    nullptr,
    nullptr
};


Object NothingObject
{
    NullClass,
    ScopeConstructor(nullptr),
    nullptr,
    nullptr,
    nullptr
};

Scope NothingScope = *NothingObject.Attributes;

Reference NothingReference
{
    "Nothing",
    &NothingObject,
};

Object SomethingObject
{
    SomethingClass,
    ScopeConstructor(nullptr),
    nullptr,
    nullptr,
    nullptr
};

Scope SomethingScope = *SomethingObject.Attributes;



// ---------------------------------------------------------------------------------------------------------------------
// Instructions

typedef void (*BCI_Method)(extArg_t);

const inline int BCI_NumberOfInstructions = 23;

void BCI_LoadRefName(extArg_t arg);
void BCI_LoadPrimitive(extArg_t arg);
void BCI_Dereference(extArg_t arg);

void BCI_Assign(extArg_t arg);

void BCI_Add(extArg_t arg);
void BCI_Subtract(extArg_t arg);
void BCI_Multiply(extArg_t arg);
void BCI_Divide(extArg_t arg);

void BCI_SysCall(extArg_t arg);

void BCI_And(extArg_t arg);
void BCI_Or(extArg_t arg);
void BCI_Not(extArg_t arg);

void BCI_JumpFalse(extArg_t arg);
void BCI_Jump(extArg_t arg);

void BCI_Copy(extArg_t arg);

void BCI_ResolveDirect(extArg_t arg);
void BCI_ResolveScoped(extArg_t arg);

void BCI_Eval(extArg_t arg);
void BCI_Return(extArg_t arg);

void BCI_EnterLocal(extArg_t arg);
void BCI_LeaveLocal(extArg_t arg);

void BCI_Extend(extArg_t arg);
void BCI_NOP(extArg_t arg);

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

    BCI_JumpFalse,
    BCI_Jump,

    BCI_Copy,

    BCI_ResolveDirect,
    BCI_ResolveScoped,

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

inline void TOS_discard()
{
    MemoryStack.pop_back();
}

inline Reference* FindInScopeOnlyImmediate(Scope* scope, String refName)
{
    for(auto ref: scope->ReferencesIndex)
    {
        if(ref->Name == refName)
            return ref;
    }
    return nullptr;
}

inline Reference* FindInScopeChain(Scope* scope, String refName)
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


// ---------------------------------------------------------------------------------------------------------------------
// Instruction Defintions

/// no assumptions
/// leaves ReferenceNameReg containing the name 
void BCI_LoadRefName(extArg_t arg)
{
    ReferenceNameReg = &ReferenceNames[arg];
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
    }

    PushToStack<Object>(obj);
}

/// TODO:
void BCI_Multiply(extArg_t arg){}
void BCI_Divide(extArg_t arg){}

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

void BCI_And(extArg_t arg){}
void BCI_Or(extArg_t arg){}
void BCI_Not(extArg_t arg){}

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

/// assumes ReferenceNameRef is valued
/// leaves resolved reference as TOS
void BCI_ResolveDirect(extArg_t arg)
{
    auto resolvedRef = FindInScopeOnlyImmediate(LocalScopeReg, *ReferenceNameReg);

    if(resolvedRef == nullptr)
    {
        resolvedRef = FindInScopeChain(SelfReg->Attributes, *ReferenceNameReg);
    }

    if(resolvedRef == nullptr)
    {
        resolvedRef = FindInScopeOnlyImmediate(CallerReg->Attributes, *ReferenceNameReg);
    }

    if(resolvedRef == nullptr)
    {
        resolvedRef = FindInScopeOnlyImmediate(ProgramReg, *ReferenceNameReg);
    }

    if(resolvedRef == nullptr)
    {
        auto newRef = ReferenceConstructor(*ReferenceNameReg, &SomethingObject);
        AddRefToScope(newRef, LocalScopeReg);
        PushToStack<Reference>(newRef);
    }
    else
    {
        PushToStack<Reference>(resolvedRef);
    }

}

/// assumes TOS is a ref and ReferenceNameReg is valued
/// leaves resolved reference as TOS
void BCI_ResolveScoped(extArg_t arg)
{
    /// TODO: Catch errors for accessing Nothing and Something
    if(TOSpeek_Ref() == &NothingReference || TOSpeek_Ref()->To == &SomethingObject)
        return;
    auto callerRef = TOS_Ref();
    auto resolvedRef = FindInScopeOnlyImmediate(ScopeOf(callerRef), *ReferenceNameReg);

    if(resolvedRef == nullptr)
    {
        auto newRef = ReferenceConstructor(*ReferenceNameReg, &SomethingObject);
        AddRefToScope(newRef, callerRef->To->Attributes);
        PushToStack<Reference>(newRef);
    }
    else
    {
        PushToStack<Reference>(resolvedRef);
    }
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
    LocalScopeReg = &LocalScopeStack().back();
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

// ---------------------------------------------------------------------------------------------------------------------
// Flattening the AST, Helpers
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
// Program Execution


void InitRuntime()
{
    std::vector<Scope> localScopeStack;
    unsigned long programEnd = ByteCodeProgram.size();
    
    /// TODO: update arg 3 (caller ID);
    CallStack.push_back( {programEnd, 0, 0, localScopeStack });

    CallerReg = &NothingObject;
    SelfReg = &NothingObject;

    ProgramReg = ScopeConstructor(nullptr);
    LocalScopeReg = ProgramReg;
}

/// true if ins is not Extend and 
inline bool ShouldUseExtendedArg(const ByteCodeInstruction& ins)
{
    return ins.Op != IndexOfInstruction(BCI_Extend) && ExtensionExp;
}

inline bool IsNOP(const ByteCodeInstruction& ins)
{
    return ins.Op == IndexOfInstruction(BCI_NOP);
}

void DoByteCodeProgram()
{
    InitRuntime();

    while(InstructionReg < ByteCodeProgram.size())
    {
        auto ins = ByteCodeProgram[InstructionReg];
        LogItDebug(ToString(ins), "DoByteCodeProgram");

        if(IsNOP(ins))
        {
            InstructionReg++;
            continue;
        }

        if(ShouldUseExtendedArg(ins))
        {
            BCI_Instructions[ins.Op](ExtendedArg ^ ins.Arg);
            ExtendedArg = 0;
            ExtensionExp = 0;
        }
        else
        {
            BCI_Instructions[ins.Op](ins.Arg);
        }

        if(!JumpStatusReg)
        {
            InstructionReg++;
        }
        else
        {
            JumpStatusReg = 0;
        }
    }
}


// ---------------------------------------------------------------------------------------------------------------------
// Flattening the AST

uint8_t noArg = 0;

inline const extArg_t ByteFlag = 0xff;

inline uint8_t ReduceLongArgByExp(extArg_t& arg, int extendExp)
{
    extArg_t extendArg = arg & (ByteFlag << (extendExp * 8));
    uint8_t scaledExtendArg = extendArg >> (extendExp * 8);
    arg = (~(ByteFlag << (extendExp * 8))) & arg; 
    return scaledExtendArg;
}

inline uint8_t IfNecessaryAddExtendInstructionAndReduce(extArg_t& arg)
{
    int extendExp = 1;
    while(arg > 255 && extendExp < 8)
    {
        uint8_t opId = IndexOfInstruction(BCI_Extend);
        uint8_t scaledExtendArg = ReduceLongArgByExp(arg, extendExp);
        ByteCodeProgram.push_back( {opId, scaledExtendArg} );
    }
    arg = arg & ByteFlag;

    return arg;
}

inline void AddByteCodeInstruction(uint8_t opId, extArg_t arg)
{
    uint8_t reducedArg = IfNecessaryAddExtendInstructionAndReduce(arg);
    ByteCodeProgram.push_back( {opId, reducedArg} );
}


inline uint8_t IfNecessaryRewriteExtendInstructionAndReduce(extArg_t& arg, extArg_t& atPos)
{
    int extendExp = 1;
    while(arg > 255 && extendExp < 8)
    {
        uint8_t opId = IndexOfInstruction(BCI_Extend);
        uint8_t scaledExtendArg = ReduceLongArgByExp(arg, extendExp);
        ByteCodeProgram[atPos++] = {opId, scaledExtendArg};
    }
    arg = arg & ByteFlag;

    return arg;
}

inline void RewriteByteCodeInstruction(uint8_t opId, extArg_t arg, extArg_t atPos)
{
    uint8_t reducedArg = IfNecessaryRewriteExtendInstructionAndReduce(arg, atPos);
    ByteCodeProgram[atPos] = {opId, reducedArg};
}

inline extArg_t CurrentInstructionId()
{
    return ByteCodeProgram.size()-1;
}

inline extArg_t NextInstructionId()
{
    return ByteCodeProgram.size();
}

int CurrentInstructionMagnitude()
{
    extArg_t ip = CurrentInstructionId();
    int magnitude = 0;
    for(ip = ip >> 8; ip; ip = ip >> 8)
    {
        magnitude++;   
    }

    return magnitude;
}

void AddNOPS(int i)
{
    for(;i; i-=1)
    {
        uint8_t opId = IndexOfInstruction(BCI_NOP);
        AddByteCodeInstruction(opId, noArg);
    }
}



void FlattenOperation(Operation* op);


/// true if the reference operation points to a primitive or instance of Object/Something
bool OperationRefIsPrimitive(Operation* op)
{
    auto ref = op->Value;
    if(IsReferenceStub(ref))
    {
        return ref->Name == "Object" || ref->Name == "Something";
    }

    return true;
}

void FlattenOperationRefDirect(Operation* op, bool& isRef)
{
    uint8_t opId;
    extArg_t arg = op->EntityIndex;

    if(OperationRefIsPrimitive(op))
    {
        opId = IndexOfInstruction(BCI_LoadPrimitive);
        isRef = false;
    }
    else
    {
        opId = IndexOfInstruction(BCI_LoadRefName);
        isRef = true;
    }
    AddByteCodeInstruction(opId, arg);
}

/// flattens operation assuming it is not a primitive
void FlattenOperationRefScoped(Operation* op, bool& isRef)
{
    uint8_t opId = IndexOfInstruction(BCI_LoadRefName);
    extArg_t arg = op->EntityIndex;

    isRef = true;

    AddByteCodeInstruction(opId, arg);
}

inline void IfNecessaryAddDereference(bool shouldDereference)
{
    if(shouldDereference)
    {
        uint8_t opId = IndexOfInstruction(BCI_Dereference);
        AddByteCodeInstruction(opId, noArg);
    }
}

void FlattenOperationScopeResolutionDirect(Operation* op, bool shouldDereference)
{
    auto firstOperand = op->Operands[0];
    bool isRef = false;
    FlattenOperationRefDirect(firstOperand, isRef);
    
    // primitives (isRef == false) are loaded directly and do not need to be
    // resolved
    if(isRef)
    {
        uint8_t opId = IndexOfInstruction(BCI_ResolveDirect);
        AddByteCodeInstruction(opId, noArg);
        IfNecessaryAddDereference(shouldDereference);
    }
}

void FlattenOperationScopeResolution(Operation* op);

void FlattenOperationScopeResolutionScoped(Operation* op, bool shouldDereference)
{
    auto firstOperand = op->Operands[0];
    FlattenOperationScopeResolution(firstOperand);

    auto secondOperand = op->Operands[1];
    bool isRef = false;
    FlattenOperationRefScoped(secondOperand, isRef);

    if(isRef)
    {
        uint8_t opId = IndexOfInstruction(BCI_ResolveScoped);
        AddByteCodeInstruction(opId, noArg);
        IfNecessaryAddDereference(shouldDereference);
    }
}

/// TODO: might (won't) work with scoped pritmitives (ie X.4)
void FlattenOperationScopeResolution(Operation* op)
{
    if(op->Operands.size() == 1)
    {
        FlattenOperationScopeResolutionDirect(op, false);
    }
    else
    {
        // must be case of scoped resolution
        FlattenOperationScopeResolutionScoped(op, false);
    }
}

void FlattenOperationScopeResolutionWithDereference(Operation* op)
{
    if(op->Operands.size() == 1)
    {
        FlattenOperationScopeResolutionDirect(op, true);
    }
    else
    {
        // must be case of scoped resolution
        FlattenOperationScopeResolutionScoped(op, true);
    }
}


void FlattenOperation(Operation* op)
{
    /// assign operation handled in a different case
    if(op->Type == OperationType::ScopeResolution)
    {
        FlattenOperationScopeResolutionWithDereference(op);
        return;
    }
    else if(op->Type == OperationType::Assign)
    {
        FlattenOperationScopeResolution(op->Operands[0]);
        FlattenOperation(op->Operands[1]);
    }
    else if(op->Type == OperationType::New)
    {
        bool isRef;
        FlattenOperationRefDirect(op->Operands[0], isRef);
        IfNecessaryAddDereference(isRef);
    }
    else
    {
        for(auto operand: op->Operands)
        {
            FlattenOperation(operand);
        }
    }


    uint8_t opId;
    extArg_t arg = noArg;
    switch(op->Type)
    {
        case OperationType::Assign:
        opId = IndexOfInstruction(BCI_Assign);
        break;

        case OperationType::Print:
        opId = IndexOfInstruction(BCI_SysCall);
        arg = 0;
        break;

        case OperationType::Add:
        opId = IndexOfInstruction(BCI_Add);
        break; 

        case OperationType::New:
        opId = IndexOfInstruction(BCI_Copy);
        break;

        case OperationType::Subtract:
        opId = IndexOfInstruction(BCI_Subtract);
        break;
        
        default:
        return;
    }
    AddByteCodeInstruction(opId, arg);
}


void HandleFlatteningControlFlow(Block* block, Operation* blockOwner, unsigned long blockOwnerInstructionStart)
{
    uint8_t opId;
    extArg_t arg = noArg;
    if(blockOwner == nullptr)
    {
        opId = IndexOfInstruction(BCI_EnterLocal);
        AddByteCodeInstruction(opId, arg);

        FlattenBlock(block);
        
        opId = IndexOfInstruction(BCI_LeaveLocal);
        AddByteCodeInstruction(opId, arg);
    }
    else if(blockOwner->Type == OperationType::While)
    {

        arg = noArg; // need to get arg by expanding block
        
        extArg_t JumpInstructionStart = NextInstructionId();
        int ipMagnitude = CurrentInstructionMagnitude();
        AddNOPS(ipMagnitude + 2);

        FlattenBlock(block);
        
        opId = IndexOfInstruction(BCI_Jump);
        arg = blockOwnerInstructionStart;
        AddByteCodeInstruction(opId, arg);
        
        opId = IndexOfInstruction(BCI_JumpFalse);
        arg = NextInstructionId();
        RewriteByteCodeInstruction(opId, arg, JumpInstructionStart);
    }
    else if(blockOwner->Type == OperationType::If)
    {

    }
    else
    {
        opId = IndexOfInstruction(BCI_EnterLocal);
        AddByteCodeInstruction(opId, arg);

        FlattenBlock(block);

        opId = IndexOfInstruction(BCI_LeaveLocal);
        AddByteCodeInstruction(opId, arg);
    }
}

/// assumes that all ifs/whiles/methods have blocks
void FlattenBlock(Block* block)
{
    Operation* blockOwner = nullptr;
    unsigned long blockOwnerInstructionStart = 0;
    for(auto exec: block->Executables)
    {
        switch(exec->ExecType)
        {
            case ExecutableType::Block:
            HandleFlatteningControlFlow(static_cast<Block*>(exec), blockOwner, blockOwnerInstructionStart);
            break;

            case ExecutableType::Operation:
            blockOwnerInstructionStart = NextInstructionId();
            FlattenOperation(static_cast<Operation*>(exec));
            blockOwner = static_cast<Operation*>(exec);
            break;
        }
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// First Pass

bool ReferenceNamesContains(String refName, size_t& atPosition)
{
    for(size_t i=0; i<ReferenceNames.size(); i++)
    {
        auto name = ReferenceNames[i];
        if(name == refName)
        {
            atPosition = i;
            return true;
        }
    }
    return false;
}

/// assumes op is a ref operation
void IfNeededAddReferenceName(Operation* op)
{
    auto refName = op->Value->Name;

    size_t atPosition;
    if(ReferenceNamesContains(refName, atPosition))
    {
        op->EntityIndex = atPosition;
        return;
    }

    op->EntityIndex = ReferenceNames.size();
    ReferenceNames.push_back(refName);
}

bool ConstPrimitivesContains(Object* obj, size_t& atPosition)
{
    // GodObject and Something object always 0 and 1
    if(obj == &GodObject)
    {
        atPosition = 0;
        return true;
    }
    else if(obj == &SomethingObject)
    {
        atPosition = 1;
        return true;
    }


    for(size_t i=0; i<ConstPrimitives.size(); i++)
    {
        auto constPrimObj = ConstPrimitives[i];
        if(obj == constPrimObj)
        {
            atPosition = i;
            return true;
        }
    }
    return false;
}

void IfNeededAddConstPrimitive(Operation* op)
{
    auto obj = op->Value->To;
    
    /// TODO: streamline this process for GodObj and SomethingObj
    if(op->Value->Name == "Object")
    {
        op->EntityIndex = 0;
        return;
    }
    else if(op->Value->Name == "Something")
    {
        op->EntityIndex = 1;
        return;
    }

    size_t atPosition;
    if(ConstPrimitivesContains(obj, atPosition))
    {
        op->EntityIndex = atPosition;
        return;
    }

    op->EntityIndex = ConstPrimitives.size();
    ConstPrimitives.push_back(obj);
}

void FirstPassOperation(Operation* op)
{
    if(op->Type == OperationType::Ref)
    {
        if(OperationRefIsPrimitive(op))
        {
            IfNeededAddConstPrimitive(op);
        }
        else
        {
            // this is the case that it is a primitive
            IfNeededAddReferenceName(op);
        }
        return;
    }

    for(auto operand: op->Operands)
    {
        FirstPassOperation(operand);
    }
}

void FirstPassBlock(Block* b)
{
    for(auto exec: b->Executables)
    {
        switch(exec->ExecType)
        {
            case ExecutableType::Block:
            FirstPassBlock(static_cast<Block*>(exec));
            break;

            case ExecutableType::Operation:
            FirstPassOperation(static_cast<Operation*>(exec));
            break;
        }
    }
}

void InitEntityLists()
{
    ConstPrimitives.clear();
    ConstPrimitives = { &GodObject, &SomethingObject };

    ReferenceNames.clear();
}


void FirstPassProgram(Program* p)
{
    FirstPassBlock(p->Main);
}

void FlattenProgram(Program* p)
{
    InitEntityLists();
    FirstPassProgram(p);
    FlattenBlock(p->Main);
}








// ---------------------------------------------------------------------------------------------------------------------
// Diagnostics

String ToString(ByteCodeInstruction& ins)
{
    String str;
    if(ins.Op == IndexOfInstruction(BCI_LoadRefName))
    {
        str += "#BCI_LoadRefName";
    }
    else if(ins.Op == IndexOfInstruction(BCI_LoadPrimitive))
    {
        str += "#BCI_LoadPrimitive";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Dereference))
    {
        str += "#BCI_Dereference";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Assign)) 
    {
        str += "#BCI_Assign";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Add))
    {
        str += "#BCI_Add";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Subtract))
    {
        str += "#BCI_Subtract";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Multiply))
    {
        str += "#BCI_Multiply";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Divide))
    {
        str += "#BCI_Divide";
    }
    else if(ins.Op == IndexOfInstruction(BCI_SysCall))
    {
        str += "#BCI_SysCall";
    }
    else if(ins.Op == IndexOfInstruction(BCI_And))
    {
        str += "#BCI_And";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Or))
    {
        str += "#BCI_Or";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Not))
    {
        str += "#BCI_Not";
    }
    else if(ins.Op == IndexOfInstruction(BCI_JumpFalse))
    {
        str += "#BCI_JumpFalse";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Jump))
    {
        str += "#BCI_Jump";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Copy))
    {
        str += "#BCI_Copy";
    }
    else if(ins.Op == IndexOfInstruction(BCI_ResolveDirect))
    {
        str += "#BCI_ResolveDirect";
    }
    else if(ins.Op == IndexOfInstruction(BCI_ResolveScoped))
    {
        str += "#BCI_ResolveScoped";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Eval))
    {
        str += "#BCI_Eval";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Return))
    {
        str += "#BCI_Return";
    }
    else if(ins.Op == IndexOfInstruction(BCI_EnterLocal))
    {
        str += "#BCI_EnterLocal";
    }
    else if(ins.Op == IndexOfInstruction(BCI_LeaveLocal))
    {
        str += "#BCI_LeaveLocal";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Extend))
    {
        str += "#BCI_Extend";
    }
    else if(ins.Op == IndexOfInstruction(BCI_NOP))
    {
        str += "#BCI_NOP";
    }
    else
    {
        str += "#????????????";
    }
    str += "\t " + std::to_string(ins.Arg) + "\n";
    return str;
}

String ToString(std::vector<ByteCodeInstruction>& bciProgram)
{
    String str = "";
    int i=0;
    for(auto& ins: bciProgram)
    {
        str += std::to_string(i++) + ToString(ins);
    }

    return str;
}

void PrintProgramInstructions()
{
    std::cout << "References\n";
    for(size_t i=0; i<ReferenceNames.size(); i++)
    {
        std::cout << i << ": " << ReferenceNames[i] << std::endl;
    }
    std::cout << ToString(ByteCodeProgram);
}