#include <iostream>

#include "flattener.h"

#include "object.h"
#include "program.h"
#include "scope.h"
#include "reference.h"
#include "operation.h"
#include "diagnostics.h"
#include "executable.h"
#include "call.h"


#include "vm.h"
#include "bytecode.h"
#include "errormsg.h"
#include "dis.h"

// ---------------------------------------------------------------------------------------------------------------------
// TODO
// 1. Add checking for reference assignment (should throw error) ie. 4 =5
// 2. Allow scoped references to exist even for primitives, ie. A.5 ??? maybe

static Operation* GlobalBlockOwner;


// ---------------------------------------------------------------------------------------------------------------------
// Helpers

/// constant used in instructions which do not use the argument
constexpr extArg_t noArg = 0;

/// constant used to mask a single byte
constexpr extArg_t ByteMask = 0xff;

/// zeros the byte with [byteNumber] in arg, modifying it, and returns the byte
inline uint8_t RemoveByteFromLongArg(extArg_t& arg, int byteNumber)
{
    extArg_t extendArg = arg & (ByteMask << (byteNumber * 8));
    uint8_t scaledExtendArg = extendArg >> (byteNumber * 8);
    arg = (~(ByteMask << (byteNumber * 8))) & arg; 
    return scaledExtendArg;
}

/// adds the necessary extend operation to represent the long form of [arg] by bytes and 
/// returns the least significant byte
inline uint8_t IfNecessaryAddExtendInstructionAndReduce(extArg_t arg)
{
    int extendExp = 1;
    while(arg > 255 && extendExp < 8)
    {
        uint8_t opId = IndexOfInstruction(BCI_Extend);
        uint8_t scaledExtendArg = RemoveByteFromLongArg(arg, extendExp);
        ByteCodeProgram.push_back( {opId, scaledExtendArg} );
        extendExp++;
    }
    arg = arg & ByteMask;

    return arg;
}

/// add the instruction for [opId] with [arg], breaking [arg] into individual bytes
/// if it is too long
inline void AddByteCodeInstruction(uint8_t opId, extArg_t arg)
{
    uint8_t reducedArg = IfNecessaryAddExtendInstructionAndReduce(arg);
    ByteCodeProgram.push_back( {opId, reducedArg} );
}

/// rewrites extend instructions starting [atPos] to represent the long for mof [arg]
/// and returns the least significant byte
inline uint8_t IfNecessaryRewriteExtendInstructionAndReduce(extArg_t arg, extArg_t& atPos)
{
    int extendExp = 1;
    while(arg > 255 && extendExp < 8)
    {
        uint8_t opId = IndexOfInstruction(BCI_Extend);
        uint8_t scaledExtendArg = RemoveByteFromLongArg(arg, extendExp);
        ByteCodeProgram[atPos++] = {opId, scaledExtendArg};
        extendExp++;
    }
    arg = arg & ByteMask;

    return arg;
}

/// rewrite a byte code instuction [opId] starting [atPOs], ensuring that the long form of [arg] is broken into
/// individual bits
inline void RewriteByteCodeInstruction(uint8_t opId, extArg_t arg, extArg_t atPos)
{
    uint8_t reducedArg = IfNecessaryRewriteExtendInstructionAndReduce(arg, atPos);
    ByteCodeProgram[atPos] = {opId, reducedArg};
}

/// returns the current instruction id
inline extArg_t CurrentInstructionId()
{
    return ByteCodeProgram.size()-1;
}

/// returns the next instruction id
inline extArg_t NextInstructionId()
{
    return ByteCodeProgram.size();
}

/// returns the number of bytes needed to represent the current instruction id
inline int CurrentInstructionMagnitude()
{
    extArg_t ip = CurrentInstructionId();
    int magnitude = 0;
    for(ip = ip >> 8; ip; ip = ip >> 8)
    {
        magnitude++;   
    }

    return magnitude;
}

/// adds [i] NOP instructions
inline void AddNOPS(int i)
{
    for(;i; i-=1)
    {
        uint8_t opId = IndexOfInstruction(BCI_NOP);
        AddByteCodeInstruction(opId, noArg);
    }
}

/// add an instruction marking the end of a line of code
inline void AddEndLineInstruction()
{
    AddByteCodeInstruction(IndexOfInstruction(BCI_EndLine), noArg);
}

