#include "unittests.h"

// ---------------------------------------------------------------------------------------------------------------------
// Tests

void testFuncInject(Params &p)
{
    const Reference *ref = *static_cast<const Reference *const *>(p.at(0));
    std::cout << "first object is of type: " << ObjectOf(ref)->Class << std::endl;
}

void TestCustomProgram()
{
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
    Assert(ProgramOutput == correctOutput);
}

void TestMethodWithNoParams()
{
    It("can define/evaluate method with no params");

    CompileAndExecuteProgram("TestMethodWithNoParams");

    IncludeStandardAssertSuite();

    String correctOutput = "48\n60\n";

    Should("define and execute method with no params");
    OtherwiseReport("diff\ngot:\n" + ProgramOutput + "\nexpected:\n" + correctOutput);
    Assert(ProgramOutput == correctOutput);
}

void TestMethodWithSingleParam()
{
    It("can define/evaluate method with single param");

    CompileAndExecuteProgram("TestMethodWithSingleParam");

    IncludeStandardAssertSuite();

    String correctOutput = "10\n7\n";

    Should("define and execute method with single param");
    OtherwiseReport("diff\ngot:\n" + ProgramOutput + "\nexpected:\n" + correctOutput);
    Assert(ProgramOutput == correctOutput);
}

void TestMethodWithMultipleParams()
{
    It("can define/evaluate method with multiple params");

    CompileAndExecuteProgram("TestMethodWithMultipleParams");

    IncludeStandardAssertSuite();

    String correctOutput = "9\n3\n";

    Should("define and execute method with multiple params");
    OtherwiseReport("diff\ngot:\n" + ProgramOutput + "\nexpected:\n" + correctOutput);
    Assert(ProgramOutput == correctOutput);
}

void TestScopeAccess()
{
    It("keeps variables accessible only in local scope");

    CompileAndExecuteProgram("TestScopeAccess");

    TestObjectMemoryLoss();
    TestReferenceMemoryLoss();

    String correctOutput = "Nothing\nNothing\nNothing\nNothing\n10\n";

    Should("not allow accessing variables outside declared scope");
    OtherwiseReport("diff\ngot:\n" + ProgramOutput + "\nexpected:\n" + correctOutput);
    Assert(ProgramOutput == correctOutput);
}

void TestMethodRecursion()
{
    It("method can call itself");

    CompileAndExecuteProgram("TestMethodRecursion");

    IncludeStandardAssertSuite();

    String correctOutput = "5\n4\n3\n2\n1\n0\n120\n64\n";

    Should("allow method to call itself");
    OtherwiseReport("diff\ngot:\n" + ProgramOutput + "\nexpected:\n" + correctOutput);
    Assert(ProgramOutput == correctOutput);
}

void TestComments()
{
    It("tests the hashtag for comment functionality");

    CompileAndExecuteProgram("TestComments");

    IncludeStandardAssertSuite();

    String correctOutput = "7";

    Should("prevent anything to the right of a # from being parsed");
    OtherwiseReport("diff\ngot:\n" + ProgramOutput + "\nexpected:\n" + correctOutput);
    Assert(ProgramOutput == correctOutput);
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

        // Tests for operations
        TestOrderOfOperations,

        // Tests for methods
        TestMethodWithNoParams,
        TestMethodWithSingleParam,
        TestMethodWithMultipleParams,
        TestMethodRecursion,

        // Test for comments
        TestComments};
