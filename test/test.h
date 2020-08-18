#ifndef __TEST_H
#define __TEST_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "abstract.h"
#include "consolecolor.h"

// ---------------------------------------------------------------------------------------------------------------------
// Unit test architecture

inline const char* const VoidName = "N/A";

struct UnitTest
{
    std::string TestName;
    std::string AssertName;
    std::string ExpectedClause;
    std::string AssertType;
    std::string OtherwiseReport;

    std::string ProgramFile;
    std::string ProgramName;
    std::string ProgramOutput;
    int ProgramReturnCode;

    bool HasBeenCompiled;
    bool HasBeenRun;
    
    bool EncounteredCompiletimeError;
    bool EncounteredRuntimeError;

    Program* ProgramToRun;
};

extern UnitTest test;

void ClearTest();
void ClearRun();
void ClearAssert();
void ClearRunMetrics();

extern std::string testBuffer;

extern int failedAsserts;
extern int succeededAsserts;


typedef std::vector<const void*> Params;
typedef void (*InjectedFunction)(Params&);

typedef std::string MethodName;
extern std::map<MethodName, InjectedFunction> FunctionInjections; 


void TEST_Tracer(const char* str);
extern std::map<MethodName, int> methodHitMap;

extern bool g_shouldRunCustomProgram;
extern bool g_noisyReport;
extern bool g_onlyRunOneProgram;
extern bool g_useBytecodeRuntime;
extern bool g_tracerOn;
extern bool g_keepLog;

extern std::string g_onlyProgramToRun;

bool Test();

// ---------------------------------------------------------------------------------------------------------------------
// Program output wrapper

class NotResult
{
    public:
    bool Equals(std::string msg);
    bool Contains(std::string msg);
    bool EncounteredFatalException();
    bool EncounteredNonFatalException();
    bool AsExpected();
};

class ProgramResult
{
    public:
    bool Equals(std::string msg);
    bool Contains(std::string msg);
    bool EncounteredFatalException();
    bool EncounteredNonFatalException();
    bool AsExpected();
    std::string Output();
    NotResult Not;
};


extern ProgramResult Result;
void Valgrind();
void TestConstantsFidelity();


// ---------------------------------------------------------------------------------------------------------------------
// Test fundementals

/// describes the current assertion. MUST BE CALLED BEFORE EACH UNIQUE ASSERTION
inline void Should(const std::string& descriptionOfTest)
{
    test.AssertName = descriptionOfTest;
}

inline void OtherwiseReport(const std::string& descriptionOfFailureCase)
{
    test.OtherwiseReport = descriptionOfFailureCase;
}

std::string Diff();

inline void Expected(std::string& msg)
{
    test.ExpectedClause = msg;
}

inline void Expected(const char* msg)
{
    test.ExpectedClause = std::string(msg);
}

/// assert that [b] is true. logs error if this is not the case
void Assert(bool b);


/// name of test
inline void ItTests(const std::string& name)
{
    test.TestName = name;
}

inline void InjectBefore(MethodName name, InjectedFunction func)
{
    FunctionInjections[name] = func;
}

/// configure the logging properties for speed and optimization
void ConfigureLogging(LogSeverityType level, bool clearBefore);

inline void DisableLogging()
{
    ConfigureLogging(LogSeverityType::Sev3_Critical, true);
}

/// compiles the program
void Compile();

/// execute the program
void Execute();

/// returns the number of calls to [methodName]
inline int NumberOfCallsTo(const std::string& methodName)
{
    return methodHitMap[methodName];
}

inline void SetProgramToRun(const std::string& fileName)
{
    test.ProgramName = fileName;
    test.ProgramFile = "./test/programs/" + fileName + ".pebl";
    if(g_noisyReport)
    {        
        std::cout << CONSOLE_CYAN << "\n\n  - " << fileName << "\n      > " << CONSOLE_RESET;
    }
}

inline void RunCustomProgram()
{
    test.ProgramName = "CustomProgram";
    test.ProgramFile = "./program.pebl";
    if(g_noisyReport)
    {        
        std::cout << CONSOLE_CYAN << " - CustomProgram\n      > " << CONSOLE_RESET;
    }
}

inline void CompileAndExecuteProgram(const std::string& programName)
{
    SetProgramToRun(programName);
    DisableLogging();
    Compile();
    Execute();
}

void TestGenericMemoryLoss(String className);

#endif
