#include <sstream>
#include <chrono>
#include <algorithm>

#include "test.h"
#include "consolecolor.h"
#include "unittests.h"

#include "program.h"
#include "diagnostics.h"
#include "bytecode.h"
#include "call.h"
#include "vm.h"
#include "grammar.h"
#include "parse.h"
#include "flattener.h"

// ---------------------------------------------------------------------------------------------------------------------
// Documentation
/*

Overview

    The Pebble testbuild is a special build with method decorators that
    incorporate additional features such as method-hit tracking, custom function
    injection, and tracing log statements. By default these features are not
    used, but can be enabled via different flags

FLags

    [--only NAME]       Only runs the test program with the specified 'NAME'. 
                        This program will be located along path
                        `./test/programs/`NAME`.pebl` and only assertions for
                        this program will be evaluated.

    [--custom]          Run the Pebble program along path `./program.pebl` which
                        can contain custom user defined input. Will also run
                        the MetaFailureTest which is an assertion that always
                        fails.

    [--noisy]           Will print out each program name as it is being run
                        which is useful for identifying failures when a segfault
                        occurs.

    [--ast]             Use the abstract syntax tree recursive walker runtime
                        which is currently disabled (don't use this option)

    [--trace NAME]      Only runs the program with the specified 'NAME' (see
                        the [--only] flag). In addition, enables log trace 
                        statements which are written to `./logs/log`
*/

// ---------------------------------------------------------------------------------------------------------------------
// Global variables

std::map<std::string, int> methodHitMap;
std::map<MethodName, InjectedFunction> FunctionInjections;

std::string testBuffer;

static double timespan;

int failedAsserts = 0;
int succeededAsserts = 0;

bool g_shouldRunCustomProgram = false;
bool g_noisyReport = false;
bool g_onlyRunOneProgram = false;
bool g_useBytecodeRuntime = true;
bool g_tracerOn = false;

std::string g_onlyProgramToRun;

LogSeverityType LogAtLevel = LogSeverityType::Sev3_Critical;


// ---------------------------------------------------------------------------------------------------------------------
// Program output wrapper

const std::string FatalExceptionString = "Fatal Exception";
const std::string NonFatalExceptionString = "Exception";

bool ProgramResult::Equals(std::string msg)
{
    return test.ProgramOutput == msg;
}

bool ProgramResult::Contains(std::string msg)
{
    test.ExpectedClause = msg;
    return test.ProgramOutput.find(msg) != std::string::npos;
}

bool ProgramResult::EncounteredFatalException()
{
    return test.ProgramOutput.find(FatalExceptionString) != std::string::npos;
}

bool ProgramResult::EncounteredNonFatalException()
{
    return test.ProgramOutput.find(NonFatalExceptionString) != std::string::npos
        && test.ProgramOutput.find(FatalExceptionString) == std::string::npos;
}

bool ProgramResult::AsExpected()
{
    return test.ProgramOutput == test.ExpectedClause;
}

std::string ProgramResult::Output()
{
    return test.ProgramOutput;
}

bool NotResult::Equals(std::string msg)
{
    test.ExpectedClause = msg;
    return test.ProgramOutput != msg;
}

bool NotResult::Contains(std::string msg)
{
    test.ExpectedClause = msg;
    return test.ProgramOutput.find(msg) == std::string::npos;
}

bool NotResult::EncounteredFatalException()
{
    return test.ProgramOutput.find(FatalExceptionString) == std::string::npos;
}

bool NotResult::EncounteredNonFatalException()
{
    return test.ProgramOutput.find(NonFatalExceptionString) == std::string::npos
        && test.ProgramOutput.find(FatalExceptionString) != std::string::npos;
}

bool NotResult::AsExpected()
{
    return test.ProgramOutput != test.ExpectedClause;
}


ProgramResult Result;



// ---------------------------------------------------------------------------------------------------------------------
// Tracing

