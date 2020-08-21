#include <iostream>
#include <fstream>

#include "abstract.h"
#include "display.h"

#include "grammar.h"
#include "program.h"
#include "flattener.h"
#include "vm.h"

int RunDemo()
{
    std::string filepath = "./program.pebl";

    CompileGrammar();

    Program* p = nullptr;
    p = ParseProgram(filepath);

    if(p == nullptr)
    {
        return 1;
    }

    PrintProgramToConsole(p);

//     FlattenProgram(p);
//     DoByteCodeProgram(p);
//     ProgramDestructor(p);

    return 0;
}