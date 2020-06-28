#include "test.h"
#include "main.h"
#include "arch.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"
#include "reference.h"

std::map<std::string, int> methodHitMap;
std::map<MethodName, InjectedFunction> FunctionInjections;

void InjectBefore(MethodName name, InjectedFunction func)
{
    FunctionInjections[name] = func;
}


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

void testFuncInject(Params& p)
{
    const Reference* ref = *static_cast<const Reference* const*>(p.at(0));
    std::cout << ObjectOf(ref)->Class << std::endl;
}

void ConfigureLogging(LogSeverityType level, bool clearBefore)
{
    if(clearBefore)
        PurgeLog();

    LogAtLevel = level;
}

std::string chiefFile = "./program";

void Compile()
{
    CompileGrammar();
    ParseProgram(chiefFile);
}

void Execute()
{
    std::cout << "################################################################\n";
    DoProgram(*PROGRAM);
    std::cout << "################################################################\n";
}

void Test()
{
    InjectBefore("OperationAdd", testFuncInject);

    ConfigureLogging(LogSeverityType::Sev3_Critical, true);
    Compile();
    Execute();



    std::cout << "INFO:\n";
    std::cout << "Objects created: " << methodHitMap["ObjectConstructor"]+1 << std::endl; // +1 for null
    std::cout << "Objects in Index: " << ObjectIndexSize(PROGRAM) << std::endl;

    std::cout << std::endl;

    std::cout << "References created: " << methodHitMap["ReferenceConstructor"] << std::endl;
    std::cout << "References in index: " << ReferencesInIndex(PROGRAM) << std::endl;
    std::cout << "References destroyed: " << methodHitMap["Dereference"] << std::endl;
    std::cout << "Methods: " << methodHitMap["MethodConstructor"] << std::endl;
}