#include "decision.h"

#include "main.h"
#include "reference.h"
#include "operation.h"
#include "program.h"
#include "token.h"
#include "object.h"
#include "diagnostics.h"

// ---------------------------------------------------------------------------------------------------------------------
// Deciding stuff

// can put this somewhere
typedef void (*ProbabilityFunctions)(PossibleOperationsList&, const TokenList&);
ProbabilityFunctions decideProbabilities[] = 
{
    DecideProbabilityAssign,
    DecideProbabilityIsEqual,
    DecideProbabilityIsLessThan,
    DecideProbabilityIsGreaterThan,
    DecideProbabilityAdd,
    DecideProbabilitySubtract,
    DecideProbabilityMultiply,
    DecideProbabilityDivide,
    DecideProbabilityAnd,
    DecideProbabilityOr,
    DecideProbabilityNot,
    DecideProbabilityEvaluate,
    DecideProbabilityPrint,
    DecideProbabilityRef,
    DecideProbabilityDefineMethod
};

/// decide the probability that a line represented by [tokens] corresponds to each of the atomic operations and stores
/// this in [typeProbabilities]
void DecideOperationTypeProbabilities(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    for(ProbabilityFunctions pFunc : decideProbabilities)
    {
        pFunc(typeProbabilities, tokens);
    }
}

double ProbabilityForType(OperationType type, PossibleOperationsList& typeProbabilites)
{
    for(OperationTypeProbability p: typeProbabilites)
    {
        if(p.Type == type)
            return p.Probability;
    }
    return 0;
}

// TODO: figure out how to decide line type
/// assign [lineType] based on [typeProbabitlites] for each atomic operation
void DecideLineType(PossibleOperationsList& typeProbabilities, const TokenList& tokens, LineType& lineType) // MAJOR
{
    if(override)
    {
        lineType = LineType::Composite;
        return;
    }

    if(FindToken(tokens, "if") != nullptr && FindToken(tokens, ":") != nullptr)
        lineType = LineType::IfLine;
    else if(FindToken(tokens, "while") != nullptr && FindToken(tokens, ":") != nullptr)
        lineType = LineType::WhileLine;
    else
    {
        double totalProb;
        for(OperationTypeProbability p: typeProbabilities)
            totalProb += p.Probability;

        if((totalProb > 7 && ProbabilityForType(OperationType::DefineMethod, typeProbabilities) < 5))
            lineType = LineType::Composite;
        else
            lineType = LineType::Atomic;
    }
}

/// given pre-completed [typeProbabilities], decides what operation is most likey
void DecideOperationType(PossibleOperationsList& typeProbabilities, OperationType& opType)
{
    std::sort(typeProbabilities.begin(), typeProbabilities.end(),
        [](const OperationTypeProbability& ltp1, const OperationTypeProbability& ltp2){
            return ltp1.Probability > ltp2.Probability;
        });
    opType = typeProbabilities.at(0).Type;
}


typedef void(*DecideOperandsFunction)(TokenList& tokens, OperationsList&);
DecideOperandsFunction decideOperands[] = 
{
    DecideOperandsAssign,
    DecideOperandsIsEqual,
    DecideOperandsIsLessThan,
    DecideOperandsIsGreaterThan,
    DecideOperandsAdd,
    DecideOperandsSubtract,
    DecideOperandsMultiply,
    DecideOperandsDivide,
    DecideOperandsAnd,
    DecideOperandsOr,
    DecideOperandsNot,
    DecideOperandsEvaluate,
    DecideOperandsPrint,
    DecideOperandsRef,
    DecideOperandsDefineMethod
};

/// decides and adds the operations for the Operation of [opType] to [operands] 
void DecideOperands(const OperationType& opType, TokenList& tokens, OperationsList& operands)
{
    // TODO fPtr
    // makes going through a linear search to find what function to use instant with array indexing
    decideOperands[opType](tokens, operands);
}





// ---------------------------------------------------------------------------------------------------------------------
// Adding to operands

/// adds a new Operation::Ref type operation for [ref] to [operands]
void AddRefOperationTo(OperationsList& operands, Reference* ref)
{
    operands.push_back(OperationConstructor(OperationType::Ref, ref));
}

/// adds an existing Operation [op] to [operands]
void AddOperationTo(OperationsList& operands, Operation* op)
{
    operands.push_back(op);
}

