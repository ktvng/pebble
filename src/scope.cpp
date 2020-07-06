#include "scope.h"



// ---------------------------------------------------------------------------------------------------------------------
// Constructors

/// create a new scope with [inheritedScope]. all new scopes should be created from this
/// constructor method
Scope* ScopeConstructor(Scope* inheritedScope)
{
    Scope* s = new Scope;
    s->InheritedScope = inheritedScope;
    s->ReferencesIndex = {};

    return s;
}


// ---------------------------------------------------------------------------------------------------------------------
// Scopestack

using namespace utils;

/// stack which contains the hierarchy of scopes building up to the current Scope of execution
static Stack<Scope*> ScopeStack;

/// returns the current scope
Scope* CurrentScope()
{
    return ScopeStack.Peek();
}

/// enters a new scope
void EnterScope(Scope* newScope)
{
    ScopeStack.Push(newScope);
}

/// removes all references from a scope
void ClearScope()
{
    CurrentScope()->ReferencesIndex.clear();
}

/// exit the current scope and return to the previous scope (i.e. the scope before entering this one)
void ExitScope()
{
    ScopeStack.Pop();
}

/// add [ref] to current scope
void AddReferenceToCurrentScope(Reference* ref)
{
    if(CurrentScope() == nullptr)
        LogItDebug("current scope is not set", "AddReferenceToCurrentScope");
    LogItDebug("added reference to current scope", "AddReferenceToCurrentScope");
    CurrentScope()->ReferencesIndex.push_back(ref);
}
