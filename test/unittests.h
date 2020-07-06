#ifndef __UNITTESTS_H
#define __UNITTESTS_H

#include "test.h"

typedef void (*TestFunction)();
extern std::vector<TestFunction> Tests;

#endif