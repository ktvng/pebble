#include <iostream>

#include "main.h"
#include "arch.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"
#include "reference.h"


// Logging
LogSeverityType LogAtLevel = LogSeverityType::Sev3_Critical;

int main()
{

    PurgeLog();                         // cleans log between each run
    CompileGrammar();                   // compile grammar from grammar.txt

    ParseProgram(".\\program");
    DoProgram(*PROGRAM);
    return 0;
}
