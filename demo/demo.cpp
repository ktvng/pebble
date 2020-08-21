#include <iostream>
#include <fstream>
#include <sstream>

#include "abstract.h"
#include "display.h"
#include "consolecolor.h"

#include "main.h"
#include "grammar.h"
#include "program.h"
#include "flattener.h"
#include "vm.h"

#include "diagnostics.h"

int g_demoNumber = 1;
bool g_sandboxMode = false;

void Wait()
{
    std::string s;
    std::cout << CONSOLE_RESET << "Press (ENTER) to continue\n";
    std::getline(std::cin, s);
}

bool PromptYN(std::string prompt)
{
    while(true)
    {
        std::string s;
        std::cout << CONSOLE_RESET << prompt << " (y/n)\n";
        std::getline(std::cin, s);

        if(s == "y")
        {
            return 1;
        }
        else if(s == "n")
        {
            return 0;
        }
        else
        {
            std::cout << "Please enter either 'y' or 'n'!\n";
        }
    }

    return 0;
}

void RunProgramSuppressingOutput(Program* p)
{
    ProgramOutput.clear();
    g_outputOn = false;
    FlattenProgram(p);
    DoByteCodeProgram(p);
    ProgramDestructor(p);
}

void DisplayProgramOutput()
{
    std::cout << "  The Output:\n\n";
    ProgramOutput[ProgramOutput.size()-1] = ' ';

    String rightEdge = "    > ";
    std::istringstream is(ProgramOutput);

    std::string line;

    bool isLastResultError = false;
    while(std::getline(is, line))
    {
        if(isLastResultError)
        {
            isLastResultError = false;
            std::cout << CONSOLE_RESET << rightEdge << CONSOLE_RED << line;
        }
        else if(line.find("Fatal Exception at line[") != std::string::npos)
        {
            isLastResultError = true;
            std::cout << CONSOLE_RESET << rightEdge << CONSOLE_RED << line;
        }
        else
        {
            std::cout << CONSOLE_RESET << rightEdge << line;
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

void DisplayDemoHeader()
{
    if(g_sandboxMode)
    {
        std::cout << "\nYour program:\n\n";
        return;
    }

    std::cout << "\nDemo #" << g_demoNumber << "\n";
    g_demoNumber += 1;
    std::cout << std::endl;

}

void RunFileAsDemo(std::string filepath)
{
    std::system("clear");
    DisplayDemoHeader();
    
    CompileGrammar();

    Program* p = nullptr;
    p = ParseProgram(filepath);

    if(p == nullptr)
    {
        return;
    }

    PrintProgramToConsole(p);
    RunProgramSuppressingOutput(p);

    std::cout << CONSOLE_RESET << std::endl << std::endl;

    DisplayProgramOutput();

    Wait();
}

void DeploySandbox()
{
    do
    {
        std::system("clear");
        std::cout << "\nSandbox!\n\n";
        g_sandboxMode = true;

        std::cout 
            << "    Now is the time to play around! Open up the './program.pebl "
            << "file\n    and hack away to your heart's content!\n\n"
            << "    When you're done editing, press (ENTER) on here on terminal "
            << "to run\n    your custom Pebble program!\n\n";

        Wait();

        RunFileAsDemo("./program.pebl");

    } while(PromptYN("Keep editing your program?"));

}

int RunDemo()
{
    g_demoNumber = 1;
    g_sandboxMode = false;

    RunFileAsDemo("./test/programs/TestFeature_Nothing.pebl");
    RunFileAsDemo("./test/programs/TestFeature_TypedContainer.pebl");

    DeploySandbox();

    return 0;
}