#include <limits>

#include "test.h"
#include "unittests.h"

#include "bytecode.h"
#include "call.h"
#include "vm.h"

// ---------------------------------------------------------------------------------------------------------------------
// Global variables

std::map<std::string, int> methodHitMap;
std::map<MethodName, InjectedFunction> FunctionInjections;

std::string testBuffer;

std::string assertName = "N/A";
std::string failureDescription = "N/A";
std::string testName = "N/A";
std::string expected = "N/A";

std::string programFile = "./program";
std::string programName = "program";

Program* programToRun = nullptr;

int failedAsserts = 0;
int succeededAsserts = 0;

bool g_shouldRunCustomProgram = true;
bool g_noisyReport = false;

// ---------------------------------------------------------------------------------------------------------------------
// Program output wrapper

const std::string FatalExceptionString = "(!) Fatal Exception";
const std::string NonFatalExceptionString = "(!) Exception";

bool ProgramResult::Equals(std::string msg)
{
    return ProgramOutput == msg;
}

bool ProgramResult::Contains(std::string msg)
{
    return ProgramOutput.find(msg) != std::string::npos;
}

bool ProgramResult::EncounteredFatalException()
{
    return ProgramOutput.find(FatalExceptionString) != std::string::npos;
}

bool ProgramResult::EncounteredNonFatalException()
{
    return ProgramOutput.find(NonFatalExceptionString) != std::string::npos;
}

bool ProgramResult::AsExpected()
{
    return ProgramOutput == expected;
}

std::string ProgramResult::Output()
{
    return ProgramOutput;
}

bool NotResult::Equals(std::string msg)
{
    return ProgramOutput != msg;
}

bool NotResult::Contains(std::string msg)
{
    return ProgramOutput.find(msg) == std::string::npos;
}

bool NotResult::EncounteredFatalException()
{
    return ProgramOutput.find(FatalExceptionString) == std::string::npos;
}

bool NotResult::EncounteredNonFatalException()
{
    return ProgramOutput.find(NonFatalExceptionString) == std::string::npos;
}

bool NotResult::AsExpected()
{
    return ProgramOutput != expected;
}


ProgramResult Result;


// ---------------------------------------------------------------------------------------------------------------------
// Test helpers