// ---------------------------------------------------------------------------------------------------------------------
// Decide Probabilities

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityAdd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> addKeyWords = { "add", "plus", "+", "adding" };
    if(TokenListContainsContent(tokens, addKeyWords))
    {
        OperationTypeProbability addType = { OperationType::Add, 4.0 };
        typeProbabilities.push_back(addType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityPrint(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> printKeyWords = { "print", "display", "show", "output" };
    if(TokenListContainsContent(tokens, printKeyWords))
    {
        OperationTypeProbability printType = { OperationType::Print, 4.0 };
        typeProbabilities.push_back(printType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityAssign(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(Token* pos = FindToken(tokens, "="); pos != nullptr)
    {
        OperationTypeProbability assignType = { OperationType::Assign, 6 };
        typeProbabilities.push_back(assignType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityRef(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    OperationTypeProbability returnType = { OperationType::Ref, 1 };
    typeProbabilities.push_back(returnType);
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityIsEqual(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{

}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityIsLessThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityIsGreaterThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilitySubtract(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> subKeyWords = { "sub", "subtract", "minus", "-", "subtracting" };
    if(TokenListContainsContent(tokens, subKeyWords))
    {
        OperationTypeProbability subType = { OperationType::Subtract, 4.0 };
        typeProbabilities.push_back(subType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityMultiply(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityDivide(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityAnd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> andKeyWords = { "and", "&&", "together", "with" };
    if(TokenListContainsContent(tokens, andKeyWords))
    {
        OperationTypeProbability andType = { OperationType::And, 4.0 };
        typeProbabilities.push_back(andType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityOr(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityNot(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}


// ---------------------------------------------------------------------------------------------------------------------
// Decide operation reference value

/// handles OperationType::Ref by assigning the operation.Value to the approriate reference
void DecideValueRef(TokenList& tokens, Reference** refValue)
{
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes), c_operationReferenceName);
    *refValue = arg1;
}


// ---------------------------------------------------------------------------------------------------------------------
// Decide Operands
// should edit token list remove used tokens

/// gets references for the next two tokens which refer to Objects
void GetTwoOperands(TokenList& tokens, OperationsList& operands)
{
    int pos = 0;
 
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes, pos), c_operationReferenceName);
    Reference* arg2 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes, pos), c_operationReferenceName);

    AddRefOperationTo(operands, arg1);
    AddRefOperationTo(operands, arg2);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsAdd(TokenList& tokens, OperationsList& operands) // EDIT
{
    GetTwoOperands(tokens, operands);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsDefine(TokenList& tokens, OperationsList& operands)
{
    Reference* ref; 

    std::vector<String> arrayWords = { "array", "list", "collection" };
    if(TokenListContainsContent(tokens, arrayWords))
    {
        // do array stuff
        Token* name = NextTokenMatching(tokens, TokenType::Reference);
        Token* size = NextTokenMatching(tokens, TokenType::Integer);
        int* i = new int;
        *i = std::stoi(size->Content);

        AddRefOperationTo(operands, ReferenceFor(name->Content, ArrayClass, static_cast<void*>(i)));
        return;
    }

    // treat as primitive
    Token* name = NextTokenMatching(tokens, TokenType::Reference);
    Token* value = NextTokenMatching(tokens, PrimitiveTokenTypes);

    if(name == nullptr){
        LogIt(LogSeverityType::Sev3_Critical, "DecideOperandsDefine", "cannot determine reference name");
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference name");
        // TODO: should be critical error
        AddRefOperationTo(operands, NullReference()); 
        return;
    }
    
    if(value == nullptr)
    {
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference value");
        ref = NullReference(name->Content);
    }
    else
    {
        ref = ReferenceFor(value, name->Content);
    }
    AddRefOperationTo(operands, ref);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsPrint(TokenList& tokens, OperationsList& operands)
{
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes), c_operationReferenceName);
    AddRefOperationTo(operands, arg1);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsAssign(TokenList& tokens, OperationsList& operands)
{

    int pos = 0;
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, TokenType::Reference, pos));
    AddRefOperationTo(operands, arg1);

    TokenList rightTokens = RightOfToken(tokens, tokens.at(pos));
    tokens = rightTokens;

    Operation* op2 = ParseLine(tokens);
    AddOperationTo(operands, op2);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsIsEqual(TokenList& tokens, OperationsList& operands)
{

}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsIsLessThan(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsIsGreaterThan(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsSubtract(TokenList& tokens, OperationsList& operands)
{
    GetTwoOperands(tokens, operands);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsMultiply(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsDivide(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsAnd(TokenList& tokens, OperationsList& operands)
{
    GetTwoOperands(tokens, operands);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsOr(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsNot(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsRef(TokenList& tokens, OperationsList& operands)
{
    // no operands
}


// ---------------------------------------------------------------------------------------------------------------------
// OperationType::DefineMethod

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityDefineMethod(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(FindToken(tokens, "method") != nullptr && FindToken(tokens, ":") != nullptr)
    {
        OperationTypeProbability defineMethodType = { OperationType::DefineMethod, 10.0 };
        typeProbabilities.push_back(defineMethodType);
    }
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsDefineMethod(TokenList& tokens, OperationsList& operands)
{
    // TODO: assumes first reference is method name
    int i=0;
    Token* t = NextTokenMatching(tokens, TokenType::Reference, i);
    String methodName = t->Content;
    
    ParameterList paramNames;
    for(t = NextTokenMatching(tokens, TokenType::Reference, i); t != nullptr; t = NextTokenMatching(tokens, TokenType::Reference, i))
    {
        paramNames.push_back(t->Content);
    }

    Reference* refToMethodObj = CreateReferenceToNewObject(methodName, BaseClass, nullptr);
    refToMethodObj->To->Action = MethodConstructor();
    ObjectOf(refToMethodObj)->Action->ParameterNames = paramNames;
    
    AddRefOperationTo(operands, refToMethodObj);
}


// ---------------------------------------------------------------------------------------------------------------------
// Evaluate operation

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityEvaluate(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(FindToken(tokens, "(") != nullptr)
    {
        OperationTypeProbability evaluateType = { OperationType::Evaluate, 3.0 };
        typeProbabilities.push_back(evaluateType);
    }
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsEvaluate(TokenList& tokens, OperationsList& operands)
{
    // function name is first parameter
    int i = 0;
    for(Token* t = NextTokenMatching(tokens, TokenType::Reference, i); t != nullptr; t = NextTokenMatching(tokens, TokenType::Reference, i))
    {
        AddRefOperationTo(operands, ReferenceFor(t));
    }
}