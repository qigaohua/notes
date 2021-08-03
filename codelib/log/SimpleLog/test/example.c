#include <stdio.h>
// #include <stdlib.h>
#include <unistd.h>


#include "module1.h"
#include "module2.h"

// #define SLOG_MOUDLE_LEVEL   SLOG_LEVEL_WARN
#define SLOG_MOUDLE_TAG     "test"
#include "../log.h"



int main(int argc, char *argv[])
{
    // SLogerInit(SLOG_LEVEL_INFO, 0,  NULL);
    // SLogerInit(SLOG_LEVEL_DEBUG, 0, "/home/qigaohua/work/notes/codelib/log/SimpleLog/test.log");
    SLogerInit(SLOG_LEVEL_DEBUG, SLOG_FMT_LEVEL | SLOG_FMT_TID, NULL);

    // SLogerSetMouldeLogFilterTag("test", SLOG_LEVEL_WARN);
    // SLogerSetMouldeLogFilterTag("moudle1", SLOG_LEVEL_ERROR);
    // SLogerSetMouldeLogFilterTag("moudle2", SLOG_LEVEL_ASSERT);

    // for (int i = 0; i < 128; i++) {
        slog_a("It's %s log", "assert");
        slog_e("It's %s log", "error");
        // sleep(1);
    // }

    module1();

    BUG_ON(1 == 1);

    slog_w("It's %s log", "warn");
    slog_m("It's %s log", "info");
    slog_d("It's %s log", "debug");

    module2();

    SLogerDeinit();
    return 0;
}
