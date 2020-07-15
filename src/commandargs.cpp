#include <string>
#include <iostream>

#include "commandargs.h"

static ProgramConfiguration* Config = nullptr;
static Setting* CurrentState = nullptr;
String programName;

bool MatchesFlag(String token)
{
	if(token.length() < 1)
	{
		return false;
	}

    for(Setting& s: *Config)
    {
        if(token == s.Flag)
        {
            CurrentState = &s;
            return true;
        }
        else if(token.at(0) == '-')
        {
        	// If an unknown which resembles a flag is passed, inform the user
        	Usage(std::vector<SettingOption>());
        }
    }
    return false;
}

void ParseCommandArgs(int argc, char* argv[], ProgramConfiguration config)
{
    Config = &config;
    CurrentState = nullptr;
    bool inFlag = false;
    programName = "./program.pebl";

    std::vector<SettingOption> CurrentSettingOptions;
    CurrentSettingOptions.reserve(8);

    for(int i=1; i<argc; i++)
    {
        String arg = argv[i];
        if(MatchesFlag(arg))
        {
            CurrentState->Action(CurrentSettingOptions);
            CurrentSettingOptions.clear();
            inFlag = true;
        }
        else if(inFlag)
        {
        	// Flags consume one argument
            SettingOption option = arg;
            CurrentSettingOptions.push_back(option);
            inFlag = false;
        }
        else
        {
			// Set program name to run
			programName = argv[i];
			break;
        }
    }

    /// flush
    if(CurrentState != nullptr)
    {
        CurrentState->Action(CurrentSettingOptions);
    }
}