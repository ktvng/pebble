#ifndef __COMMANDARGS_H
#define __COMMANDARGS_H

#include "abstract.h"

struct Setting;

typedef String SettingOption;
typedef bool (*SettingMethod)(std::vector<SettingOption>);

struct Setting
{
    String SettingName;
    String Flag;
    SettingMethod Action;    // Returns true if a flag is consumed
};

typedef std::vector<Setting> ProgramConfiguration;

void ParseCommandArgs(int argc, char* argv[], ProgramConfiguration* config);

bool Usage(std::vector<SettingOption> options);

#endif
