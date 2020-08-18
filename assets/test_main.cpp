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

bool Usage(std::vector<SettingOption> options)
{
    std::cerr << "Usage pebble: [--help] [--custom] [--only] [--noisy] [--ast] [--trace] [--keep] [program.pebl]" << std::endl;

    exit(2);
}

bool SettingOnlyRunOneProgram(std::vector<SettingOption> options)
{
    if(options.empty())
    {
        return true;
    }

    g_onlyRunOneProgram = true;
    g_onlyProgramToRun = options[0];

    return true;
}

bool SettingRunCustom(std::vector<SettingOption> options)
{
    g_shouldRunCustomProgram = true;
    return false;
}

bool SettingNoisy(std::vector<SettingOption> options)
{
    g_noisyReport = true;
    return false;
}

bool SettingUseAstRuntime(std::vector<SettingOption> options)
{
    g_useBytecodeRuntime = false;
    return false;
}

bool SettingTraceProgram(std::vector<SettingOption> options)
{
    if(options.empty())
    {
        return true;
    }

    g_onlyRunOneProgram = true;
    g_onlyProgramToRun = options[0];
    g_tracerOn = true;
    LogAtLevel = LogSeverityType::Sev1_Notify;

    return true;
}

bool SettingKeepLog(std::vector<SettingOption> options)
{
    g_keepLog = true;
    return true;
}

ProgramConfiguration Config
{
    {
        "Pebl File", "program.pebl", nullptr
    },
    { 
        "Ignore ./program.pebl", "--custom", SettingRunCustom
    },
    {
        "Report noisy", "--noisy", SettingNoisy
    },
    {
        "Use bytecode runtime", "--ast", SettingUseAstRuntime
    },
    {
        "Run only one program", "--only", SettingOnlyRunOneProgram
    },
    {
        "Run and trace one program", "--trace", SettingTraceProgram
    },
    {
        "Keep log", "--keep", SettingKeepLog
    }
};

bool g_outputOn = false;

int main(int argc, char *argv[])
{
    ParseCommandArgs(argc, argv, &Config);
    
    bool testsFailed = Test();
    if(testsFailed)
        return 1;
    return 0;
}
