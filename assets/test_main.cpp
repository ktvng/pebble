#include <iostream>
#include <string>

#include "main.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"
#include "reference.h"
#include "parse.h"


// Logging
LogSeverityType LogAtLevel = LogSeverityType::Sev3_Critical;
bool g_outputOn = false;

int main(int argc, char *argv[])
{
    for(int i=1; i<argc; i++)
    {
        std::string arg = argv[i];
        if(arg == "--ignore-custom")
            g_shouldRunCustomProgram = false;

        if(arg == "--noisy")
            g_noisyReport = true;
        
    }
    bool testsFailed = Test();
    if(testsFailed)
        return 1;
    return 0;
}
