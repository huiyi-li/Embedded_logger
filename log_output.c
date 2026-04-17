#include "log_output.h"
#include <string.h>

static log_output_t *output_list = NULL;

void log_output_register(log_output_t *output)
{
    if (!output || !output->write) return;

    log_output_t *p = output_list;
    while (p) {
        if (p == output) return;
        p = p->next;
    }

    output->next = output_list;
    output_list = output;
}

void log_output_unregister(log_output_t *output)
{
    if (!output) return;

    if (output_list == output) {
        output_list = output->next;
        output->next = NULL;
        return;
    }

    log_output_t *p = output_list;
    while (p && p->next != output) {
        p = p->next;
    }

    if (p && p->next == output) {
        p->next = output->next;
        output->next = NULL;
    }
}

void log_output_broadcast(const uint8_t *buf, size_t len)
{
    log_output_t *p = output_list;
    while (p) {
        if (p->write) {
            p->write(buf, len);
        }
        p = p->next;
    }
}

#ifdef LOG_OUTPUT_UART_ENABLE
#include "peripheral_uart.h"
#include "peripheral_sysctrl.h"

static void _uart_write(const uint8_t *buf, size_t len)
{
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_20)
    UART_WriteBytes(UART_PORT_0, (uint8_t *)buf, (uint32_t)len);
#else
    for (size_t i = 0; i < len; i++) {
        while (UART_GetTXFIFOAvailable(UART_PORT_0) == 0);
        UART_WriteByte(UART_PORT_0, buf[i]);
    }
#endif
}

static log_output_t _uart_output;

void log_output_uart_init(log_output_t *output)
{
    (void)output;
    _uart_output.write = _uart_write;
    _uart_output.next = NULL;
    log_output_register(&_uart_output);
}
#else

void log_output_uart_init(log_output_t *output)
{
    (void)output;
}

#endif

#ifdef LOG_OUTPUT_SWO_ENABLE

static void _swo_write(const uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        ITM_SendChar(buf[i]);
    }
}

static log_output_t _swo_output;

void log_output_swo_init(log_output_t *output)
{
    (void)output;
    _swo_output.write = _swo_write;
    _swo_output.next = NULL;
    log_output_register(&_swo_output);
}
#else

void log_output_swo_init(log_output_t *output)
{
    (void)output;
}

#endif