/// add instructions for listing params when defining a method and sets 
/// arg to the number of params the method takes
inline void AddInstructionsForDefMethodParameters(Operation* op, extArg_t& arg)
{
    uint8_t opId;
    arg = 0;
    
    opId = IndexOfInstruction(BCI_EnterLocal);
    AddByteCodeInstruction(opId, 1);

    /// assumes the tuple contains one argumented scope resolutions
    if(op->Type == OperationType::Tuple)
    {
        for(auto operand: op->Operands)
        {
            FlattenOperation(operand);
            arg++;

            opId = IndexOfInstruction(BCI_EndLine);
            AddByteCodeInstruction(opId, noArg);
        }
    }
    else
    {
        FlattenOperation(op);
        arg = 1;

        opId = IndexOfInstruction(BCI_EndLine);
        AddByteCodeInstruction(opId, noArg);
    }

    opId = IndexOfInstruction(BCI_LeaveLocal);
    AddByteCodeInstruction(opId, noArg);
}

/// true if [op] is an expression (not of type OperationType::Ref)
inline bool OperationIsExpression(Operation* op)
{
    return op->Type != OperationType::Ref;
}

/// add instructions for listing params when evaluating a method and sets
/// arg to the number of params the method takes
inline void AddInstructionsForEvaluateParameters(Operation* evalOp, extArg_t& arg)
{
    if(evalOp->Operands.size() < 3)
    {
        arg = 0;
        return;
    }

    auto op = evalOp->Operands[2];

    if(op->Type == OperationType::Tuple)
    {
        for(auto operand: op->Operands)
        {
            FlattenOperation(operand);
            arg++;
        }
    }
    else
    {
        if(OperationIsExpression(op))
        {
            FlattenOperation(op);
        }
        else
        {
            FlattenOperationRefDirect(op);
        }
        arg = 1;
    }
}

/// returns the number of NOPs to used in unresolved jumps to be reasonably sure that no bytecode
/// will be overwritten when adding extends
int NOPSafetyDomainSize()
{
    return 2 + CurrentInstructionMagnitude();
}


// ---------------------------------------------------------------------------------------------------------------------
// Jump Context
// A jump context is used to flatten if-else-blocks into bytecode. Unresolved jumps
// are used to store the jumps that occur at the end of an if/else-if block which
// must unconditionally jump to the end of the if-complex (the complete if/else-if/else
// statement). Unresolved jump falses are the if/else-if conditions which will jump to
// the next if-header (if/else-if/else) if evaluated as false.

/// adds a segment of bytecode which will later be resolved as a unconditional jump
inline void AddInstructionsForUnresolvedJump(JumpContext& ctx)
{
    ctx.UnresolvedJumps.push_back(NextInstructionId());
    AddNOPS(NOPSafetyDomainSize());
}

/// adds a segment of bytecode which will later be resolved as a conditional jump
inline void AddInstructionsForUnresolvedJumpFalse(JumpContext& ctx)
{
    ctx.UnresolvedJumpFalse = NextInstructionId();
    ctx.HasUnresolvedJumpFalse = true;
    AddNOPS(NOPSafetyDomainSize());
}

/// initialize a new JumpContext
inline void InitJumpContext(JumpContext& ctx)
{
    ctx.UnresolvedJumps.reserve(16);
    ctx.HasUnresolvedJumpFalse = false;
}

/// reset an existing JumpContext
inline void ClearJumpContext(JumpContext& ctx)
{
    ctx.HasUnresolvedJumpFalse = false;
    ctx.UnresolvedJumps.clear();
}

/// resolves all unresolved jumps (conditional/unconditional) remaining in a JumpContext
/// to jump to the NextInstructionId
inline void ResolveJumpContext(JumpContext& ctx)
{
    uint8_t opId = IndexOfInstruction(BCI_Jump);
    extArg_t jumpTo = NextInstructionId();
    for(extArg_t pos: ctx.UnresolvedJumps)
    {
        RewriteByteCodeInstruction(opId, jumpTo, pos);  
    }

    opId = IndexOfInstruction(BCI_JumpFalse);
    if(ctx.HasUnresolvedJumpFalse)
    {
        RewriteByteCodeInstruction(opId, jumpTo, ctx.UnresolvedJumpFalse);
    }

    ClearJumpContext(ctx);
}

/// resolves the latest conditional jump to the NextInstructionId
inline void ResolveJumpContextClause(JumpContext& ctx)
{
    uint8_t opId = IndexOfInstruction(BCI_JumpFalse);
    extArg_t jumpTo = NextInstructionId();

    RewriteByteCodeInstruction(opId, jumpTo, ctx.UnresolvedJumpFalse);
    ctx.HasUnresolvedJumpFalse = false;
}

/// true if JumpContext has an unresolved conditional jump clause
inline bool JumpContextHasUnresolvedClause(const JumpContext& ctx)
{
    return ctx.HasUnresolvedJumpFalse;
}

