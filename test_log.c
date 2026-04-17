#include "log_system.h"
#include "log_output.h"
#include <stdio.h>

static void _console_write(const uint8_t *buf, size_t len)
{
    fwrite(buf, 1, len, stdout);
    fflush(stdout);
}

static log_output_t g_console_output;

int main(void)
{
    log_system_init();

    g_console_output.write = _console_write;
    g_console_output.next = NULL;
    log_output_register(&g_console_output);

    LOG_D("This is a DEBUG message: %d", 42);
    LOG_I("System initialized, version %s", "1.0.0");
    LOG_W("Warning: temperature %d exceeds threshold %d", 85, 80);
    LOG_E("Error: peripheral 0x%08X not responding", 0x40002000);
    LOG_F("Fatal: stack overflow at 0x%08X", 0x20001FF0);

    log_set_level(LOG_LEVEL_WARN);
    LOG_D("This should NOT appear (filtered by level)");
    LOG_I("This should NOT appear either");
    LOG_W("This WARN should appear after level change");
    LOG_E("This ERROR should appear after level change");

    log_set_level(LOG_LEVEL_DEBUG);
    log_set_enable(0);
    LOG_I("This should NOT appear (disabled)");
    log_set_enable(1);
    LOG_I("Log re-enabled, this should appear");

    log_flush();

    printf("\n--- Log system test completed ---\n");
    return 0;
}
