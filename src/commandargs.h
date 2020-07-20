#ifndef __COMMANDARGS_H
#define __COMMANDARGS_H

#include "abstract.h"

struct Setting;

typedef String SettingOption;
typedef void (*SettingMethod)(std::vector<SettingOption>);

struct Setting
{
    String SettingName;
    String Flag;
    SettingMethod Action;
};

typedef std::vector<Setting> ProgramConfiguration;

void ParseCommandArgs(int argc, char* argv[], ProgramConfiguration config);

#endif
