#include <consolecolor.h>

#ifdef _WIN32
#include <windows.h>
#endif

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