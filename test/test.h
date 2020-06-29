#ifndef __TEST_H
#define __TEST_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

typedef std::vector<const void*> Params;
typedef void (*InjectedFunction)(Params&);

typedef std::string MethodName;
extern std::map<MethodName, InjectedFunction> FunctionInjections; 


extern std::map<MethodName, int> methodHitMap;
void Test();

inline void It(const std::string& name);
void Assert(bool b);
inline void OtherwiseReport(const std::string& descriptionOfFailureCase);
inline void Should(const std::string& descriptionOfTest);

#endif
