#include "scope.h"

#include "diagnostics.h"
#include "reference.h"
#include "utils.h"

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

void WipeScope(Scope* scope)
{
    for(auto ref: scope->ReferencesIndex)
    {
        RemoveReferenceFromObjectIndex(ref);
        ReferenceDestructor(ref);
    }
    ScopeDestructor(scope);
}

void ScopeDestructor(Scope* scope)
{
    delete scope;
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

/// exit the current scope and return to the previous scope (i.e. the scope before entering this one)
void ExitScope(bool andDestroy)
{
    auto scope = ScopeStack.Pop();
    if(andDestroy)
        ScopeDestructor(scope);
}

/// add [ref] to current scope
void AddReferenceToCurrentScope(Reference* ref)
{
    if(CurrentScope() == nullptr)
        LogItDebug("current scope is not set", "AddReferenceToCurrentScope");
    LogItDebug("added reference to current scope", "AddReferenceToCurrentScope");
    CurrentScope()->ReferencesIndex.push_back(ref);
}

bool ScopeStackIsEmpty()
{
    return ScopeStack.Size() == 0;
}