/// true if JumpContext has any unresolved jump, either conditional or unconditional
inline bool JumpContextNeedsResolution(const JumpContext& ctx)
{
    return ctx.UnresolvedJumps.size() > 0 || ctx.HasUnresolvedJumpFalse;
}

/// true if [op] includes a conditional jump in its bytecode
inline bool IsConditionalJump(Operation* op)
{
    return op->Type == OperationType::While || op->Type == OperationType::If || op->Type == OperationType::ElseIf;
}

/// true if [op] is part of an if-complex (if/else-if/else)
inline bool IsPartOfIfComplex(Operation* op)
{
    if(op == nullptr)
    {
        return false;
    }

    return op->Type == OperationType::Else || op->Type == OperationType::ElseIf || op->Type == OperationType::If;
}

inline bool IsPartOfIfContinuation(Operation* op)
{
    if(op == nullptr)
    {
        return false;
    }

    return op->Type == OperationType::Else || op->Type == OperationType::ElseIf;
}

/// true if [op] is part of an if-conditional (if/else-if)
inline bool IsPartOfIfConditional(Operation *op)
{
    if(op == nullptr)
    {
        return false;
    }

    return op->Type == OperationType::If || op->Type == OperationType::ElseIf;
}




// ---------------------------------------------------------------------------------------------------------------------
// Flattening the AST

/// true if [op] (assumed to be OperationType::Ref) points to a primitive 
inline bool OperationRefIsPrimitive(Operation* op)
{
    return !IsReferenceStub(op->Value);
}

/// true if [op] (assumd to be OperationType::Ref) points to a Call for a
/// primitive type i.e. Object/Nothing...
inline bool IsSimpleCall(Operation* op)
{
    auto ref = op->Value;
    if(IsReferenceStub(ref))
    {
        return ref->Name == "Object" 
            || ref->Name == "Nothing" 
            || ref->Name == "Array"
            || ref->Name == "Integer"
            || ref->Name == "Decimal"
            || ref->Name == "String"
            || ref->Name == "Boolean";
    }

    return false;
}

/// flatten a OperationType::Ref [op] which may point to a primitive object
void FlattenOperationRefDirect(Operation* op)
{
    uint8_t opId;
    extArg_t arg = op->EntityIndex;

    if(OperationRefIsPrimitive(op))
    {
        opId = IndexOfInstruction(BCI_LoadPrimitive);
        AddByteCodeInstruction(opId, arg);
    }
    else
    {
        opId = IndexOfInstruction(BCI_LoadCallName);
        AddByteCodeInstruction(opId, arg);

        opId = IndexOfInstruction(BCI_ResolveDirect);
        AddByteCodeInstruction(opId, noArg);
    }
}

/// flattens an OperationType::Ref [op] which cannot be a primitive (it is scoped)
void FlattenOperationRefScoped(Operation* op, bool& isRef)
{
    uint8_t opId = IndexOfInstruction(BCI_LoadCallName);
    extArg_t arg = op->EntityIndex;

    isRef = true;

    AddByteCodeInstruction(opId, arg);
}

/// flattens a OperationType::ScopeResolution [op] assuming it is direct and has
/// only 1 operand
void FlattenOperationScopeResolutionDirect(Operation* op)
{
    auto firstOperand = op->Operands[0];
    FlattenOperationRefDirect(firstOperand);
}

/// flattens a OperationType::ScopeResolution [op] assuming it is scoped and has
/// exactly 2 operands
void FlattenOperationScopeResolutionScoped(Operation* op)
{
    auto firstOperand = op->Operands[0];
    if(firstOperand->Type == OperationType::ScopeResolution)
    {
        FlattenOperationScopeResolution(firstOperand);
    }
    else
    {
        /// must be an expression
        FlattenOperation(firstOperand);
    }

    auto secondOperand = op->Operands[1];
    bool isRef = false;
    FlattenOperationRefScoped(secondOperand, isRef);

    if(isRef)
    {
        uint8_t opId = IndexOfInstruction(BCI_ResolveScoped);
        AddByteCodeInstruction(opId, noArg);
    }
}

/// TODO: might (won't) work with scoped pritmitives (ie X.4)
/// flattens an OperationType::ScopeResolution [op] w
void FlattenOperationScopeResolution(Operation* op)
{
    if(op->Operands.size() == 1)
    {
        FlattenOperationScopeResolutionDirect(op);
    }
    else
    {
        // must be case of scoped resolution
        FlattenOperationScopeResolutionScoped(op);
    }
}

/// true if [op] is a comparison operation
bool IsOperationComparision(Operation* op)
{
    auto opType = op->Type;
    return opType == OperationType::IsGreaterThan
        || opType == OperationType::IsLessThan
        || opType == OperationType::IsGreaterThanOrEqualTo
        || opType == OperationType::IsLessThanOrEqualTo;
}

