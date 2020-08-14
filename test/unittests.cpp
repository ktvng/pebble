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
// Documentation
/*

Unit Tests
    
    Unit tests are used to test both overall Pebble functionality as well as 
    the behavior of individual methods. Tests are run on a special build of 
    Pebble (pebble_testbuild) which has most important method decorated by 
    wrappers that allow for additional metrics for testing.

    A unit test has a few main ingredients which comprise a standardized 
    structure, but also provide the tester the flexiblity to write targeted and
    precise tests for niche cases.

    Okay, enough plugging the tests, let's dive into it.

    Unit tests are simple methods which follow a structure that allows the 
    test harness to execute Pebble code and ensure certain assertions are valid.
    Each test method is of the form

        void MethodName();
    
    and in order for the test to be registered, must be added to the std::vector
    of tests at the bottom of this file

        std::vector<TestFunction> Tests = { ... }

    Test methods are expected to have a relatively uniform structure.

Structure

    Unit tests are build from the following main ingredients which are assembled
    in the order listed below:

        - [ItTests] declaration
        - [CompileAndExecute] a program
        - [Should] statement
        - [Expected] statement
        - [Assert] clause

    [ItTests]
        Usage: ItTests(<string>) 
        Description:
            The <string> should be a description of what is being tested by the
            unit test. This can be a method, feature, or other characteristic.

    [CompileAndExecute]
        Usage: CompileAndExecuteProgram(<string>)
        Description:
            The <string> should be the name of a program located at path
            "./test/programs/" with the file extension ".pebl". Best practice
            is to have this program file share the name of the unit test method
        Note:
            The Compile() and Execute() methods can alse be used in conjunction 
            with SetProgramToRun(<string>) to split the compile and runtime 
            phases for additional test granularity. 

            SetProgramToRun() takes the same parameter as
            CompileAndExecuteProgram

    [Should]
        Usage: Should(<string>)
        Description:
            The Should() method specifies the attribute/condition which is 
            being tested in a single assertion. It should hint at the expected
            behavior of the entity being tested which was specified by the 
            [ItTests] declaration.

    [Expected]
        Usage: Expected(<string>)
        Description:
            The Expected() method specifies the expected output or running the
            Pebble program. It is used primarily for feature tests which look
            at the output of an entire Pebble program

    [Assert]
        Usage: Assert(<bool>)
        Description:
            The Assert() method takes in a <bool> which describes whether or not
            the assertion is a success or failure. This <bool> can be the result
            of a user defined expression or can be taken from a selection of
            prebuilt boolean expression methods which target specific outcomes
            of running a Pebble program. These are documented further on.

Example structure

    The following example highlights the guidelines of writing clear, neat, and
    nice looking unit test methods

        void TestAnExample()
        {
            // enter the name of the entity being tested
            ItTests("someMethodNameHere");     

            // there should be a program saved at
            // './test/programs/TestAnExample.pebl'
            // with the code to trigger the tested
            // entity. notice that this shares a name
            // with the unit test method
            CompileAndExecuteProgram("TestAnExample"); 
                // indent recommended here
                Should("successfuly update X and do Y");    // specify the intended
                                                            // behavior
                Expected("math result:\n12\n");             // expected output of 
                                                            // running TestAnExample.pebl
                Assert(Result.AsExpected());                // assert the result is 
                                                            // equal to what is Expected()
        }
    
The Result wrapper

    There is a predefined wrapper around the output of the Pebble program called
    'Result'. This has built-in attributes and method that can be used to make
    testing for specific properties easier and more readable. These are

        - AsExpected()
            Description: true if ProgramOutput == Expected() as strings
        
        - Eauals(<string>)
            Description: true if ProgramOutput == <string> 
        
        - Contains(<string>)
            Description: true if ProgramOutput contains <string>

        - EncounteredFatalException()
            Description: true if a fatal runtime execption was encountered

        - EncountedNonFatalException()
            Description: true if a non-fatal runtime execption was encountered

    Additionally, 'Result' has the 'Not' attributes which effectively negates 
    the boolean conditions described above

        Ex:
        Result.Not.Contains("error!")   // true if ProgramOutput does not contain "error!"

FAQ:

    This is intended to save a lot of stressful debug print statements because 
    something tivial went wrong. Everything here was tried by fire

    1)  Q:  Weird stuff is going on. I'm failing tests I shouldn't be!
        A:  Make sure your file endings are LF and not CFLF. This breaks a lot
            of things and if you aren't careful, will frustrate you to no end.

    2)  Q:  My output looks right but the test is failing?
        A:  Make sure that if you are testing for Pebble program output, you include
            a final '\n' on your Expected() string.

    3)  Q:  Does the order of Should() and Expected() matter?
        A:  Short answer, no. Long answer... for consistency Should() ought to
            precede Expected(). Thanks

    4)  Q:  Something is wrong? My tests don't seem to be run
        A:  Make sure you add your tests to the std::vector at the bottom of this
            file!!!! It's best practice to always write a faling test first
            so you have visual confirmation that a test is being run.

    5)  Q:  How do I know that tests can fail?
        A:  Run the testbuild without the flag 'ignore-custom' and fail the very
            sad, very lonely meta failure test.


*/

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
        
    ItTests("Custom Program executes");
    RunCustomProgram();
        // InjectBefore("OperationAdd", testFuncInject);

        ConfigureLogging(LogSeverityType::Sev3_Critical, true);
        Compile();
        Execute();
}

