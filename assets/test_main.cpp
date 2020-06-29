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
bool g_outputOn = false;

int main()
{
    Test();
}
