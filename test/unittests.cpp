#include "unittests.h"

#include "decision.h"
#include "parse.h"
#include "token.h"
#include "diagnostics.h"
#include "main.h"
#include "object.h"
#include "operation.h"
#include "program.h"
#include "reference.h"
#include "scope.h"


// ---------------------------------------------------------------------------------------------------------------------
// Tests

void testFuncInject(Params& p)
{
    const Reference* ref = *static_cast<const Reference* const*>(p.at(0));
    std::cout << "first object is of type: " << ObjectOf(ref)->Class << std::endl;
}

void TestCustomProgram()
{
    if(!g_shouldRunCustomProgram)
        return;
        
    It("Custom Program executes");
    RunCustomProgram();
    // InjectBefore("OperationAdd", testFuncInject);

    ConfigureLogging(LogSeverityType::Sev3_Critical, true);
    Compile();
    Execute();

    IncludeStandardAssertSuite();
}

void TestIf()
{
    It("evaluates an if statement");
    
    CompileAndExecuteProgram("TestIf");


    Should("conditionally execute code by evaluating if-expression");
    OtherwiseReport("unknown failure condition");
    Assert(Result.Equals("INIF\n"));

    IncludeStandardAssertSuite();
}

void TestOrderOfOperations()
{
    It("evaluates order of operations");

    SetProgramToRun("TestOrderOfOperations");
    Expected("28\n6\n7\n-5\n-10\n8\n");

    DisableLogging();
    Compile();
    Execute();
    
    Should("execute operations in PEMDAS order");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestMethodWithNoParams()
{
    It("can define/evaluate method with no params");

    CompileAndExecuteProgram("TestMethodWithNoParams");

    Expected("48\n60\n");
    Should("define and execute method with no params");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestMethodWithSingleParam()
{
    It("can define/evaluate method with single param");

    CompileAndExecuteProgram("TestMethodWithSingleParam");
    Expected("10\n7\n");

    Should("define and execute method with single param");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestMethodWithMultipleParams()
{
    It("can define/evaluate method with multiple params");

    CompileAndExecuteProgram("TestMethodWithMultipleParams");
    Expected("9\n3\n");

    Should("define and execute method with multiple params");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestScopeAccess()
{
    It("keeps variables accessible only in local scope");

    CompileAndExecuteProgram("TestScopeAccess");
    Expected("<Nothing>\n<Nothing>\n<Nothing>\n<Nothing>\n10\n");

    Valgrind();

    Should("not allow accessing variables outside declared scope");
    OtherwiseReport(Diff());
    Assert(Result.Expected());
}

void TestMethodRecursion()
{
    It("method can call itself");

    CompileAndExecuteProgram("TestMethodRecursion");
    Expected("5\n4\n3\n2\n1\n0\n120\n64\n");

    Should("allow method to call itself");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestComments()
{
    It("tests that comments are removed from files");

    CompileAndExecuteProgram("TestComments");
    
    IncludeStandardAssertSuite();

    Expected("30\n#60\n");

    Should("not parse program comments marked by a #");
    OtherwiseReport(Diff());
    Assert(Result.Expected());
}

void TestIfElseComplex()
{
    It("correctly evaluates an if/else-if/else statement");

    CompileAndExecuteProgram("TestIfElseComplex");
    Expected("1\n2\nX\n3\n1\n");

    Should("cascade through else-ifs and pool in else");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestWhile()
{
    It("correctly evaluates a while-loop");

    CompileAndExecuteProgram("TestWhile");
    Expected("5\n4\n3\n2\n1\n-5\n-4\n-3\n-2\n-1\n");

    Should("break out of while when condition is false");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestHere()
{
    It("correctly evaluates a method with the here operator");

    CompileAndExecuteProgram("TestHere");
    Expected("7\n");

    Should("execute method in inplace without changing scope when called with here operator");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestMethodReturn()
{
    It("correctly returns caller/self/object");

    CompileAndExecuteProgram("TestMethodReturn");
    Expected("Caller\nCaller\nSelf1\nSelf2\nSelf3\n12\n<Nothing>\nSelf4\n");

    Should("return the appropriate value");
    OtherwiseReport(Diff());
    Assert(Result.Expected());

    IncludeStandardAssertSuite();
}

void TestTypeSystem()
{
    It("enforces strict typing");

    CompileAndExecuteProgram("TestMismatchedTypeAssignment");
    Expected("thrown exception");
    Valgrind();
    
    Should("return the appropriate value");
    OtherwiseReport(Diff());
    Assert(Result.EncounteredFatalException());

    CompileAndExecuteProgram("TestAlignedTypeAssignment");
    Should("allow assignment when types match");
    Assert(Result.Not.EncounteredFatalException());

    IncludeStandardAssertSuite();
}

// ---------------------------------------------------------------------------------------------------------------------
// Test index

std::vector<TestFunction> Tests = 
{
    TestCustomProgram,

    // Tests for scoping
    TestScopeAccess,

    // Test for control flow
    TestIf,
    TestIfElseComplex,
    TestWhile,

    // Tests for operations
    TestOrderOfOperations,

    // Tests for methods
    TestMethodWithNoParams,
    TestMethodWithSingleParam,
    TestMethodWithMultipleParams,
    TestMethodRecursion,
    TestMethodReturn,

    // Tests for compile time
    TestComments,

    // Tests for the type system
    TestTypeSystem,
};
