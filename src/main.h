#ifndef __MAIN_H
#define __MAIN_H

#include <string>
#include <vector>
#include <map>

#include "abstract.h"
#include "utils.h"


// ---------------------------------------------------------------------------------------------------------------------
// Typedefs

typedef std::string String;


/// if true will turn on output for 'print' command of the language
extern bool g_outputOn;

/// if true will use the BytecodeRuntime engine
extern bool g_useBytecodeRuntime;

#endif