/// flattens a comparison operation [op] based on the following argument encoding
/// bit at position [i] of arg indicates:
/// i: indicates
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

/// flattens generic operations [op] which all follow the the same format and are generally
/// fixed in their operands 
/// compatible with:
/// add/subtract/multiply/divide
/// and/or/not
/// syscall (print)
/// while/if/elseif/else
/// isequal/is
/// array
/// dotypebinding new
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

        case OperationType::DoTypeBinding:
        opId = IndexOfInstruction(BCI_BindType);
        break;

        case OperationType::New:
        opId = IndexOfInstruction(BCI_Copy);
        break;

        case OperationType::Is:
        opId = IndexOfInstruction(BCI_Is);
        break;
        
        case OperationType::IsEqual:
        opId = IndexOfInstruction(BCI_Equals);
        break; 

        case OperationType::IsNotEqual:
        opId = IndexOfInstruction(BCI_NotEquals);
        break;

        case OperationType::Array:
        opId = IndexOfInstruction(BCI_Array);
        break;

        case OperationType::Print:
        opId = IndexOfInstruction(BCI_SysCall);
        arg = 0;
        break;

        case OperationType::If:
        case OperationType::ElseIf:
        case OperationType::Else:
        case OperationType::While:
        return;
        
        default:
        LogIt(LogSeverityType::Sev3_Critical, "FlattenOperationGeneric", Msg("unknown or unimplemented, %s", ToString(op->Type)));
        return;
    }
    AddByteCodeInstruction(opId, arg);
}

/// adds bytecode instructions for [op] with OperationType::Assign
inline void FlattenOperationAssign(Operation* op)
{
    if(op->Operands[0]->Type == OperationType::ScopeResolution)
    {
        FlattenOperationScopeResolution(op->Operands[0]);
    }
    else if(op->Operands[0]->Type == OperationType::Ref)
    {
        FlattenOperationRefDirect(op->Operands[0]);
    }
    else
    {
        FlattenOperation(op->Operands[0]);
    }

    FlattenOperation(op->Operands[1]);
    uint8_t opId = IndexOfInstruction(BCI_Assign);
    AddByteCodeInstruction(opId, noArg);
}

/// adds bytecode instructions for [op] with OperationType::DefineMethod
inline void FlattenOperationDefineMethod(Operation* op)
{
    uint8_t opId;
    extArg_t arg;
    
    arg = noArg;

    if(op->Operands.size() > 0)
    {
        AddInstructionsForDefMethodParameters(op->Operands[0], arg);
    }
    else
    {
        opId = IndexOfInstruction(BCI_EnterLocal);
        AddByteCodeInstruction(opId, 1);

        opId = IndexOfInstruction(BCI_LeaveLocal);
        AddByteCodeInstruction(opId, noArg);
    }

    opId = IndexOfInstruction(BCI_BindScope);
    AddByteCodeInstruction(opId, arg);
}

inline void FlattenOperationEvaluateAsArrayInitialization(Operation* op)
{
    uint8_t opId;
    extArg_t arg;

    opId = IndexOfInstruction(BCI_LoadCallName);
    arg = NOTHING_CALL_ID; 
    AddByteCodeInstruction(opId, arg);

    opId = IndexOfInstruction(BCI_ResolveDirect);
    AddByteCodeInstruction(opId, arg);

    opId = IndexOfInstruction(BCI_LoadCallName);
    arg = ARRAY_CALL_ID;
    AddByteCodeInstruction(opId, arg);

    opId = IndexOfInstruction(BCI_ResolveDirect);
    AddByteCodeInstruction(opId, noArg);

    opId = IndexOfInstruction(BCI_Copy);
    AddByteCodeInstruction(opId, noArg);

    AddInstructionsForEvaluateParameters(op, arg);

    opId = IndexOfInstruction(BCI_Eval);
    AddByteCodeInstruction(opId, 1);
}

inline void FlattenOperationNew(Operation* op)
{
    uint8_t opId;
    extArg_t arg; 

    opId = IndexOfInstruction(BCI_LoadCallName);
    arg = NOTHING_CALL_ID; 
    AddByteCodeInstruction(opId, arg);

    opId = IndexOfInstruction(BCI_ResolveDirect);
    AddByteCodeInstruction(opId, arg);


    FlattenOperation(op->Operands[0]);

    /// methods are done on cloned objects
    opId = IndexOfInstruction(BCI_Copy);
    AddByteCodeInstruction(opId, noArg);
    
    opId = IndexOfInstruction(BCI_Eval);
    arg = 0;
    AddByteCodeInstruction(opId, arg);
}

