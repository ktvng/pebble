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
#include "commandargs.h"
#include "vm.h"
#include "flattener.h"
#include "scope.h"
#include "dis.h"
#include "grammar.h"
#include "astvm.h"

#include "dfa.h"

bool g_standalone = false;

bool Usage(std::vector<SettingOption> options)
{
    // If this was derived at build time, that would be fantastic
    // TODO - generate from Settings table and use null flags as non-flag [] args
    std::cerr << "Usage pebble: [--help] [--log sev0|sev1|sev2|sev3] [program.pebl]" << std::endl;

    exit(2);
}

bool ChangeLogType(std::vector<SettingOption> options)
{
    if(options.size() < 1)
        return true;

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
    
    return true;
}

bool ChangeRuntime(std::vector<SettingOption> options)
{
    if(options.size() < 1)
        return true;

    SettingOption option = options[0];
    if(option == "ast")
    {
        g_useBytecodeRuntime = false;
    }
    else if(option == "bc")
    {
        g_useBytecodeRuntime = true;
    }

    return true;
}

ProgramConfiguration Config
{
    {
        /* 
            This is the .pebl file to ingest. 
            The flag string will be filled retro-actively with the program name, if any. 
            Must be the 0'th Setting. 
            Note: making Flag a vector rather than string could allow multiple files/args
        */
        "Pebl File", "program.pebl", nullptr
    },
    {
        "Usage", "--help", Usage
    },
    { 
        "Log Setting", "--log", ChangeLogType
    },
    {
        "Runtime version", "--runtime", ChangeRuntime
    }
};

// Logging
LogSeverityType LogAtLevel = LogSeverityType::Sev3_Critical;
bool g_outputOn = true;

// Runtime
bool g_useBytecodeRuntime = true;

int main(int argc, char* argv[])
{
    ParseCommandArgs(argc, argv, &Config);

    bool ShouldPrintInitialCompileResult = true; 

    PurgeLog();                         // cleans log between each run
    CompileGrammar();                   // compile grammar from grammar.txt


    // compile program
    LogIt(LogSeverityType::Sev1_Notify, "main", "compile begins");

    auto prog = ParseProgram(Config.at(0).Flag);
    if(FatalCompileError || prog == nullptr)
    {
        LogIt(LogSeverityType::Sev2_Important, "main", "compile fails");
        return 1;
    }

    PROGRAM = prog;
    LogIt(LogSeverityType::Sev1_Notify, "main", "compile finished");

    if(ShouldPrintInitialCompileResult)
    {
        LogDiagnostics(PROGRAM->Main, "initial parse structure", "main");
    }

    if(g_useBytecodeRuntime)
    {
        FlattenProgram(PROGRAM);
        LogProgramInstructions();
    }

    // run program
    LogIt(LogSeverityType::Sev1_Notify, "main", "execution begins");
    if(g_useBytecodeRuntime)
    {
        DoByteCodeProgram(prog);
    }   
    else
    {
        DoProgram(prog);
    }
    
    LogIt(LogSeverityType::Sev1_Notify, "main", "execution finished");

    LogIt(LogSeverityType::Sev1_Notify, "main", "cleanup");
    ProgramDestructor(PROGRAM);

    LogItDebug("method end reached", "main");
    if(g_standalone)
    {
        String endStr;
        std::cout << "Press (ENTER) to quit\n";
        std::getline(std::cin, endStr);
    }

    return 0;
}
