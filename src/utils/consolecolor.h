#ifndef __CONSOLECOLOR_H
#define __CONSOLECOLOR_H

// ---------------------------------------------------------------------------------------------------------------------
// Console

/// console display color
#ifdef _WIN32

const char* WIN32_CONSOLE_COLOR(int i);

#define CONSOLE_RESET       WIN32_CONSOLE_COLOR(7)
#define CONSOLE_BLACK       WIN32_CONSOLE_COLOR(8)
#define CONSOLE_RED         WIN32_CONSOLE_COLOR(4)
#define CONSOLE_GREEN       WIN32_CONSOLE_COLOR(10)
#define CONSOLE_YELLOW      WIN32_CONSOLE_COLOR(6)
#define CONSOLE_BLUE        WIN32_CONSOLE_COLOR(1)
#define CONSOLE_MAGENTA     WIN32_CONSOLE_COLOR(35)
#define CONSOLE_CYAN        WIN32_CONSOLE_COLOR(11)
#define CONSOLE_WHITE       WIN32_CONSOLE_COLOR(7)

#define CONSOLE_UNDERLINE   WIN32_CONSOLE_COLOR(-1)
#define CONSOLE_BOLD        WIN32_CONSOLE_COLOR(-1)

#define CONSOLE_C1          WIN32_CONSOLE_COLOR(-1)
#define CONSOLE_C2          WIN32_CONSOLE_COLOR(-1)
#define CONSOLE_C3          WIN32_CONSOLE_COLOR(-1)
#define CONSOLE_C4          WIN32_CONSOLE_COLOR(-1)
#define CONSOLE_C5          WIN32_CONSOLE_COLOR(-1)
#define CONSOLE_C6          WIN32_CONSOLE_COLOR(-1)
#define CONSOLE_C7          WIN32_CONSOLE_COLOR(-1)

#else

#define CONSOLE_RESET       "\033[0m"
#define CONSOLE_BLACK       "\033[30m"
#define CONSOLE_RED         "\033[31m"
#define CONSOLE_GREEN       "\033[32m"
#define CONSOLE_YELLOW      "\033[33m"
#define CONSOLE_BLUE        "\033[34m"
#define CONSOLE_MAGENTA     "\033[35m"
#define CONSOLE_CYAN        "\033[36m"
#define CONSOLE_WHITE       "\033[37m"

#define CONSOLE_UNDERLINE   "\033[4m"
#define CONSOLE_BOLD        "\033[1m"

#define CONSOLE_C1      "\033[38;2;184;187;38m"
#define CONSOLE_C2      "\033[38;2;215;153;33m"
#define CONSOLE_C3      "\033[38;2;141;165;165m"
#define CONSOLE_C4      "\033[38;2;211;134;155m"
#define CONSOLE_C5      "\033[38;2;142;192;124m"
#define CONSOLE_C6      "\033[38;2;204;26;29m"
#define CONSOLE_C7      "\033[38;2;124;111;100m"

#endif


#endif
