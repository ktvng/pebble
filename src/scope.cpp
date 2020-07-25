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
    s->IsDurable = false;

    return s;
}

void ScopeDestructor(Scope* scope)
{
    delete scope;
}

