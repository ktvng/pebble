#ifndef __SCOPE_H
#define __SCOPE_H

#include "abstract.h"






// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// a scope is the context in which the compiler and runtime environment resolve references
/// and add new references.
/// [ReferencesIndex] contains all references available in the scope
/// [InheritedScope] is a link to the parent scope and to inherited references
struct Scope
{
    std::vector<Reference*> ReferencesIndex;
    Scope* InheritedScope;
    bool IsDurable;
};


// ---------------------------------------------------------------------------------------------------------------------
// Scoping

/// create a new scope with [inheritedScope]
Scope* ScopeConstructor(Scope* inheritedScope);

bool ScopeStackIsEmpty();

/// destroys the [scope]
void ScopeDestructor(Scope* scope);

void WipeScope(Scope* scope);

/// add [newscope] to the top of the scope stack so all reference matching is done inside [newscope] 
void EnterScope(Scope* newScope);

/// returns the active scope which is the top of the scope stack
Scope* CurrentScope();

/// pops the top of the scope stack to return to the previous scope
void ExitScope(bool andDestroy = false); 

/// adds [ref] to the ReferencesIndex of the current scope
void AddReferenceToCurrentScope(Reference* ref);

#endif
