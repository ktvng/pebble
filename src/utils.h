#ifndef __UTILS_H
#define __UTILS_H

namespace utils
{
    template <typename T>
    class Stack
    {
    private:
        T* stack;
        int stackHead;
        size_t stackSize;

        void Resize();
    public:
        constexpr static int StackDefaultSize = 16;

        Stack(int reservedSize);

        Stack();

        T& Peek();
        T Pop();
        void Push(T obj);
        int Size();
    };
}
#include "utils.cpp"
#endif