/// adds bytecode instructions for [op] with OperationType::Evaluate
inline void FlattenOperationEvaluate(Operation* op)
{
    /// order of operands is caller, method, params
    auto callerOp = op->Operands[0];
    auto methodOp = op->Operands[1];

    uint8_t opId;
    extArg_t arg;

    if(methodOp->Type == OperationType::DoTypeBinding && methodOp->Operands[0]->EntityIndex == ARRAY_CALL_ID)
    {
        FlattenOperationEvaluateAsArrayInitialization(op);
        return;
    }

    if(methodOp->Type == OperationType::Ref && OperationRefIsPrimitive(methodOp) && methodOp->EntityIndex == ARRAY_CALL_ID)
    {
        FlattenOperationEvaluateAsArrayInitialization(op);
        return;
    }

    /// case with no caller
    if(callerOp->Type == OperationType::Ref && callerOp->Value->Name == "Nothing")
    {
        opId = IndexOfInstruction(BCI_LoadCallName);
        arg = NOTHING_CALL_ID; 
        AddByteCodeInstruction(opId, arg);

        opId = IndexOfInstruction(BCI_ResolveDirect);
        AddByteCodeInstruction(opId, arg);

        FlattenOperation(methodOp);
    }
    else
    {
        // case for Expr.Ref()
        FlattenOperation(callerOp);

        opId = IndexOfInstruction(BCI_Dup);
        AddByteCodeInstruction(opId, noArg);

        opId = IndexOfInstruction(BCI_LoadCallName);
        arg = methodOp->EntityIndex;
        AddByteCodeInstruction(opId, arg);

        opId = IndexOfInstruction(BCI_ResolveScoped);
        AddByteCodeInstruction(opId, noArg);
    }
    

    /// methods are done on cloned objects
    opId = IndexOfInstruction(BCI_Copy);
    AddByteCodeInstruction(opId, noArg);
    
    arg = 0;
    AddInstructionsForEvaluateParameters(op, arg);

    opId = IndexOfInstruction(BCI_Eval);
    AddByteCodeInstruction(opId, arg);
}

/// adds bytecode instructions for [op] with OperationType::EvaluateHere
inline void FlattenOperationEvaluateHere(Operation* op)
{
    op = op->Operands[0];
    /// order of operands is caller, method, params
    auto callerOp = op->Operands[0];
    auto methodOp = op->Operands[1];

    uint8_t opId;
    extArg_t arg;

    if(callerOp->Type == OperationType::Ref && callerOp->Value->Name == "Nothing")
    {
        opId = IndexOfInstruction(BCI_LoadCallName);
        arg = methodOp->EntityIndex;
        AddByteCodeInstruction(opId, arg);

        opId = IndexOfInstruction(BCI_ResolveDirect);
        AddByteCodeInstruction(opId, noArg);
    }
    else
    {
        FlattenOperation(callerOp);

        opId = IndexOfInstruction(BCI_LoadCallName);
        arg = methodOp->EntityIndex;
        AddByteCodeInstruction(opId, arg);

        opId = IndexOfInstruction(BCI_ResolveScoped);
        AddByteCodeInstruction(opId, noArg);
    }

    /// methods are done on cloned objects
    opId = IndexOfInstruction(BCI_Copy);
    AddByteCodeInstruction(opId, noArg);
    
    arg = 0;
    AddInstructionsForEvaluateParameters(op, arg);

    opId = IndexOfInstruction(BCI_EvalHere);
    AddByteCodeInstruction(opId, arg);
}

/// adds bytecode instructions for [op] with OperationType::Return
inline void FlattenOperationReturn(Operation* op)
{
    uint8_t opId;
    extArg_t arg;
    
    if(op->Operands.size() == 0)
    {
        arg = noArg;
    }
    else
    {
        FlattenOperation(op->Operands[0]);
        arg = 1;
    }

    opId = IndexOfInstruction(BCI_Return);
    AddByteCodeInstruction(opId, arg);    
}

/// adds bytecode instructions for [op] with OperationType::Ask
inline void FlattenOperationAsk(Operation* op)
{
    uint8_t opId = IndexOfInstruction(BCI_SysCall);
    extArg_t arg;

    if(op->Operands.size() > 0)
    {
        FlattenOperation(op->Operands[0]);
        arg = 0;
        AddByteCodeInstruction(opId, noArg);

        AddEndLineInstruction();
    }

    arg = 1;
    AddByteCodeInstruction(opId, arg);
}

