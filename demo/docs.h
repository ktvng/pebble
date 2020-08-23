#ifndef __DOCS_H
#define __DOCS_H

#include <vector>
#include <string>

struct Section
{
    std::string Title;
    std::string Content;
};

struct Documentation
{
    std::vector<Section> Sections;
};

void ParseDoc(std::string filepath, Documentation& doc);
void DisplaySection(Documentation& doc, std::string sectionName);

#endif