std::vector<std::string> TracedMethods = 
{
    // grammar.cpp
    "CompileGrammar",
    
    // parse.cpp
    "ExpressionParser",
    "Preprocess",
    "TryReversingGrammarRules",

    // program.cpp
    "ParseBlock",
    "ParseProgram",

    // bytecode.cpp
    "BCI_LoadCallName",
    "BCI_LoadPrimitive",
    "BCI_Assign",
    "BCI_Add",
    "BCI_Subtract",

    "BCI_Multiply",
    "BCI_Divide",
    "BCI_SysCall",
    "BCI_And",
    "BCI_Or",

    "BCI_Not",
    "BCI_NotEquals",
    "BCI_Equals",
    "BCI_Cmp",
    "BCI_LoadCmp",

    "BCI_JumpFalse",
    "BCI_Jump",
    "BCI_Copy",
    "BCI_BindType",
    "BCI_ResolveDirect",

    "BCI_ResolveScoped",
    "BCI_BindScope",
    "BCI_BindSection",
    "BCI_EvalHere"
    "BCI_Eval",

    "BCI_Return",
    "BCI_Array",
    "BCI_EnterLocal",
    "BCI_LeaveLocal",
    "BCI_Extend",

    "BCI_NOP",
    "BCI_Dup",
    "BCI_Endline",
    "BCI_DropTOS",
    "BCI_Is"
};

