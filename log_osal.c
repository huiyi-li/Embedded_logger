#include "log_osal.h"
#include <stdint.h>

static void _dummy_lock(void) {}
static void _dummy_unlock(void) {}
static uint32_t _dummy_tick(void) { return 0; }
static uint32_t _dummy_tick_ms(void) { return 0; }
static int _dummy_in_isr(void) { return 0; }
static void _dummy_flush_task(void) {}

void log_osal_default_init(log_osal_t *osal)
{
    if (!osal) return;
    osal->lock           = _dummy_lock;
    osal->unlock         = _dummy_unlock;
    osal->get_tick       = _dummy_tick;
    osal->get_tick_ms    = _dummy_tick_ms;
    osal->in_isr         = _dummy_in_isr;
    osal->flush_task_hook = _dummy_flush_task;
}
