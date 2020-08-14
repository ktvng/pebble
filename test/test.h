#ifndef __TEST_H
#define __TEST_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "abstract.h"
#include "diagnostics.h"
#include "program.h"
#include "parse.h"
#include "flattener.h"
#include "vm.h"
#include "dis.h"
#include "grammar.h"

extern std::string testBuffer;

extern std::string assertName;
extern std::string failureDescription;
extern std::string testName;
extern std::string expected;

extern std::string programFile;
extern std::string programName;

extern int failedAsserts;
extern int succeededAsserts;


typedef std::vector<const void*> Params;
typedef void (*InjectedFunction)(Params&);

typedef std::string MethodName;
extern std::map<MethodName, InjectedFunction> FunctionInjections; 


extern std::map<MethodName, int> methodHitMap;

extern bool g_shouldRunCustomProgram;
extern bool g_noisyReport;

extern Program* programToRun;
extern bool g_useBytecodeRuntime;

bool Test();
void ResetRun();

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

inline void ResetAssert()
{
    assertName = "N/A";
    failureDescription = "N/A";
    expected = "N/A";
}

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

std::string Diff();

inline void Expected(std::string& msg)
{
    expected = msg;
}

inline void Expected(const char* msg)
{
    expected = std::string(msg);
}

/// assert that [b] is true. logs error if this is not the case
void Assert(bool b);


/// name of test
inline void ItTests(const std::string& name)
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
    programToRun = ParseProgram(programFile);
}

/// execute the program
inline void Execute()
{
    ResetAssert();

    if(!FatalCompileError)
    {
        if(g_useBytecodeRuntime)
        {
            FlattenProgram(programToRun);
            DoByteCodeProgram(programToRun);
            ProgramDestructor(programToRun);
        }
        else
        {
            // DoProgram(programToRun);
            ProgramDestructor(programToRun);
        }

        Valgrind();
        TestConstantsFidelity();
    }
}

/// returns the number of calls to [methodName]
inline int NumberOfCallsTo(const std::string& methodName)
{
    return methodHitMap[methodName];
}

inline void SetProgramToRun(const std::string& fileName)
{
    programName = fileName;
    programFile = "./test/programs/" + fileName + ".pebl";
    if(g_noisyReport)
    {        
        std::cout << CONSOLE_MAGENTA << "\n" << fileName << "\n  >> " << CONSOLE_RESET;
    }
}

inline void RunCustomProgram()
{
    programFile = "./program.pebl";
    if(g_noisyReport)
    {        
        std::cout << CONSOLE_MAGENTA << "\nstarting: CustomProgram" << CONSOLE_RESET;
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
