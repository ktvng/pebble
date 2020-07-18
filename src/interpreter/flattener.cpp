#include <iostream>

#include "flattener.h"

#include "object.h"
#include "program.h"
#include "scope.h"
#include "reference.h"
#include "operation.h"
#include "diagnostics.h"


#include "vm.h"
#include "bytecode.h"
#include "errormsg.h"


// ---------------------------------------------------------------------------------------------------------------------
// Helpers

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



// ---------------------------------------------------------------------------------------------------------------------
// Flattening the AST



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


bool IsOperationComparision(Operation* op)
{
    auto opType = op->Type;
    return opType == OperationType::IsGreaterThan
        || opType == OperationType::IsLessThan
        || opType == OperationType::IsEqual
        || opType == OperationType::IsGreaterThanOrEqualTo
        || opType == OperationType::IsLessThanOrEqualTo;
}

/// bit at position [i] indicates:
/// 0: hardcoded false
/// 1: hardcoded true
/// 2: ==
/// 3: <
/// 4: >
/// 5: <=
/// 6: >=
/// see vm.cpp for official list
inline void FlattenOperationComparison(Operation* op)
{
    for(auto operand: op->Operands)
    {
        FlattenOperation(operand);
    }

    uint8_t opId;
    extArg_t arg;

    opId = IndexOfInstruction(BCI_Cmp);
    AddByteCodeInstruction(opId, noArg);

    opId = IndexOfInstruction(BCI_LoadCmp);
    switch(op->Type)
    {
        case OperationType::IsEqual:
        arg = 2;
        break;

        case OperationType::IsLessThan:
        arg = 3;
        break;

        case OperationType::IsGreaterThan:
        arg = 4;
        break;

        case OperationType::IsLessThanOrEqualTo:
        arg = 5;
        break;

        case OperationType::IsGreaterThanOrEqualTo:
        arg = 6;
        break;

        default:
        return;
    }
    
    AddByteCodeInstruction(opId, arg);
}

/// compatible with:
/// add/subtract/multiply/divide
/// and/or/not
/// syscall
inline void FlattenOperationGeneric(Operation* op)
{
    for(auto operand: op->Operands)
    {
        FlattenOperation(operand);
    }

    uint8_t opId;
    extArg_t arg = noArg;
    switch(op->Type)
    {
        case OperationType::Add:
        opId = IndexOfInstruction(BCI_Add);
        break; 

        case OperationType::Subtract:
        opId = IndexOfInstruction(BCI_Subtract);
        break; 

        case OperationType::Multiply:
        opId = IndexOfInstruction(BCI_Multiply);
        break; 

        case OperationType::Divide:
        opId = IndexOfInstruction(BCI_Divide);
        break; 

        case OperationType::And:
        opId = IndexOfInstruction(BCI_And);
        break;

        case OperationType::Or:
        opId = IndexOfInstruction(BCI_Or);
        break;

        case OperationType::Not:
        opId = IndexOfInstruction(BCI_Not);
        break;

        case OperationType::Print:
        opId = IndexOfInstruction(BCI_SysCall);
        arg = 0;
        break;
        
        default:
        LogIt(LogSeverityType::Sev3_Critical, "FlattenOperationGeneric", "unknown or unimplemented");
        return;
    }
    AddByteCodeInstruction(opId, arg);
}

inline void FlattendOperationAssign(Operation* op)
{
    FlattenOperationScopeResolution(op->Operands[0]);
    FlattenOperation(op->Operands[1]);
    uint8_t opId = IndexOfInstruction(BCI_Assign);
    AddByteCodeInstruction(opId, noArg);
}

inline void FlattenOperationNew(Operation* op)
{
    bool isRef;
    FlattenOperationRefDirect(op->Operands[0], isRef);
    IfNecessaryAddDereference(isRef);
    uint8_t opId = IndexOfInstruction(BCI_Copy);
    AddByteCodeInstruction(opId, noArg);
}

void FlattenOperation(Operation* op)
{
    if(op->Type == OperationType::ScopeResolution)
    {
        FlattenOperationScopeResolutionWithDereference(op);
    }
    else if(op->Type == OperationType::Assign)
    {
        FlattendOperationAssign(op);
    }
    else if(op->Type == OperationType::New)
    {
        FlattenOperationNew(op);
    }
    else if(IsOperationComparision(op))
    {
        FlattenOperationComparison(op);
    }
    else
    {
        FlattenOperationGeneric(op);
    }
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
    else if(blockOwner->Type == OperationType::DefineMethod)
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
            ByteCodeLineAssociation.push_back(NextInstructionId());
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
    
    ByteCodeProgram.clear();
    ByteCodeLineAssociation.clear();
    ByteCodeLineAssociation.push_back(0);
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
