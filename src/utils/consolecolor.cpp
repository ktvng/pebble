#include <consolecolor.h>

const char* WIN32_CONSOLE_COLOR(int color)
{
#ifdef _WIN32
    if(color == -1)
    {
        return "";
    }
    
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);

    return "";
#else
    // effectively a NOP
    return "";
#endif
}