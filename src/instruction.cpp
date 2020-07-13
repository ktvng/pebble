#include "instruction.h"
#include "object.h"
#include "main.h"
#include "program.h"
#include "scope.h"
#include "reference.h"
#include "operation.h"
#include "cctype" //remove
#include "diagnostics.h"

std::vector<InstructionCall> ProgramInstructions;

std::vector<Object*> Memory;
std::vector<Reference*> ReferencesCache;

Reference* ReferenceRegister = nullptr;

int InstructionPointer = 0;

ReferenceId nullArg1 = "noArg";
JumpTo nullArg2 = 0;

void InitModule()
{
    Memory.reserve(64);
    Memory.clear();

    ReferencesCache.clear();
    ReferencesCache.reserve(8);
}

void DoNextInstruction()
{
    InstructionCall& i = ProgramInstructions[InstructionPointer];
    LogItDebug(ToString(i), "DoNextInstruction");
    i.Op(i.Arg1, i.Arg2);

    InstructionPointer++;
}

inline bool ExecutionFinished()
{
    return static_cast<size_t>(InstructionPointer) >= ProgramInstructions.size();
}

void DoAllInstructions()
{
    while(!ExecutionFinished())
    {
        DoNextInstruction();
    }
}

inline void AddObjectToMemory(Object* obj)
{
    Memory.push_back(obj);
}

inline Object* PopMemory()
{
    Object* obj = Memory.back();
    Memory.pop_back();
    return obj;
}

inline Reference* PopReferenceCache()
{
    Reference* ref = ReferencesCache.back();
    ReferencesCache.pop_back();
    return ref;
}

bool IsNumeric(const Object* obj)
{
    return obj->Class == IntegerClass || obj->Class == DecimalClass;
}

/// TODO: Doesn't assign object class
inline Object* MakeIntObjectInternal(void* val)
{
    Object* obj = ObjectConstructor();
    obj->Value = val;
    obj->Class = IntegerClass;
    return obj;
}

/// TODO: currently only works for integers
void InstructionAdd(ReferenceId id, JumpTo i)
{
    Object* obj2 = PopMemory();
    Object* obj1 = PopMemory();

    Object* result = nullptr;
    if(IsNumeric(obj1) && IsNumeric(obj2))
    {
        int* val = new int;
        *val =  GetIntValue(*obj1) + GetIntValue(*obj2);
        result = MakeIntObjectInternal(val);
        Memory.push_back(result);
    }
    else
    {
        Memory.push_back(NullObject());
    }
}

void InstructionPrint(ReferenceId id, JumpTo i)
{
    Object* obj1 = PopMemory();

    if(g_outputOn)
        std::cout << GetStringValue(*obj1) << "\n";
    ProgramOutput.append(GetStringValue(*obj1) + "\n");

    Object* result = NullObject();
    Memory.push_back(result);
}    

void InstructionDirectReferenceResolution(ReferenceId id, JumpTo i)
{
    Reference* ref = GetReference(id);
    if(ref == nullptr)
    {
        ReferenceRegister = NullReference(id);
        Memory.push_back(NullObject());
    }
    else
    {
        ReferenceRegister = ref;
        auto obj = ref->To;
        Memory.push_back(obj);
    }
}

/// TODO: handle "scoped primitives"
void InstructionScopedReferenceResolution(ReferenceId id, JumpTo i)
{
    // containing object
    Object* container = PopMemory();

    Reference* ref = nullptr;
    EnterScope(container->Attributes);
    {
        ref = GetReference(id);
    }
    ExitScope();

    if(ref == nullptr)
    {
        ReferenceRegister = NullReference(id);
        Memory.push_back(NullObject());
    }
    else
    {
        ReferenceRegister = ref;
        auto obj = ref->To;
        Memory.push_back(obj);
    }
}

void InstructionAssign(ReferenceId id, JumpTo i)
{
    Object* obj1 = PopMemory();
    Reference* ref = PopReferenceCache();

    LogDiagnostics(obj1);

    /// TODO: should throw error
    if(ref == nullptr)
    {
        Memory.push_back(NullObject());
    }
    else
    {
        ReassignReference(ref, obj1);
        Memory.push_back(obj1);
    }
}

void InstructionIf(ReferenceId id, JumpTo i)
{
    Object* obj1 = PopMemory();

    if(!GetBoolValue(*obj1))
    {
        InstructionPointer = i;
    }
}

void InstructionJump(ReferenceId id, JumpTo i)
{
    InstructionPointer = i;
}




