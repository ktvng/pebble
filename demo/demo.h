#ifndef __DEMO_H
#define __DEMO_H

#include <vector>
#include <string>

int RunDemo();

struct Demo
{
    std::string FilePath;
    std::string MetaFilePath;
};

static std::vector<Demo> Demos;


#endif
