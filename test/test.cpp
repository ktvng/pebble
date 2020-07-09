#include <limits>

#include "test.h"
#include "unittests.h"

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

bool g_shouldRunCustomProgram = true;


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


/// tests that all objects are accessible
void TestObjectMemoryLoss()
{
    Should("not lose any objects in memory");

    int createdObjs = NumberOfCallsTo("ObjectConstructor");
    int destroyedObjs = NumberOfCallsTo("ObjectDestructor");

    OtherwiseReport(Msg("created objects (%i) != destroyed objects (%i)", createdObjs, destroyedObjs));
    Assert(createdObjs == destroyedObjs);
}

void TestMethodMemoryLoss()
{
    Should("not lose any objects in memory");

    int createdMethods = NumberOfCallsTo("MethodConstructor");
    int destroyedMethods = NumberOfCallsTo("MethodDestructor");

    OtherwiseReport(Msg("createdMethods (%i) != destroyedMethods (%i)", createdMethods, destroyedMethods));
    Assert(createdMethods == destroyedMethods);
}

/// tests that all references are accessible
void TestReferenceMemoryLoss()
{
    Should("not lose any references in memory");

    int createdRefs = NumberOfCallsTo("ReferenceConstructor");
    int destroyedRefs = NumberOfCallsTo("ReferenceDestructor");
    int accessibleRefs = ReferencesInIndex(PROGRAM);
    int methods = NumberOfCallsTo("MethodConstructor");

    OtherwiseReport(Msg("created refs (%i) != destroyed (%i) + accessible (%i) + methods (%i)", createdRefs, destroyedRefs, accessibleRefs, methods));
    Assert(createdRefs == destroyedRefs + accessibleRefs + methods);
}

void TestNoProgramMessages()
{
    Should("not throw system messages");

    OtherwiseReport("reported messages:\n" + ProgramMsgs);
    Assert(ProgramMsgs == "");
}

/// tests standard things
void IncludeStandardAssertSuite()
{
    TestObjectMemoryLoss();
    // TestReferenceMemoryLoss();
    TestNoProgramMessages();
}








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

bool Test()
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
    if(failedAsserts) 
        return 1;
    
    return 0;
}