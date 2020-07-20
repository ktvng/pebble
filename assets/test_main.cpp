#include <iostream>
#include <string>

#include "main.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"
#include "reference.h"
#include "parse.h"
#include "commandargs.h"

void SettingIgnoreCustom(std::vector<SettingOption> options)
{
    g_shouldRunCustomProgram = false;
}

void SettingNoisy(std::vector<SettingOption> options)
{
    g_noisyReport = true;
}

void SettingUseBytecodeRuntime(std::vector<SettingOption> options)
{
    g_useBytecodeRuntime = true;
}

ProgramConfiguration Config
{
    { 
        "Ignore ./program.pebl", "--ignore-custom", SettingIgnoreCustom
    },
    {
        "Report noisy", "--noisy", SettingNoisy
    },
    {
        "Use bytecode runtime", "--bytecode", SettingUseBytecodeRuntime
    }
};

// Logging
LogSeverityType LogAtLevel = LogSeverityType::Sev3_Critical;
bool g_outputOn = false;
bool g_useBytecodeRuntime = true;

int main(int argc, char *argv[])
{
    ParseCommandArgs(argc, argv, Config);
    
    bool testsFailed = Test();
    if(testsFailed)
        return 1;
    return 0;
}
