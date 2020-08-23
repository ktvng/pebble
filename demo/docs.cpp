#include "docs.h"
#include "consolecolor.h"
#include "demo.h"

#include <fstream>
#include <iostream>

size_t IndexOfFirstNonspaceChar(std::string& str)
{
    size_t i;
    for(i=0; i<str.size() && str[i] == ' '; i++);
    return i;
}

void InitSection(Section& sect)
{
    sect.Content.clear();
    sect.Content.reserve(1024);

    sect.Title.clear();
    sect.Title.reserve(32);
}

void ParseDoc(std::string filepath, Documentation& doc)
{
    std::fstream file;
    file.open(filepath, std::ios::in);

    Section sect;
    InitSection(sect);

    bool shouldAddNewline;
    bool lastLineEmpty;

    std::string line;
    while(std::getline(file, line))
    {
        if(line.empty())
        {
            if(lastLineEmpty)
            {
                shouldAddNewline = true;
            }
            else
            {
                lastLineEmpty = true;
            }

            continue;
        }

        if(shouldAddNewline)
        {
            sect.Content.push_back('\n');
        }

        lastLineEmpty = false;
        shouldAddNewline = false;

        size_t i = IndexOfFirstNonspaceChar(line);
        if(i < line.size())
        {
            if(line[i] == '#')
            {
                if(!sect.Title.empty())
                {
                    doc.Sections.push_back(sect);
                    InitSection(sect);
                }
                
                sect.Title = line.substr(i+2);
            }
            else
            {
                sect.Content += line;   
            }
        }
        else
        {
            sect.Content.push_back('\n');
        }
    }

    if(!sect.Title.empty())
    {
        doc.Sections.push_back(sect);
    }
}

const int GuardLineSize = 75;
const int IndentSize = 4;

inline void StartLine(int& currentLineSize, int indent)
{
    for(int i=0; i<IndentSize*indent; i++)
    {
        std::cout << ' ';
    }
    
    currentLineSize += IndentSize;
}

void IfNeededEndLine(int& currentLineSize, int indent)
{
    if(currentLineSize > GuardLineSize)
    {
        std::cout << '\n';
        currentLineSize = 0;

        StartLine(currentLineSize, indent);
    }
}

void FormattedPrint(std::string str, int& currentLineSize, int indent)
{
    std::cout << CONSOLE_RESET;

    if(currentLineSize == 0)
    {
        StartLine(currentLineSize, indent);
    }

    size_t pos = 0;
    while(pos < str.size())
    {
        for(; pos < str.size() && str[pos] != ' '; pos++)
        {
            std::cout << str[pos];
            currentLineSize++;

            if(str[pos] == '\n')
            {
                currentLineSize = GuardLineSize + 1;
                IfNeededEndLine(currentLineSize, indent);
            }
        }

        for(; pos < str.size() && str[pos] == ' '; pos++)
        {
            std::cout << ' ';
            currentLineSize++;
        }

        IfNeededEndLine(currentLineSize, indent);
    }
}

std::string Trim(std::string& str)
{
    std::string trimmedStr;
    trimmedStr.reserve(str.size());

    size_t start;
    size_t end;

    for(start = 0; start < str.size() && str[start] == ' '; start++);
    for(end = str.size()-1; end != 0 && str[end] == ' '; end--);

    for(size_t i=start; i<=end; i++)
    {
        trimmedStr += str[i];
    }

    return trimmedStr;
}

void DisplaySection(Section& sect, int indent=0)
{
    std::cout 
        << CONSOLE_RESET
        << DemoIndentLevel(indent) 
        << CONSOLE_UNDERLINE
        << CONSOLE_BOLD 
        << "" << Trim(sect.Title) << ":\n\n"
        << CONSOLE_RESET;

    int size = 0;
    FormattedPrint(sect.Content, size, indent+1);
    std::cout << "\n\n";
}

void DisplaySection(Documentation& doc, std::string sectionName, int indent)
{
    for(auto& sect: doc.Sections)
    {
        if(sect.Title == sectionName)
        {
            DisplaySection(sect, indent);
        }
    }
}
