#include <string>
#include <iostream>

#include "commandargs.h"

static ProgramConfiguration* Config = nullptr;
static Setting* CurrentState = nullptr;

bool MatchesFlag(String token)
{
    for(Setting& s: *Config)
    {
        if(token == s.Flag)
        {
            CurrentState = &s;
            return true;
        }
    }
    return false;
}

void ParseCommandArgs(int argc, char* argv[], ProgramConfiguration config)
{
    Config = &config;
    CurrentState = nullptr;

    std::vector<SettingOption> CurrentSettingOptions;
    CurrentSettingOptions.reserve(8);

    for(int i=1; i<argc; i++)
    {
        String arg = argv[i];
        if(MatchesFlag(arg))
        {
            CurrentState->Action(CurrentSettingOptions);
            CurrentSettingOptions.clear();
        }
        else
        {
            SettingOption option = arg;
            CurrentSettingOptions.push_back(option);
        }
    }

    /// flush
    if(CurrentState != nullptr)
    {
        CurrentState->Action(CurrentSettingOptions);
    }
}