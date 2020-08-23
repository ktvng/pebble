#include "docs.h"
#include "consolecolor.h"

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
}

const int GuardLineSize = 80;
const int IndentSize = 4;

inline void StartLine(int& currentLineSize)
{
    for(int i=0; i<IndentSize; i++)
    {
        std::cout << ' ';
    }
    
    currentLineSize += IndentSize;
}

void IfNeededEndLine(int& currentLineSize)
{
    if(currentLineSize > GuardLineSize)
    {
        std::cout << '\n';
        currentLineSize = 0;

        StartLine(currentLineSize);
    }
}

void FormattedPrint(std::string str, int& currentLineSize)
{
    std::cout << CONSOLE_RESET;

    if(currentLineSize == 0)
    {
        StartLine(currentLineSize);
    }

    size_t pos = 0;
    while(pos < str.size())
    {
        for(; pos < str.size() && str[pos] != ' '; pos++)
        {
            std::cout << str[pos];
            currentLineSize++;
        }

        for(; pos < str.size() && str[pos] == ' '; pos++)
        {
            std::cout << ' ';
            currentLineSize++;
        }

        IfNeededEndLine(currentLineSize);
    }
}

void DisplaySection(Section& sect)
{
    std::cout << CONSOLE_RESET << "  " << sect.Title << "\n\n";
    int size = 0;
    FormattedPrint(sect.Content, size);
    std::cout << "\n\n";
}

void DisplaySection(Documentation& doc, std::string sectionName)
{
    for(auto& sect: doc.Sections)
    {
        if(sect.Title == sectionName)
        {
            DisplaySection(sect);
        }
    }
}