/// adds bytecode instructions for an [op] based on its OperationType
void FlattenOperation(Operation* op)
{
    if(op->Type == OperationType::ScopeResolution)
    {
        FlattenOperationScopeResolution(op);
    }
    else if(op->Type == OperationType::Ref)
    {
        FlattenOperationRefDirect(op);
    }
    else if(op->Type == OperationType::Assign)
    {
        FlattenOperationAssign(op);
    }
    else if(op->Type == OperationType::DefineMethod)
    {
        FlattenOperationDefineMethod(op);
        GlobalBlockOwner = op;
    }
    else if(op->Type == OperationType::Evaluate)
    {
        FlattenOperationEvaluate(op);
    }
    else if(op->Type == OperationType::Return)
    {
        FlattenOperationReturn(op);
    }
    else if(IsOperationComparision(op))
    {
        FlattenOperationComparison(op);
    }
    else if(op->Type == OperationType::EvaluateHere)
    {
        FlattenOperationEvaluateHere(op);
    }
    else if(op->Type == OperationType::Ask)
    {
        FlattenOperationAsk(op);
    }
    else if(op->Type == OperationType::New)
    {
        FlattenOperationNew(op);
    }
    else
    {
        FlattenOperationGeneric(op);
    }
}




// ---------------------------------------------------------------------------------------------------------------------
// Control flow

/// adds instructions to enter an anonymous scope and do [block]
inline void HandleAnonymousScope(Block* block)
{
    uint8_t opId;
    extArg_t arg = noArg;

    opId = IndexOfInstruction(BCI_EnterLocal);
    AddByteCodeInstruction(opId, arg);

    FlattenBlock(block);
    
    opId = IndexOfInstruction(BCI_LeaveLocal);
    AddByteCodeInstruction(opId, arg);

    opId = IndexOfInstruction(BCI_DropTOS);
    AddByteCodeInstruction(opId, arg);
}

/// adds instructions to enter a while loop's [block] with the while loop gate
/// clause starting at [whileInstructionStart]
inline void HandleWhile(Block* block, extArg_t whileInstructionStart)
{
    uint8_t opId;
    extArg_t arg = noArg;
    extArg_t JumpInstructionStart = NextInstructionId();
    AddNOPS(NOPSafetyDomainSize());

    FlattenBlock(block);
    
    opId = IndexOfInstruction(BCI_Jump);
    arg = whileInstructionStart;
    AddByteCodeInstruction(opId, arg);


    opId = IndexOfInstruction(BCI_JumpFalse);
    arg = NextInstructionId();
    RewriteByteCodeInstruction(opId, arg, JumpInstructionStart);
}

/// add instructions to skip over executing [block] when defining it and returning out
/// of [block] after execution
inline void HandleDefineMethod(Block* block)
{
    uint8_t opId;
    extArg_t arg = noArg;

    extArg_t BindInstructionStart = NextInstructionId();
    AddNOPS(NOPSafetyDomainSize());
    extArg_t JumpInstructionStart = NextInstructionId();
    AddNOPS(NOPSafetyDomainSize());

    opId = IndexOfInstruction(BCI_BindSection);
    arg = NextInstructionId();
    RewriteByteCodeInstruction(opId, arg, BindInstructionStart);

    FlattenBlock(block);

    opId = IndexOfInstruction(BCI_Return);
    AddByteCodeInstruction(opId, noArg);

    opId = IndexOfInstruction(BCI_Jump);
    arg = NextInstructionId();
    RewriteByteCodeInstruction(opId, arg, JumpInstructionStart);
    AddEndLineInstruction();
}

/// add instructions enteri if and else-if clauses
inline void HandleIf(Block* block, JumpContext& ctx)
{
    HandleAnonymousScope(block);
    AddInstructionsForUnresolvedJump(ctx);
}

/// add instructions when presented with a [block] that may have a [blockOwner] (any gating instruction)
/// that starts at [blockOwnerInstructionStart] and where [ctx] is the JumpContext of any
/// if-complex that the block may be a part of
inline void HandleFlatteningControlFlow(Block* block, Operation* blockOwner, extArg_t blockOwnerInstructionStart, JumpContext& ctx)
{
    if(JumpContextNeedsResolution(ctx) && !IsPartOfIfComplex(blockOwner))
    {
        ResolveJumpContext(ctx);
    }

    if(blockOwner == nullptr)
    {
        HandleAnonymousScope(block);
    }
    else if(blockOwner->Type == OperationType::While)
    {
        HandleWhile(block, blockOwnerInstructionStart);
    }
    else if(blockOwner->Type == OperationType::If || blockOwner->Type == OperationType::ElseIf)
    {
        HandleIf(block, ctx);
    }
    else if(blockOwner->Type == OperationType::Else)
    {
        HandleAnonymousScope(block);
    }
    else if(blockOwner->Type == OperationType::DefineMethod)
    {
        HandleDefineMethod(block);
    }
    /// TODO: figure this out
    else if(blockOwner->Type == OperationType::Ref)
    {
        HandleDefineMethod(block);
    }
    else
    {
        HandleAnonymousScope(block);
    }
}

