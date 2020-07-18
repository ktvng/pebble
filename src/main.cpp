#include <iostream>
#include <ctime>
#include <chrono>
#include <ratio>

#include "main.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"
#include "reference.h"
#include "parse.h"
#include "execute.h"
#include "commandargs.h"
#include "vm.h"
#include "flattener.h"
#include "scope.h"

void ChangeLogType(std::vector<SettingOption> options)
{
    if(options.size() < 1)
        return;

    SettingOption option = options[0];
    if(option == "sev0")
    {
        LogAtLevel = LogSeverityType::Sev0_Debug;
    }
    else if(option == "sev1")
    {
        LogAtLevel = LogSeverityType::Sev1_Notify;
    }
    else if(option == "sev2")
    {
        LogAtLevel = LogSeverityType::Sev2_Important;
    }
    else if(option == "sev3")
    {
        LogAtLevel = LogSeverityType::Sev3_Critical;
    }
}

ProgramConfiguration Config
{
    { 
        "Log Setting", "--log", ChangeLogType
    }
};


// Logging
LogSeverityType LogAtLevel = LogSeverityType::Sev0_Debug;
bool g_outputOn = true;

int main(int argc, char* argv[])
{
    ParseCommandArgs(argc, argv, Config);

    bool ShouldPrintInitialCompileResult = true; 
    bool ShouldPrintProgramExecutionFinalResult = true;

    PurgeLog();                         // cleans log between each run
    CompileGrammar();                   // compile grammar from grammar.txt


    // compile program
    LogIt(LogSeverityType::Sev1_Notify, "main", "program compile begins");

    auto prog = ParseProgram("./program.pebl");
    if(FatalCompileError)
        return 1;

    LogIt(LogSeverityType::Sev1_Notify, "main", "program compile finished");

    if(ShouldPrintInitialCompileResult)
    {
        EnterProgram(prog);
        LogDiagnostics(PROGRAM->Main, "initial program parse structure", "main");
        for(ObjectReferenceMap& map: PROGRAM->ObjectsIndex)
        {
            LogDiagnostics(map, "initial object reference state", "main");
        }
    }

    FlattenProgram(PROGRAM);
    LogProgramInstructions();
    std::cout << "#######################start\n";
    
    auto start = std::chrono::high_resolution_clock::now();

    DoByteCodeProgram();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << time_span.count();

    std::cout << "#######################end\n";


    // run program
    // SetConsoleColor(ConsoleColor::LightBlue);
    // std::cout << "################################################################################\n";
    // SetConsoleColor(ConsoleColor::White);
    // LogIt(LogSeverityType::Sev1_Notify, "main", "program execution begins");

    // DoProgram(prog);
    
    // LogIt(LogSeverityType::Sev1_Notify, "main", "program execution finished");
    // SetConsoleColor(ConsoleColor::LightBlue);
    // std::cout << "################################################################################\n";
    // SetConsoleColor(ConsoleColor::White);

    // if(ShouldPrintProgramExecutionFinalResult)
    // {
    //     EnterProgram(prog);
    //     for(ObjectReferenceMap& map: PROGRAM->ObjectsIndex)
    //     {
    //         LogDiagnostics(map, "final object reference state", "main");
    //     }
    // }

    // ProgramDestructor(PROGRAM);
    LogItDebug("end reached.", "main");
    return 0;
}
