#include <stdio.h>
// #include <stdlib.h>

// #define SLOG_MOUDLE_LEVEL   SLOG_LEVEL_WARN
#define SLOG_MOUDLE_TAG     "moudle2"
#include "../log.h"



int module2()
{
    // SLogerInit(SLOG_LEVEL_INFO, 0,  NULL);
    // SLogerInit(0, 0, NULL);


    slog_a("It's %s log", "assert");
    slog_e("It's %s log", "error");

    // SLogerSetMouldeLogFilterTag("test");

    slog_w("It's %s log", "warn");
    slog_m("It's %s log", "info");
    slog_d("It's %s log", "debug");

    return 0;
}
