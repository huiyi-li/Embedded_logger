#ifndef LOG_OUTPUT_H
#define LOG_OUTPUT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*log_output_write_fn)(const uint8_t *buf, size_t len);

typedef struct log_output {
    log_output_write_fn write;
    struct log_output  *next;
} log_output_t;

void log_output_register(log_output_t *output);
void log_output_unregister(log_output_t *output);
void log_output_broadcast(const uint8_t *buf, size_t len);

void log_output_uart_init(log_output_t *output);
void log_output_swo_init(log_output_t *output);

#ifdef __cplusplus
}
#endif

#endif
