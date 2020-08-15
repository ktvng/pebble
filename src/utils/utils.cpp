#ifndef __UTILS_CPP
#define __UTILS_CPP

#include <stdexcept>

#include "utils.h"
#include <iostream>

namespace utils
{
    template <typename T>
    void Stack<T>::Resize()
    {
        T* resizedStack = new T[stackSize * 2];
        for(size_t i=0; i<stackSize; i++)
            resizedStack[i] = stack[i];
        
        delete[] stack;
        stackSize *= 2;
        stack = resizedStack;
    }

    template <typename T>
    Stack<T>::Stack(int reservedSize)
    {
        stackSize = reservedSize;
        stack = new T[reservedSize];
        stackHead = -1;
    }
    template <typename T>
    Stack<T>::Stack() : Stack::Stack(StackDefaultSize) {}

    template <typename T>
    Stack<T>::~Stack() 
    {
        delete stack;
    }

    template <typename T>
    T& Stack<T>::Peek()
    {
        if(stackHead < 0)
            throw std::runtime_error("cannot peek stack: is empty");
        return stack[stackHead];
    }

    template <typename T>
    T Stack<T>::Pop()
    {
        if(stackHead < 0)
            throw std::runtime_error("cannot pop stack: is empty");
        return stack[stackHead--];
    }

    template <typename T>
    void Stack<T>::Push(T obj)
    {
        if(stackHead == static_cast<int>(stackSize-1))
            Resize();
        stack[++stackHead] = obj;
    }

    template <typename T>
    int Stack<T>::Size()
    {
        return stackHead + 1;
    }
}

#endif