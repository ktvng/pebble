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

Program* programToRun = nullptr;

int failedAsserts = 0;
int succeededAsserts = 0;

bool g_shouldRunCustomProgram = true;
bool g_noisyReport = false;

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
        i+= entry.References.size();
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

void TestGenericMemoryLoss(String typeName)
{
    Should("not lose any " + typeName +"s in memory");
    
    int created = NumberOfCallsTo(typeName + "Constructor");
    int destroyed = NumberOfCallsTo(typeName + "Destructor");

    OtherwiseReport(Msg("created (%i) != destroyed (%i)", created, destroyed));
    Assert(created == destroyed);    
}

void TestNoProgramMessages()
{
    Should("not throw system messages");

    OtherwiseReport("reported messages:\n" + ProgramMsgs);
    Assert(ProgramMsgs == "");
}

void Valgrind()
{
    std::vector<String> objectsToTest = 
    {
        "Object",
        "Reference",
        "Scope",
        "Method",
        "Token",
        "Block",
        "Operation",
        "Program",
        "ParseToken",
        "ObjectValue",
        "ReferenceStub"
    };

    for(auto str: objectsToTest)
    {
        TestGenericMemoryLoss(str);
    }
}

/// tests standard things
void IncludeStandardAssertSuite()
{
    Valgrind();
    TestNoProgramMessages();
}








// ---------------------------------------------------------------------------------------------------------------------
// Test execution

void DoAllTests()
{
    for(auto test: Tests)
    {
        if(g_noisyReport)
        {
            SetConsoleColor(ConsoleColor::Purple2);
            std::cout << "\n finished " << testName;
            SetConsoleColor(ConsoleColor::White);
        }
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