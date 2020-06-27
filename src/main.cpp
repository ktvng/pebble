#include <iostream>

#include "main.h"
#include "arch.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"
#include "reference.h"




int main()
{
    PurgeLog();
    CompileGrammar();

    bool PRINT_OPERATIONS = false;
    bool PRINT_GLOBAL_REFS = false;
    LogIt(LogSeverityType::Sev1_Notify, "main", "program compile begins");
    ParseProgram(".\\program");
    LogIt(LogSeverityType::Sev1_Notify, "main", "program compile finished");
    
    for(Block* b: PROGRAM->Blocks)
        LogDiagnostics(b, "initial program parse structure", "main");
    
    // PRINT OPERATIONS
    if(PRINT_OPERATIONS)
    {
    }


    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        LogDiagnostics(map, "initial object reference state", "main");
    }

    // for(auto elem: PROGRAM->Blocks.at(0).LocalScope->ReferencesIndex)
    //     LogDiagnostics(elem);

    std::cout << "####################\n";
    LogIt(LogSeverityType::Sev1_Notify, "main", "program execution begins");
    DoProgram(*PROGRAM);
    LogIt(LogSeverityType::Sev1_Notify, "main", "program execution finished");
    std::cout << "####################\n";
    
    // PRINT GLOBALREFRENCES
    if(PRINT_GLOBAL_REFS)
    {
        // for(auto elem: GlobalScope.ReferencesIndex)
        // {
        //     std::cout << &elem << "\n";
        //     LogDiagnostics(*elem);
        // }
    }
    
    // std::string line = "test Of the Token: ;::;4 : 334 par.ser=4 &.&.3&&& = 3.1 haha \"this is awesome\" True";
    // TokenList l = LexLine(line);
    // std::cout << "######\n"; 
    // int pos=0;
    // Token* t;
    // for(t = NextTokenMatching(l, ObjectTokenTypes, pos); t != nullptr; t = NextTokenMatching(l, ObjectTokenTypes, pos))
    // {
    //     LogDiagnostics(t);
    // }
    // LogDiagnostics(l);

    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        LogDiagnostics(map, "final object reference state", "main");
    }


    // Testing New Parser
    
    // PROGRAM = new Program;
    // PROGRAM->GlobalScope = ScopeConstructor(nullptr);
    // EnterScope(PROGRAM->GlobalScope);


    // String str = "if(true):";
    // TokenList tl = LexLine(str);
    // Operation* op = ExpressionParser(tl);
    // LogDiagnostics(op);

    LogItDebug("end reached.", "main");

    return 0;
}