void ResetRun()
{
    std::map<std::string, int>::iterator it;
    for(it=methodHitMap.begin(); it!=methodHitMap.end(); it++)
    {
        methodHitMap[it->first] = 0;
    }
    
    FunctionInjections.clear();

    ResetAssert();
    testName = "N/A";
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

inline void ReportFailedAssert()
{
    failedAsserts++;
    SetConsoleColor(ConsoleColor::Red);
    std::cout << ".";
    SetConsoleColor(ConsoleColor::White);
}

inline void ReportSucceededAssert()
{
    succeededAsserts++;
    SetConsoleColor(ConsoleColor::Green);
    std::cout << ".";
    SetConsoleColor(ConsoleColor::White);
}

int DigitsOfInt(int i)
{
    if(i < 0)
    {
        LogIt(LogSeverityType::Sev1_Notify, "DigitsOfInt", "not defined for negative integers");
        return 2;
    }

    int digits = 1;
    int base = 100;

    while(i / base != 0)
    {
        base *= 10;
        digits += 1;
    }

    return digits;
}

std::string SpacesOfLength(int n)
{
    std::string spaces;
    spaces.reserve(n);
    for(int i=0; i<n; i++)
    {
        spaces += " ";
    }

    return spaces;
}

std::string Repeat(char c, int n)
{
    std::string repeated;
    repeated.reserve(n);
    for(int i=0; i<n; i++)
    {
        repeated += c;
    }

    return repeated;
}


const String c_testrightEdge = "| ";

std::string AddTestRightEdge(String str)
{
    String formattedStr = c_testrightEdge;
    formattedStr.reserve(str.size());
    for(size_t i=0; i<str.size(); i++)
    {
        formattedStr += str.at(i);
        if(str.at(i) == '\n')
            formattedStr += c_testrightEdge;
    }
    return formattedStr;
}


std::string Diff()
{
    std::string diff;
    diff.reserve(64);
    diff = "difference between result and expected";
    diff += "\n\nexpected   " + IndentStringToLevel(AddTestRightEdge(expected), 5, 1);
    diff += "\n\n     got   " + IndentStringToLevel(AddTestRightEdge(ProgramOutput), 5, 1);

    return diff;
}



void Assert(bool b)
{
    if(!b)
    {
        if(failureDescription == "N/A")
        {
            failureDescription = "default to report " + Diff();
        }

        std::string padding = "    ";
        padding += SpacesOfLength(DigitsOfInt(failedAsserts + 1));

        testBuffer.append("  " + std::to_string(failedAsserts+1) + ") " + programName + "\n\n");
        testBuffer.append(padding + "failed: it tests " + testName + "\n");
        testBuffer.append(padding + "assert: should " + assertName + "\n");
        testBuffer.append(padding + "report: " + IndentStringToLevel(failureDescription, 3) + "\n\n");

        ReportFailedAssert();
    }
    else
    {
        ReportSucceededAssert();
    }

    ResetAssert();
}


void TestConstantsFidelity()
{
    Should("not modify IntegerCall");
    OtherwiseReport("IntegerCall modified at some point during test");
    Assert(IntegerCall.BoundScope == &SomethingScope
        && Strictly(&AbstractIntegerType, &IntegerCall));
    
    Should("not modify DecimalCall");
    OtherwiseReport("DecimalCall modified at some point during test");
    Assert(DecimalCall.BoundScope == &SomethingScope
        && Strictly(&AbstractDecimalType, &DecimalCall));

    Should("not modify StringCall");
    OtherwiseReport("StringCall modified at some point during test");
    Assert(StringCall.BoundScope == &SomethingScope
        && Strictly(&AbstractStringType, &StringCall));

    Should("not modify BooleanCall");
    OtherwiseReport("BooleanCall modified at some point during test");
    Assert(BooleanCall.BoundScope == &SomethingScope
        && Strictly(&AbstractBooleanType, &BooleanCall));

    Should("not modify ObjectCall");
    OtherwiseReport("ObjectCall modified at some point during test");
    Assert(ObjectCall.BoundScope == &SomethingScope
        && Strictly(&AbstractObjectType, &ObjectCall));

    Should("not modify ArrayCall");
    OtherwiseReport("ArrayCall modified at some point during test");
    Assert(ArrayCall.BoundScope == &SomethingScope
        && Strictly(&AbstractArrayType, &ArrayCall));

    Should("not modify NothingCall");
    OtherwiseReport("NothingCall modified at some point during test");
    Assert(IsNothing(&NothingCall) && IsPureNothing(&NothingCall));
}

void Valgrind()
{
    static std::vector<String> objectsToTest = 
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
        "ReferenceStub",
        "String",
    };

    for(auto str: objectsToTest)
    {
        TestGenericMemoryLoss(str);
    }
}





// ---------------------------------------------------------------------------------------------------------------------
// Test execution

void DoAllTests()
{
    for(auto test: Tests)
    {
        testName = "N/A";
        ResetAssert();
        test();
    }
}

bool Test()
{
    std::cout.precision(2);
    std::string sectionDivider = Repeat('#', 60) + "\n";
    testBuffer.reserve(4096);
    SetConsoleColor(ConsoleColor::Yellow);
    std::cout << sectionDivider << sectionDivider;
    std::cout << "STARTING...\n";
    SetConsoleColor(ConsoleColor::White);

    DoAllTests();

    SetConsoleColor(ConsoleColor::Yellow);
    std::cout << "\n\nfinished " << (succeededAsserts + failedAsserts) << " tests at " 
        << std::fixed << (100.0 * succeededAsserts / (succeededAsserts + failedAsserts)) 
        << " %" << std::endl;

    SetConsoleColor(ConsoleColor::Green);
    std::cout << "  >> passed: " << succeededAsserts << std::endl;
    SetConsoleColor(ConsoleColor::Red);
    std::cout << "  >> failed: " << failedAsserts << std::endl << std::endl;


    if(failedAsserts)
    {
        SetConsoleColor(ConsoleColor::Yellow);
        std::cout << "failure report: \n\n";
        SetConsoleColor(ConsoleColor::Red);
        std::cout << testBuffer;
    }
    SetConsoleColor(ConsoleColor::White);
    if(failedAsserts) 
        return 1;
    
    return 0;
}