#ifndef __DEMO_H
#define __DEMO_H

#include <vector>
#include <string>

int RunDemo();

struct Demo
{
    std::string FilePath;
    std::string DocumentationPath;
};

std::string DemoIndentLevel(int level);

#endif