void TestIf()
{
    ItTests("evaluates an if statement");
    
    CompileAndExecuteProgram("TestIf");
        Should("conditionally execute code by evaluating if-expression");
        Expected("INIF\n");
        Assert(Result.AsExpected());
}

void TestOrderOfOperations()
{
    ItTests("evaluates order of operations");

    CompileAndExecuteProgram("TestOrderOfOperations");
        Should("execute operations in PEMDAS order");
        Expected("28\n6\n7\n-5\n-10\n8\n");
        Assert(Result.AsExpected());
}

void TestMethodWithNoParams()
{
    ItTests("can define/evaluate method with no params");

    CompileAndExecuteProgram("TestMethodWithNoParams");
        Should("define and execute method with no params");
        Expected("48\n60\n");
        Assert(Result.AsExpected());
}

void TestMethodWithSingleParam()
{
    ItTests("can define/evaluate method with single param");

    CompileAndExecuteProgram("TestMethodWithSingleParam");
        Should("define and execute method with single param");
        Expected("10\n7\n");
        Assert(Result.AsExpected());
}

void TestMethodWithMultipleParams()
{
    ItTests("can define/evaluate method with multiple params");

    CompileAndExecuteProgram("TestMethodWithMultipleParams");
        Should("define and execute method with multiple params");
        Expected("9\n3\n");
        Assert(Result.AsExpected());
}

void TestScopeAccess()
{
    ItTests("keeps variables accessible only in local scope");

    CompileAndExecuteProgram("TestScopeAccess");
        Should("not allow accessing variables outside declared scope");
        Expected("<Nothing>\n<Nothing>\n<Nothing>\n<Nothing>\n10\n");
        Assert(Result.AsExpected());
}

void TestMethodRecursion()
{
    ItTests("method can call itself");

    CompileAndExecuteProgram("TestMethodRecursion");
        Should("allow method to call itself");
        Expected("5\n4\n3\n2\n1\n0\n120\n64\n");
        Assert(Result.AsExpected());
}

void TestComments()
{
    ItTests("tests that comments are removed from files");

    CompileAndExecuteProgram("TestComments");
        Should("not parse program comments marked by a #");
        Expected("30\n#60\n");
        Assert(Result.AsExpected());
}

void TestIfElseComplex()
{
    ItTests("correctly evaluates an if/else-if/else statement");

    CompileAndExecuteProgram("TestIfElseComplex");
        Should("cascade through else-ifs and pool in else");
        Expected("1\n2\nX\n3\n1\n");
        Assert(Result.AsExpected());
}

void TestWhile()
{
    ItTests("correctly evaluates a while-loop");

    CompileAndExecuteProgram("TestWhile");
        Should("break out of while when condition is false");
        Expected("5\n4\n3\n2\n1\n-5\n-4\n-3\n-2\n-1\n");
        Assert(Result.AsExpected());
}

void TestHere()
{
    ItTests("correctly evaluates a method with the here operator");

    CompileAndExecuteProgram("TestHere");
        Should("execute method in inplace without changing scope when called with here operator");
        Expected("7\n");
        Assert(Result.AsExpected());
}

void TestMethodReturn()
{
    ItTests("correctly returns caller/self/object");

    CompileAndExecuteProgram("TestMethodReturn");
        Should("return the appropriate value");
        Expected("Caller\nCaller\nSelf1\nSelf2\nSelf3\n12\n<Nothing>\nSelf4\n");
        Assert(Result.AsExpected());
}

void TestTypeSystem()
{
    ItTests("enforces a runtime type system");

    CompileAndExecuteProgram("TestMismatchedTypeAssignment");
        Should("return the appropriate value");
        Expected("exception to be thrown");
        Assert(Result.EncounteredFatalException());

    CompileAndExecuteProgram("TestAlignedTypeAssignment");
        Should("allow assignment when types match");
        Expected("no execeptions thrown");
        Assert(Result.Not.EncounteredFatalException());

        Should("match expected output");
        Expected("hello\n");
        Assert(Result.AsExpected());
}

void TestMetaFailure()
{
    if(!g_shouldRunCustomProgram)
    {
        return;
    }

    ItTests("is will ever succeed");
    CompileAndExecuteProgram("TestMetaFailure");
    Should("run with custom program and always fail");
    Expected("this test is expected to fail\nand serve as an example of\na failing test");
    Assert(false);
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

    // Meta tests
    TestMetaFailure,
};
