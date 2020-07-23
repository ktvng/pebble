#ifndef __DFA_CPP
#define __DFA_CPP

#include <iostream>

#include "dfa.h"

#include "diagnostics.h"

template <typename T>
State<T>::State(T value, bool isAccepting)
{
    this->Value = value;
    this->Accepting = isAccepting;
}

template <typename T>
TransitionTable<T>::TransitionTable(std::vector<State<T>> states, std::vector<Transition<T>> transitions)
{
    this->States = states;
    this->Transitions = transitions;
}

template <typename T>
DFA<T>::DFA(TransitionTable<T>* table, T startingState)
{
    this->InternalTransitionTable = table;
    this->CurrentStateValue = startingState;
}

template <typename T>
bool DFA<T>::TypeToState(T type, State<T>** matchingState)
{
    for(auto state: InternalTransitionTable->States)
    {
        if(state.Value == type)
        {
            *matchingState = &state;
            return true;
        }
    }
    matchingState = nullptr;
    return false;
}

template <typename T> 
bool DFA<T>::CurrentStateAccepts()
{
    State<T>* state = nullptr;
    if(!(TypeToState(CurrentStateValue, &state)))
    {
        LogIt(LogSeverityType::Sev3_Critical, "CurrentStateAccepts", "unknown state");
        return false;
    }

    return state->Accepting;
}

template <typename T>
bool DFA<T>::TransitionOn(T nextInput)
{
    for(auto transition: InternalTransitionTable->Transitions)
    {
        if(transition.From == CurrentStateValue && transition.Into == nextInput)
        {
            CurrentStateValue = transition.Into;
            return true;
        }
    }

    return false;
}

#endif