/// add instructions when presented with an [op] and sets [blockOwner] and [blockOwnerInstructionStart]
/// to the values corresponding to the flatten bytecode of [op]. will resolve [ctx] if necessary
inline void HandleFlatteningOperation(Operation* op, extArg_t& blockOwnerInstructionStart, Operation** blockOwner, JumpContext& ctx)
{
    if(JumpContextNeedsResolution(ctx) && !IsPartOfIfContinuation(op))
    {
        ResolveJumpContext(ctx);
    }
    else if(JumpContextHasUnresolvedClause(ctx))
    {
        ResolveJumpContextClause(ctx);
    }
    
    blockOwnerInstructionStart = NextInstructionId();
    GlobalBlockOwner = nullptr;
    FlattenOperation(op);
    if(GlobalBlockOwner != nullptr)
    {
        *blockOwner = GlobalBlockOwner;
    }
    else
    {
        *blockOwner = op;
    }
    

    ByteCodeLineAssociation.push_back(NextInstructionId());
    
    if(!IsConditionalJump((*blockOwner)) && (*blockOwner)->Type != OperationType::Else && (*blockOwner)->Type != OperationType::DefineMethod) 
    {
        AddEndLineInstruction();
    }

    if(IsPartOfIfConditional(op))
    {
        AddInstructionsForUnresolvedJumpFalse(ctx);
    }
}

/// assumes that all ifs/whiles/methods have blocks
void FlattenBlock(Block* block)
{
    Operation* blockOwner = nullptr;
    extArg_t blockOwnerInstructionStart = 0;
    
    JumpContext ctx;
    InitJumpContext(ctx);

    for(auto exec: block->Executables)
    {
        switch(exec->ExecType)
        {
            case ExecutableType::Block:
            HandleFlatteningControlFlow(static_cast<Block*>(exec), blockOwner, blockOwnerInstructionStart, ctx);
            blockOwner = nullptr;
            break;

            case ExecutableType::Operation:
            HandleFlatteningOperation(static_cast<Operation*>(exec), blockOwnerInstructionStart, &blockOwner, ctx);

            break;
        }
    }

    if(JumpContextNeedsResolution(ctx) && !IsPartOfIfContinuation(blockOwner))
    {
        ResolveJumpContext(ctx);
    }
}


// ---------------------------------------------------------------------------------------------------------------------
// First Pass
// The first pass fills in the CallNames and ConstPrimitives arrrays and maps 
// from the string representation to the integer encoding (position in list) for references
// and constant primitives

std::vector<Object*> PrimitiveObjectsEncountered;

/// true if CallNames array contains [callName], sets [atPosition] to this position
bool CallNamesContains(String callName, size_t& atPosition)
{
    for(size_t i=0; i<CallNames.size(); i++)
    {
        auto name = CallNames[i];
        if(name == callName)
        {
            atPosition = i;
            return true;
        }
    }
    return false;
}

/// add a new entry to CallNames if needed
/// assumes [op] is OperationType::Ref
void IfNeededAddReferenceName(Operation* op)
{
    auto callName = op->Value->Name;

    size_t atPosition;
    if(CallNamesContains(callName, atPosition))
    {
        op->EntityIndex = atPosition + SIMPLE_CALLS;
        return;
    }

    op->EntityIndex = CallNames.size() + SIMPLE_CALLS;
    CallNames.push_back(callName);
}

bool EncounteredPrimitiveObject(Object* obj, size_t& atPosition)
{
    for(size_t i=0; i<PrimitiveObjectsEncountered.size(); i++)
    {
        auto encounteredObj = PrimitiveObjectsEncountered[i];
        if(obj == encounteredObj)
        {
            atPosition = i;
            return true;
        }
    }
    return false;
}

BindingType ObjectClassToType(Object* obj)
{
    if(obj->Class == IntegerClass)
    {
        return &IntegerType;
    }
    else if(obj->Class == DecimalClass)
    {
        return &DecimalType;
    }
    else if(obj->Class == BooleanClass)
    {
        return &BooleanType;
    }
    else if(obj->Class == StringClass)
    {
        return &StringType;
    }
    else if(obj->Class == MethodClass)
    {
        return &MethodType;
    }
    else
    {
        return &NothingType;
    }
}

