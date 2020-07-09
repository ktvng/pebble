#ifndef __TEST_H
#define __TEST_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "abstract.h"
#include "diagnostics.h"
#include "program.h"
#include "execute.h"
#include "parse.h"


extern std::string testBuffer;

extern std::string assertName;
extern std::string failureDescription;
extern std::string testName;

extern std::string programFile;

extern int failedAsserts;
extern int succeededAsserts;


typedef std::vector<const void*> Params;
typedef void (*InjectedFunction)(Params&);

typedef std::string MethodName;
extern std::map<MethodName, InjectedFunction> FunctionInjections; 


extern std::map<MethodName, int> methodHitMap;

extern bool g_shouldRunCustomProgram;
extern bool g_noisyReport;

bool Test();
void ResetRun();

// ---------------------------------------------------------------------------------------------------------------------
// Test fundementals

/// describes the current assertion. MUST BE CALLED BEFORE EACH UNIQUE ASSERTION
inline void Should(const std::string& descriptionOfTest)
{
    assertName = descriptionOfTest;
}

inline void OtherwiseReport(const std::string& descriptionOfFailureCase)
{
    failureDescription = descriptionOfFailureCase;
}

/// assert that [b] is true. logs error if this is not the case
inline void Assert(bool b)
{
    if(!b)
    {
        failedAsserts++;
        testBuffer.append("(" + std::to_string(failedAsserts) + "):\n");
        testBuffer.append(testName + "\n");
        testBuffer.append("  failed assertion: (should) " + assertName + "\n");
        testBuffer.append("    > " + IndentStringToLevel(failureDescription, 3) + "\n\n");
        SetConsoleColor(ConsoleColor::Red);
        std::cout << ".";
        SetConsoleColor(ConsoleColor::White);
    }
    else
    {
        succeededAsserts++;
        SetConsoleColor(ConsoleColor::Green);
        std::cout << ".";
        SetConsoleColor(ConsoleColor::White);
    }

    assertName = "*unspecified*";
    failureDescription = "*unspecified*";
}

/// name of test
inline void It(const std::string& name)
{
    ResetRun();
    testName = name;
}








inline void InjectBefore(MethodName name, InjectedFunction func)
{
    FunctionInjections[name] = func;
}

/// configure the logging properties for speed and optimization
inline void ConfigureLogging(LogSeverityType level, bool clearBefore)
{
    if(clearBefore)
        PurgeLog();

    LogAtLevel = level;
}

inline void DisableLogging()
{
    ConfigureLogging(LogSeverityType::Sev3_Critical, true);
}

/// compiles the program
inline void Compile()
{
    ProgramOutput = "";
    ProgramMsgs = "";
    FatalCompileError = false;
    CompileGrammar();
    ParseProgram(programFile);
}

/// execute the program
inline void Execute()
{
    if(!FatalCompileError)
    {
        DoProgram(*PROGRAM);
        ProgramDestructor(PROGRAM);
    }
}

/// returns the number of calls to [methodName]
inline int NumberOfCallsTo(const std::string& methodName)
{
    return methodHitMap[methodName];
}

inline void SetProgramToRun(const std::string& fileName)
{
    programFile = "./test/programs/" + fileName;
}

inline void RunCustomProgram()
{
    programFile = "./program.pebl";
}

inline void CompileAndExecuteProgram(const std::string& programName)
{
    SetProgramToRun(programName);
    DisableLogging();
    Compile();
    Execute();
}

void TestGenericMemoryLoss(String className);
void IncludeStandardAssertSuite();


#endif
