#ifndef __DFA_H
#define __DFA_H

#include "abstract.h"

template <typename T>
struct Transition
{
    T From;
    T Into;
};

template <typename T>
class State
{
    public:

    T Value;
    bool Accepting;
    State(T value, bool isAccepting=false);
};

template <typename T>
class TransitionTable
{
    public:
    std::vector<State<T>> States;
    std::vector<Transition<T>> Transitions;
    TransitionTable(std::vector<State<T>> states, std::vector<Transition<T>> transitions);
};

template <typename T>
class DFA
{
    private:
    T CurrentStateValue;
    bool TypeToState(T type, State<T>** matchingState);
    TransitionTable<T>* InternalTransitionTable;

    public:
    DFA(TransitionTable<T>* table, T startingState);
    bool CurrentStateAccepts();
    bool TransitionOn(T nextInput);

};

#include "dfa.cpp"
#endif