void AssignValueFromObject(Call* c, Object* obj)
{
    if(obj->Class == IntegerClass)
    {
        c->BoundValue.i = *static_cast<int*>(obj->Value);
    }
    else if(obj->Class == DecimalClass)
    {
        c->BoundValue.d = *static_cast<double*>(obj->Value);
    }
    else if(obj->Class == StringClass)
    {
        c->BoundValue.s = StringConstructor(*static_cast<String*>(obj->Value));
    }
    else if(obj->Class == BooleanClass)
    {
        c->BoundValue.b = *static_cast<bool*>(obj->Value);
    }
}

/// add a new entry to ConstPrimitives if needed
/// assumes [op] is OperationType::Ref
void IfNeededAddConstPrimitive(Operation* op)
{
    auto obj = op->Value->To;
    
    size_t atPosition;
    if(EncounteredPrimitiveObject(obj, atPosition))
    {
        op->EntityIndex = atPosition;
        return;
    }

    op->EntityIndex = PrimitiveObjectsEncountered.size();
    PrimitiveObjectsEncountered.push_back(obj);
    
    Call* call = CallConstructor();
    AssignValueFromObject(call, obj);
    BindScope(call, &SomethingScope);
    /// TODO: make this more robust
    BindType(call, ObjectClassToType(op->Value->To));
    ConstPrimitives.push_back(call);
}

void AssignEntityIndexForSimpleCall(Operation* op)
{
    auto name = op->Value->Name;
    if(name == "Object")
    {
        op->EntityIndex = OBJECT_CALL_ID;
    }
    else if(name == "Nothing")
    {
        op->EntityIndex = NOTHING_CALL_ID;
    }
    else if(name == "Array")
    {
        op->EntityIndex = ARRAY_CALL_ID;
    }
    else if(name == "Integer")
    {
        op->EntityIndex = INTEGER_CALL_ID;
    }
    else if(name == "Decimal")
    {
        op->EntityIndex = DECIMAL_CALL_ID;
    }
    else if(name == "String")
    {
        op->EntityIndex = STRING_CALL_ID;
    }
    else if(name == "Boolean")
    {
        op->EntityIndex = BOOLEAN_CALL_ID;
    }
    else
    {
        LogIt(LogSeverityType::Sev1_Notify, 
            "AssignEntityIndexForSimpleCall", "bad simple call name");
    }
}

/// recurses over the ast with [op] as head 
void FirstPassOperation(Operation* op)
{
    if(op->Type == OperationType::Ref)
    {
        if(OperationRefIsPrimitive(op))
        {
            IfNeededAddConstPrimitive(op);
        }
        else if(IsSimpleCall(op))
        {
            AssignEntityIndexForSimpleCall(op);
        }
        else
        {
            IfNeededAddReferenceName(op);
        }
        return;
    }

    for(auto operand: op->Operands)
    {
        FirstPassOperation(operand);
    }
}

/// iterates through the ast list [b]
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

std::vector<void*> DeletedValueAddrs;

/// resets the CallNames and ConstPrimtiives lists
void InitEntityLists()
{
    ConstPrimitives.clear();
    ConstPrimitives.reserve(64);

    CallNames.clear();
    CallNames.reserve(64);

    PrimitiveObjectsEncountered.clear();
    PrimitiveObjectsEncountered.reserve(64);
    
    ByteCodeProgram.clear();
    ByteCodeLineAssociation.clear();
    ByteCodeLineAssociation.push_back(0);

    DeletedValueAddrs.clear();
    DeletedValueAddrs.reserve(64);
}

/// iterates through the main block of [p]
void FirstPassProgram(Program* p)
{
    FirstPassBlock(p->Main);
}

void SafeDeleteObjectValue(Object* obj)
{
    if(obj->Value == nullptr)
    {
        return;
    }


    for(void* addr: DeletedValueAddrs)
    {
        if(obj->Value == addr)
        {
            return;
        }
    }

    DeletedValueAddrs.push_back(obj->Value);
    ObjectValueDestructor(obj->Class, obj->Value);
}

void InternalDeleteObject(Object* obj)
{
    ScopeDestructor(obj->Attributes);
    if(IsCallable(obj))
    {
        MethodDestructor(obj->Action);
    }
    SafeDeleteObjectValue(obj);
    
    ObjectDestructor(obj);
}

/// performs first pass and the flattens [p] to generate a BytecodeProgram
void FlattenProgram(Program* p)
{
    InitEntityLists();
    FirstPassProgram(p);
    FlattenBlock(p->Main);

    for(auto obj: PrimitiveObjectsEncountered)
    {
        InternalDeleteObject(obj);
    }
}
