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

    /// new scope
    std::vector<Call*> CallsIndex;
};


// ---------------------------------------------------------------------------------------------------------------------
// Constructors/Destructors

/// create a new scope with [inheritedScope]
Scope* ScopeConstructor(Scope* inheritedScope);

/// destroys the [scope]
void ScopeDestructor(Scope* scope);

/// DEP: 
void AddReferenceToScope(Reference* ref, Scope* scope);

/// add [call] to [scope]
void AddCallToScope(Call* call, Scope* scope);

/// return a deep copy of [scp]
Scope* CopyScope(Scope* scp);

#endif