void TEST_Tracer(const char* str)
{
    if(!g_tracerOn)
    {
        return;
    }

    std::string niceStr(str);
    if(std::find(TracedMethods.begin(), TracedMethods.end(), niceStr) != TracedMethods.end())
    {
        LogIt(LogSeverityType::Sev1_Notify, str, "");
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Test helpers

bool ShouldRunProgram()
{
    if(!g_onlyRunOneProgram)
    {
        return true;
    }

    return (g_onlyRunOneProgram && test.ProgramName == g_onlyProgramToRun);
}

void ClearRunMetrics()
{
    std::map<std::string, int>::iterator it;
    for(it=methodHitMap.begin(); it!=methodHitMap.end(); it++)
    {
        methodHitMap[it->first] = 0;
    }
    
    FunctionInjections.clear();
}

void ConfigureLogging(LogSeverityType level, bool clearBefore)
{
    if(g_tracerOn)
    {
        LogAtLevel = LogSeverityType::Sev1_Notify;
        return;
    }

    if(clearBefore)
        PurgeLog();

    LogAtLevel = level;
}

void Compile()
{
    if(!ShouldRunProgram())
    {
        return;
    }

    ClearRunMetrics();

    ProgramOutput = "";
    ProgramMsgs = "";
    CompileGrammar();
    test.ProgramToRun = ParseProgram(test.ProgramFile);
    test.HasBeenCompiled = true;
    test.EncounteredCompiletimeError = FatalCompileError;
}

void Execute()
{
    if(!ShouldRunProgram())
    {
        return;
    }

    if(test.ProgramToRun == nullptr)
    {
        LogIt(LogSeverityType::Sev3_Critical, "Execute", 
            "no program is able to be run. make sure to compile beforehand.");
        return;
    }

    test.ProgramReturnCode = 0;
    
    if(!test.EncounteredCompiletimeError)
    {
        if(g_useBytecodeRuntime)
        {
            FlattenProgram(test.ProgramToRun);
            test.ProgramReturnCode = DoByteCodeProgram(test.ProgramToRun);
            test.ProgramOutput = ProgramOutput;
            ProgramDestructor(test.ProgramToRun);
        }
        else
        {
            LogIt(LogSeverityType::Sev3_Critical, "Execute", "ast runtime not available");
            // DoProgram(programToRun);
        }

        test.HasBeenRun = true;
        test.EncounteredRuntimeError = test.ProgramReturnCode;

        Valgrind();
        TestConstantsFidelity();
    }
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
    std::cout << CONSOLE_RED << "." << CONSOLE_RESET;
}

inline void ReportSucceededAssert()
{
    succeededAsserts++;
    std::cout << CONSOLE_GREEN << "." << CONSOLE_RESET;
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
    diff.reserve(256);
    diff = "difference between result and expected";
    diff += "\n\nexpected   " + IndentStringToLevel(AddTestRightEdge(test.ExpectedClause), 5, 1);
    diff += "\n\n     got   " + IndentStringToLevel(AddTestRightEdge(test.ProgramOutput), 5, 1);

    return diff;
}

inline void AddHeader()
{
    testBuffer.append("  " + std::to_string(failedAsserts+1) + ") " + test.ProgramName + "\n\n");
}

inline void AddTestNameField(std::string& padding)
{
    std::string testName;
    if(test.TestName.empty())
    {
        testName = "failed: N/A";
    }
    else
    {
        testName = "failed: it tests " + test.TestName;
    }
    testBuffer.append(padding + testName + "\n");
}

inline void AddShouldField(std::string& padding)
{
    std::string assert;
    if(test.AssertName.empty())
    {
        assert = "assert: N/A";
    }
    else
    {
        assert = "assert: should " + test.AssertName;
    }

    testBuffer.append(padding + assert + "\n");
}

inline void AddExpectedField(std::string& padding)
{
    std::string otherwiseReport;
    if(test.OtherwiseReport.empty())
    {
        otherwiseReport = "default is " + Diff();
    }
    else
    {
        otherwiseReport = test.OtherwiseReport;
    }

    testBuffer.append(padding + "report: " + IndentStringToLevel(otherwiseReport, 3) + "\n\n");
}

void AddFailedTestToBuffer()
{
    std::string padding = "    ";
    padding += SpacesOfLength(DigitsOfInt(failedAsserts + 1));

    AddHeader();
    AddTestNameField(padding);
    AddShouldField(padding);
    AddExpectedField(padding);

}

void Assert(bool b)
{
    if(!ShouldRunProgram())
    {
        return;
    }

    if(!b)
    {
        AddFailedTestToBuffer();
        ReportFailedAssert();
    }
    else
    {
        ReportSucceededAssert();
    }

    ClearAssert();
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

    if(test.ProgramReturnCode == 0)
    {
        Should("leave memory stack containing only global Caller/Self calls");
        OtherwiseReport(Msg("vm memory stack size is %i (expected 2)",
            MemoryStack.size()));
        Assert(MemoryStack.size() == 2);
    }
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

UnitTest test;

void ClearAssert()
{
    test.AssertName.clear();
    test.AssertType.clear();
    test.OtherwiseReport.clear();
    test.ExpectedClause.clear();

}

void ClearRun()
{
    test.ProgramFile.clear();
    test.ProgramName.clear();
    test.ProgramOutput.clear();
    test.ProgramReturnCode = 0;
    test.ProgramToRun = nullptr;

}

void ClearTest()
{
    test.TestName.clear();
    ClearRun();
    ClearAssert();
}

void DoAllTests()
{
    auto start = std::chrono::high_resolution_clock::now();
    for(auto test: Tests)
    {
        ClearTest();
        test();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    timespan = time_span.count();
}

char FirstNonSpaceChar(std::string& str)
{
    size_t i;
    for(i=0; i < str.size() && str[i] == ' '; i++);
    if(i < str.size())
    {
        return str[i];
    }

    return ' ';
}

void DisplayTestBuffer()
{
    std::istringstream iss(testBuffer);
    std::string line;
    while(std::getline(iss, line))
    {
        char c = FirstNonSpaceChar(line);

        if(std::isdigit(c))
        {
            std::cout << CONSOLE_WHITE;
        }
        else if(c == 'f' || c == 'a' || c == 'r')
        {
            std::cout << CONSOLE_CYAN;
        }
        else
        {
            std::cout << CONSOLE_RED;
        }
        
        std::cout << line << std::endl;
    }
}

bool Test()
{
    testBuffer.reserve(4096);
    std::cout << CONSOLE_YELLOW << "starting...\n" << CONSOLE_RESET;

    DoAllTests();


    if(failedAsserts)
    {
        std::cout << CONSOLE_YELLOW << "failure report: \n\n";
        DisplayTestBuffer();
    }

    std::cout.precision(2);
    std::cout << CONSOLE_YELLOW << "\n\nfinished " << (succeededAsserts + failedAsserts) << " assertions at " 
        << std::fixed << (100.0 * succeededAsserts / (succeededAsserts + failedAsserts)) 
        << " %";

    std::cout.precision(4);
    std::cout << CONSOLE_YELLOW << " in " << timespan << " secs\n";

    std::cout << CONSOLE_GREEN << "  >> passed: " << succeededAsserts << std::endl;
    std::cout << CONSOLE_RED << "  >> failed: " << failedAsserts << std::endl << std::endl;



    std::cout << CONSOLE_RESET;

    if(failedAsserts) 
        return 1;
    
    return 0;
}