// ---------------------------------------------------------------------------------------------------------------------
// Flattening

std::vector<InstructionCall*> UnresolvedJumps;


inline ReferenceId RefernceIdDirect(Operation* op)
{
    String refName = op->Operands.at(0)->Value->Name;
    if(refName != c_operationReferenceName)
    {
        return refName;
    }
    else
    {
        return GetStringValue(*op->Operands.at(0)->Value->To);
    }
}

inline ReferenceId ReferenceIdScoped(Operation* op)
{
    return op->Operands.at(1)->Value->Name;
}

inline void OpTypeScopeResolution(Operation* op)
{
    if(op->Operands.size() == 1)
    {
        // this is the case for direct reference resolution
        ReferenceId arg = RefernceIdDirect(op);
        ProgramInstructions.push_back( {InstructionDirectReferenceResolution, arg, nullArg2} );
    }
    else
    {
        // this is the case for scoped reference resolution
        ReferenceId arg = ReferenceIdScoped(op);
        ProgramInstructions.push_back( {InstructionScopedReferenceResolution, arg} );
    }
}

void InstructionAddReferenceToCache(ReferenceId id, JumpTo i)
{
    ReferencesCache.push_back(ReferenceRegister);
}

InstructionCall* BlockOwner = nullptr;

inline void OpTypeIf(Operation* op, int instructionNumber)
{
    ProgramInstructions.push_back( {InstructionIf, nullArg1, instructionNumber} );
    BlockOwner = &ProgramInstructions.back();
}

void FlattenAST(Operation* astNode, int& instructionNumber)
{
    // if the block owner was set but an operation was listed immediately afterwards, then that 
    // block owner has no block and is removed as a block owner.
    BlockOwner = nullptr;

    if(astNode->Type == OperationType::Ref)
        return;
    
    bool firstTime = true;
    for(auto op: astNode->Operands)
    {
        FlattenAST(op, instructionNumber);

        /// assign is the only operation which needs to know the reference it operates on
        if(astNode->Type == OperationType::Assign && firstTime)
        {
            firstTime = false;
            ProgramInstructions.push_back( {InstructionAddReferenceToCache, nullArg1, nullArg2} );
        }

    }

    switch(astNode->Type)
    {
        case OperationType::ScopeResolution:
        OpTypeScopeResolution(astNode);
        break;

        case OperationType::Add:
        ProgramInstructions.push_back( {InstructionAdd, nullArg1, nullArg2} );
        break;

        case OperationType::Assign:
        ProgramInstructions.push_back( {InstructionAssign, nullArg1, nullArg2} );
        break;

        case OperationType::Print:
        ProgramInstructions.push_back( {InstructionPrint, nullArg1, nullArg2} );
        break;

        case OperationType::If:
        OpTypeIf(astNode, instructionNumber);
        break;

        default:
        return;
    }
    instructionNumber++;
}

void FlattenBlock(Block* b, int& instructionNumber)
{
    InstructionCall* localBlockOwnerCopy = BlockOwner;

    for(auto exec: b->Executables)
    {
        switch(exec->ExecType)
        {
            case ExecutableType::Block:
            FlattenBlock(static_cast<Block*>(exec), instructionNumber);
            break;

            case ExecutableType::Operation:
            FlattenAST(static_cast<Operation*>(exec), instructionNumber);
            break;
        }
    }

    if(localBlockOwnerCopy != nullptr)
    {
        localBlockOwnerCopy->Arg2 = instructionNumber;
    }
}

void FlattenProgram(Program* p)
{
    int i = 0;
    FlattenBlock(p->Main, i);
}

String ToString(InstructionCall& call)
{
    String str;

    auto op = call.Op;
    if(op == InstructionDirectReferenceResolution)
    {
        str += "DirectReferenceResolution: " + call.Arg1 + "\n";
    }
    else if(op == InstructionScopedReferenceResolution)
    {
        str += "ScopedReferenceResolution: " + call.Arg1 + "\n";
    }
    else if(op == InstructionAssign)
    {
        str += "Assign: \n";
    }
    else if(op == InstructionPrint)
    {
        str += "Print: \n";
    }
    else if(op == InstructionAdd)
    {
        str += "Add: \n";
    }
    else
    {
        str += "<unimplemented>\n";
    }

    return str;
}

String ToString(std::vector<InstructionCall>& instructions)
{
    String str = "";
    for(auto& call: instructions)
    {
        str += ToString(call);
    }

    return str;
}

void PrintProgramInstructions()
{
    std::cout << ToString(ProgramInstructions);
}