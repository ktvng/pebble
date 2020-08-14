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
    std::cerr << "Usage pebble: [--help] [--custom] [--noisy] [--ast] [program.pebl]" << std::endl;

    exit(2);
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
    }
};

// Logging
LogSeverityType LogAtLevel = LogSeverityType::Sev3_Critical;
bool g_outputOn = false;
bool g_useBytecodeRuntime = true;

int main(int argc, char *argv[])
{
    ParseCommandArgs(argc, argv, &Config);
    
    bool testsFailed = Test();
    if(testsFailed)
        return 1;
    return 0;
}
