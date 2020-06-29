#include <limits>

#include "test.h"
#include "main.h"
#include "arch.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"
#include "reference.h"


// ---------------------------------------------------------------------------------------------------------------------
// Global variables

std::map<std::string, int> methodHitMap;
std::map<MethodName, InjectedFunction> FunctionInjections;

std::string testBuffer;

std::string assertName = "*unspecified*";
std::string failureDescription = "*unspecified*";
std::string testName = "*unspecified*";

std::string programFile = "./program";


int failedAsserts = 0;
int succeededAsserts = 0;


// ---------------------------------------------------------------------------------------------------------------------
// Test helpers

int ObjectIndexSize(Program* p)
{
    return p->ObjectsIndex.size();
}

int ReferencesInIndex(Program* p)
{
    int i=0;
    for(auto entry: p->ObjectsIndex)
    {
        i+= entry->References.size();
    }
    return i;
}

void ResetRun()
{
    std::map<std::string, int>::iterator it;
    for(it=methodHitMap.begin(); it!=methodHitMap.end(); it++)
    {
        methodHitMap[it->first] = 0;
    }
    
    FunctionInjections.clear();
}


// ---------------------------------------------------------------------------------------------------------------------
// Test primitives

/// inject a function [func] to evaluate before method [name] is called
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
    CompileGrammar();
    ParseProgram(programFile);
}

/// execute the program
inline void Execute()
{
    DoProgram(*PROGRAM);
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

inline void CompileAndExecuteProgram(const std::string& programName)
{
    SetProgramToRun(programName);
    DisableLogging();
    Compile();
    Execute();
}

/// tests that all objects are accessible
void TestObjectMemoryLoss()
{
    Should("not lose any objects in memory");

    // add 1 b/c null object is not created through constructor
    int createdObjs = NumberOfCallsTo("ObjectConstructor") + 1;
    int objsInIndex = ObjectIndexSize(PROGRAM);

    OtherwiseReport(Msg("created objects %i != objects in index %i", createdObjs, objsInIndex));
    Assert(createdObjs == objsInIndex);
}

/// tests that all references are accessible
void TestReferenceMemoryLoss()
{
    Should("not lose any references in memory");

    int createdRefs = NumberOfCallsTo("ReferenceConstructor");
    int destroyedRefs = NumberOfCallsTo("Dereference");
    int accessibleRefs = ReferencesInIndex(PROGRAM);
    int methods = NumberOfCallsTo("MethodConstructor");

    OtherwiseReport(Msg("created refs != destroyed + accessible + methods"));
    Assert(createdRefs == destroyedRefs + accessibleRefs + methods);
}

/// tests standard things
void IncludeStandardAssertSuite()
{
    TestObjectMemoryLoss();
    TestReferenceMemoryLoss();
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

/// assert that [b] is true. logs error if this is not the case
void Assert(bool b)
{
    if(!b)
    {
        failedAsserts++;
        testBuffer.append("(" + std::to_string(failedAsserts) + "):\n");
        testBuffer.append(testName + "\n");
        testBuffer.append("  failed assertion: (should) " + assertName + "\n");
        testBuffer.append("    > " + failureDescription + "\n\n");
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



// ---------------------------------------------------------------------------------------------------------------------
// Tests

void testFuncInject(Params& p)
{
    const Reference* ref = *static_cast<const Reference* const*>(p.at(0));
    std::cout << "first object is of type: " << ObjectOf(ref)->Class << std::endl;
}

void TestCustomProgram()
{
    It("Custom Program executes");
    programFile = "./program";
    // InjectBefore("OperationAdd", testFuncInject);

    ConfigureLogging(LogSeverityType::Sev3_Critical, true);
    Compile();
    Execute();

    IncludeStandardAssertSuite();
}

void TestIf()
{
    It("evaluates an if statement");
    
    SetProgramToRun("TestIf");
    ConfigureLogging(LogSeverityType::Sev3_Critical, true);
    Compile();
    Execute();

    IncludeStandardAssertSuite();

    Should("conditionally execute code by evaluating if-expression");
    OtherwiseReport("unknown failure condition");
    Assert(ProgramOutput == "INIF\n");
}

void TestOrderOfOperations()
{
    It("evaluates order of operations");

    SetProgramToRun("TestOrderOfOperations");
    DisableLogging();
    Compile();
    Execute();

    IncludeStandardAssertSuite();
    
    String correctOutput = "28\n6\n7\n-5\n-10\n8\n";

    Should("execute operations in PEMDAS order");
    OtherwiseReport("diff\ngot:\n" + ProgramOutput + "\nexpected:\n" + correctOutput);
    Assert(ProgramOutput == correctOutput );
}

void TestMethodWithNoParams()
{
    It("can define/evaluate method with no params");

    CompileAndExecuteProgram("TestMethodWithNoParams");

    IncludeStandardAssertSuite();

    String correctOutput = "48\n60\n";

    Should("define and execute method with no params");
    OtherwiseReport("diff\ngot:\n" + ProgramOutput + "\nexpected:\n" + correctOutput);
    Assert(ProgramOutput == correctOutput );
}



// ---------------------------------------------------------------------------------------------------------------------
// Test index

typedef void (*TestFunction)();
TestFunction Tests[] = 
{
    TestCustomProgram,
    TestIf,
    TestOrderOfOperations,
    TestMethodWithNoParams,
};



// ---------------------------------------------------------------------------------------------------------------------
// Test execution

void DoAllTests()
{
    for(auto test: Tests)
    {
        testName = "*unspecified*";
        test();
    }
}

void Test()
{
    std::cout.precision(2);

    testBuffer.reserve(2048);
    SetConsoleColor(ConsoleColor::Yellow);
    std::cout << "starting...\n";
    SetConsoleColor(ConsoleColor::White);

    DoAllTests();

    SetConsoleColor(ConsoleColor::Yellow);
    std::cout << "\nfinished " << (succeededAsserts + failedAsserts) << " tests at " 
        << std::fixed << (100.0 * succeededAsserts / (succeededAsserts + failedAsserts)) 
        << " %" << std::endl;

    SetConsoleColor(ConsoleColor::Green);
    std::cout << "  >> passed: " << succeededAsserts << std::endl;
    SetConsoleColor(ConsoleColor::Red);
    std::cout << "  >> failed: " << failedAsserts << std::endl << std::endl;


    if(failedAsserts)
    {
        SetConsoleColor(ConsoleColor::Yellow);
        std::cout << "failure report: \n";
        SetConsoleColor(ConsoleColor::Red);
        std::cout << testBuffer;
    }
    SetConsoleColor(ConsoleColor::White);
}