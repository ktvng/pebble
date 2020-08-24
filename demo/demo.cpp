#include <iostream>
#include <fstream>
#include <sstream>

#include "demo.h"

#include "abstract.h"
#include "display.h"
#include "consolecolor.h"
#include "docs.h"

#include "main.h"
#include "grammar.h"
#include "program.h"
#include "flattener.h"
#include "vm.h"

#include "diagnostics.h"


int g_demoNumber = 1;

void Wait()
{
    std::string s;
    std::cout << CONSOLE_MAGENTA << "\nPress (ENTER) to continue\n" << CONSOLE_RESET;
    std::getline(std::cin, s);
}

bool PromptYN(std::string prompt)
{
    while(true)
    {
        std::string s;
        std::cout << CONSOLE_MAGENTA << prompt << " (Y/n)\n" << CONSOLE_RESET;
        std::getline(std::cin, s); 

        if(s == "n")
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

void RunProgramSuppressingOutput(Program* p)
{
    g_outputOn = false;
    FlattenProgram(p);
    DoByteCodeProgram(p);
    ProgramDestructor(p);
}

const int IndentSize = 4;

std::string DemoIndentLevel(int level)
{
    std::string indent;
    indent.reserve(level*IndentSize);

    for(int i=0; i<level*IndentSize; i++)
    {
        indent.push_back(' ');
    }

    return indent;
}

void InternalDisplayProgramOutput(std::string output)
{
    std::cout 
        << CONSOLE_RESET 
        << DemoIndentLevel(1) 
        << CONSOLE_UNDERLINE 
        << CONSOLE_BOLD 
        << "The Output:\n\n"
        << CONSOLE_RESET;

    if(output[output.size()-1] == '\n')
    {
        output[output.size()-1] = ' ';
    }

    String rightEdge = DemoIndentLevel(2) + "> ";
    std::istringstream is(output);

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

void DisplayProgramOutput()
{
    InternalDisplayProgramOutput(ProgramOutput);
}

void DisplayCompileOutput()
{
    InternalDisplayProgramOutput(ProgramMsgs);
}

void DisplayDemoHeader()
{
    std::cout 
        << CONSOLE_RESET
        << CONSOLE_BOLD
        << CONSOLE_UNDERLINE
        << "DEMO #" << g_demoNumber << "\n"
        << CONSOLE_RESET;

    g_demoNumber += 1;
    std::cout << std::endl;

}

void RunDemoFile(std::string filepath)
{
    ProgramOutput.clear();
    ProgramMsgs.clear();
    FatalCompileError = false;

    CompileGrammar();

    Program* p = nullptr;
    p = ParseProgram(filepath);

    std::cout 
        << CONSOLE_RESET 
        << DemoIndentLevel(1) 
        << CONSOLE_UNDERLINE 
        << CONSOLE_BOLD 
        << "The Code:\n\n"
        << CONSOLE_RESET;

    PrintProgramToConsole(p);

    if(FatalCompileError || p == nullptr)
    {
        std::cout << CONSOLE_RESET << std::endl;
        DisplayCompileOutput();
        return;
    }

    RunProgramSuppressingOutput(p);
    std::cout << CONSOLE_RESET << std::endl;
    DisplayProgramOutput();
}


void InternalRunDemo(Demo& demo)
{
    std::system("clear");


    DisplayDemoHeader();
    
    Documentation doc;
    ParseDoc(demo.DocumentationPath, doc);
    DisplaySection(doc, "Overview");
    RunDemoFile(demo.FilePath);
    DisplaySection(doc, "Details");

    Wait();
}

void DipslayConclusion()
{
    std::system("clear");

    Documentation doc;
    ParseDoc("./demo/demos/docs/conclusion", doc);
    DisplaySection(doc, "Thanks", 0);
    Wait();
    
    std::system("clear");
}

void DisplayIntro()
{
    std::system("clear");

    Documentation doc;
    ParseDoc("./demo/demos/docs/intro", doc);
    DisplaySection(doc, "Intro", 0);
    DisplaySection(doc, "Details", 0);

    Wait();
}

void DeploySandbox()
{
    do
    {
        std::system("clear");
        Documentation doc;
        ParseDoc("./demo/demos/docs/sandbox", doc);
        DisplaySection(doc, "Sandbox", 0);

        Wait();

        RunDemoFile("./program.pebl");

        if(FatalCompileError)
        {
            std::cout << CONSOLE_RESET << DemoIndentLevel(1) 
                << "It looks like you made a syntax error...\n\n";
        }

    } while(PromptYN("Keep editing your program?"));
}


#define PEBL_DEMO_N(X)                                      \
{                                                           \
    "./demo/demos/programs/p"+std::to_string(X)+".pebl",    \
    "./demo/demos/docs/demo"+std::to_string(X)              \
}

static std::vector<Demo> Demos = 
{
    PEBL_DEMO_N(1),
    PEBL_DEMO_N(2),
    PEBL_DEMO_N(3),
    PEBL_DEMO_N(4),
    PEBL_DEMO_N(5),
    PEBL_DEMO_N(6),
    PEBL_DEMO_N(7),
    PEBL_DEMO_N(8),
    PEBL_DEMO_N(9),
};

int RunDemo()
{
    DisplayIntro();

    g_demoNumber = 1;
    for(auto& demo: Demos)
    {
        InternalRunDemo(demo);
    }

    DeploySandbox();
    DipslayConclusion();

    return 0;
}