#include <iostream>

#include "execute.h"
#include "main.h"
#include "diagnostics.h"
#include "scope.h"
#include "operation.h"
#include "reference.h"
#include "program.h"
#include "object.h"

// ---------------------------------------------------------------------------------------------------------------------
// Diagnostics

/// logs scope
void LogDiagnosticsForRuntimeLine(Scope* scope, Operation* op)
{
    for(; scope != nullptr; scope=scope->InheritedScope)
        for(auto ref: scope->ReferencesIndex)
            LogDiagnostics(ref, Msg("scope before %s line %i", ToString(op->Type), op->LineNumber), "DoOperationOnReferences");
}



// ---------------------------------------------------------------------------------------------------------------------
// Program execution

Block* AsBlock(Executable* exec)
{
    return static_cast<Block*>(exec);
}

Operation* AsOperation(Executable* exec)
{
    return static_cast<Operation*>(exec);
}

/// executes an operation [op] on the ordered list of references [operands] inside [scope]
/// note: this method should only be called through DoOperation
Reference* DoOperationOnReferences(Operation* op, std::vector<Reference*> operands)
{
    return OperationEvaluators[op->Type](op->Value, operands);
}

/// resolve the references return by each operand operation
std::vector<Reference*> GetOperandReferences(Operation* op)
{
    std::vector<Reference*> operandReferences;
    
    for(Operation* operand: op->Operands)
    {
        Reference* operandRef = DoOperation(operand);
        operandReferences.push_back(operandRef);
    }
    return operandReferences;
}

/// executes an operation [op] inside [scope]
Reference* DoOperation(Operation* op)
{
    std::vector<Reference*> operandReferences = GetOperandReferences(op);

    Reference* returnRef = DoOperationOnReferences(op, operandReferences);
    LogItDebug(Msg("line[%i] operation %s returned a reference", op->LineNumber, ToString(op->Type)), "DoOperation");

    DereferenceAll(operandReferences);
    return returnRef;
}

void UpdatePreviousResult(Reference** result, Reference** previousResult)
{
    if(*previousResult != nullptr)
        Dereference(*previousResult);
    *previousResult = *result;
    PROGRAM->That = *result;
}

/// handles the If operation
Reference* HandleControlFlowIf(Operation* op, size_t& execLine)
{
    Reference* ifExpressionResult = DoOperation(op);
    bool ifIsTrue = GetBoolValue(*ObjectOf(ifExpressionResult));
    if(op->Type == OperationType::If && !ifIsTrue)
    {
        execLine++;
    }
    return ifExpressionResult;
}

/// handles the while operation
Reference* HandleControlFlowWhile(Operation* op, size_t& execLine, Block* codeBlock)
{
    Reference* whileExpression = DoOperation(op);
    bool condition = GetBoolValue(*ObjectOf(whileExpression));
    if(op->Type == OperationType::While && !condition)
    {
        execLine++;
    } else if(op->Type == OperationType::While && condition){
        if(codeBlock->Executables.size() > execLine + 1 && codeBlock->Executables.at(execLine+1)->ExecType == ExecutableType::Block)
        {
            auto ref = DoBlock(static_cast<Block *> (codeBlock->Executables.at(execLine+1)));
            AddReferenceToCurrentScope(ref);
            Dereference(ref);
            execLine -= 1;
        }
    }
    return whileExpression;
}

Reference* HandleControlFlowClass(Operation* op, size_t& execLine, Block* block)
{
    /// TODO: currently expects a block
    if(!(execLine + 1 < block->Executables.size() && block->Executables[execLine+1]->ExecType == ExecutableType::Block))
    {
        ReportRuntimeMsg(SystemMessageType::Exception, "no block after class");
        return NullReference();                
    }

    Reference* newClass = DoOperation(op);
    
    auto classBlock = AsBlock(block->Executables[++execLine]);
    auto classScope = ObjectOf(newClass)->Attributes;
    DoBlock(classBlock, classScope);

    return newClass;
}

/// executes [op] in [scope] and updates [execLine] based on the control flow properties
/// of [op]
Reference* HandleControlFlow(Operation* op, size_t& execLine, Block* block)
{
    switch(op->Type)
    {
        case OperationType::If:
        return HandleControlFlowIf(op, execLine);

        case OperationType::Class:
        return HandleControlFlowClass(op, execLine, block);

        case OperationType::While:
        return HandleControlFlowWhile(op, execLine, block);
        // for any non-control flow operation;
        default:
        return DoOperation(op);
    }
}

void HandleRuntimeMessages(int lineNumber)
{
    if(RuntimeMsgFlag)
    {
        RuntimeMsgPrint(lineNumber);
        RuntimeMsgFlag = false;
    }
}
bool shouldReturn = false;

/// executes the commands contained in a [codeBlock]
Reference* DoBlock(Block* codeBlock, Scope* scope)
{
    Reference* result = nullptr;
    Reference* previousResult = nullptr;

    shouldReturn = false;
    bool scopeIsLocal = false;
    if(scope == nullptr)
    {
        scopeIsLocal = true;
        scope = ScopeConstructor(CurrentScope());
    }
    
    EnterScope(scope);
    {
        for(auto aref: CurrentScope()->ReferencesIndex)
        {
            LogDiagnostics(aref, "scope before block");
        }

        for(size_t i=0; i<codeBlock->Executables.size(); i++)
        {
            auto exec = codeBlock->Executables.at(i);

            if(exec->ExecType == ExecutableType::Operation)
            {
                Operation* op = AsOperation(exec); 

                LogDiagnosticsForRuntimeLine(CurrentScope(), op);

                LogItDebug(Msg("starting execute line [%i] which is %s", op->LineNumber, ToString(op->Type)), "DoBlock");

                result = HandleControlFlow(op, i, codeBlock); 
                UpdatePreviousResult(&result, &previousResult);
                HandleRuntimeMessages(op->LineNumber);
                
                LogItDebug(Msg("finishes execute line [%i]", op->LineNumber), "DoBlock");

                if(op->Type == OperationType::Return)
                {
                    shouldReturn = true;
                    break;
                }
            }
            else if(exec->ExecType == ExecutableType::Block)
            {
                LogItDebug("discovered child block: starting", "DoBlock");

                result = DoBlock(AsBlock(exec));
                AddReferenceToCurrentScope(result);
                UpdatePreviousResult(&result, &previousResult);

                LogItDebug("exiting child block", "DoBlock");
                if(shouldReturn) break;
            }
        }

        // CurrentScope()->ReferencesIndex.clear();
    }
    ExitScope(scopeIsLocal);
    if(result != nullptr)
        return result;
    else
        return CreateNullReference();
}

/// executes all blocks of [program]
void DoProgram(Program& program)
{
    DoBlock(program.Main, program.GlobalScope);
}
