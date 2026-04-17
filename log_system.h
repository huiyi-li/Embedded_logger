#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The color for terminal (foreground)
 * BLACK    30
 * RED      31
 * GREEN    32
 * YELLOW   33
 * BLUE     34
 * PURPLE   35
 * CYAN     36
 * WHITE    37
 */

#define LOG_LVL_DEBUG  0
#define LOG_LVL_INFO   1
#define LOG_LVL_WARN   2
#define LOG_LVL_ERROR  3
#define LOG_LVL_FATAL  4
#define LOG_LVL_NONE   5

#ifndef LOG_LEVEL
#define LOG_LEVEL   LOG_LVL_INFO
#endif

#ifndef LOG_ASYNC
#define LOG_ASYNC   1
#endif

#ifndef LOG_SYNC_ERROR
#define LOG_SYNC_ERROR  1
#endif

#ifndef LOG_DIRECT
#define LOG_DIRECT          1
#endif

#ifndef LOG_DIRECT_PRINTF
#define LOG_DIRECT_PRINTF   printf
#endif

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE  1024
#endif

#ifndef LOG_FORMAT_BUF_SIZE
#define LOG_FORMAT_BUF_SIZE  128
#endif

#ifndef LOG_ENABLE_TIMESTAMP
#define LOG_ENABLE_TIMESTAMP  1
#endif

#ifndef LOG_ENABLE_LOCATION
#define LOG_ENABLE_LOCATION   1
#endif

#ifndef LOG_ENABLE_FUNC
#define LOG_ENABLE_FUNC       0
#endif

#ifndef LOG_ENABLE_CONTEXT
#define LOG_ENABLE_CONTEXT    1
#endif

#ifndef LOG_COLOR_ENABLE
#define LOG_COLOR_ENABLE      1
#endif

#ifndef LOG_COLOR_RESET
#define LOG_COLOR_RESET        "\033[0m"
#endif

#ifndef LOG_COLOR_DEBUG
#define LOG_COLOR_DEBUG       "\033[36m"
#endif

#ifndef LOG_COLOR_INFO
#define LOG_COLOR_INFO        "\033[32m"
#endif

#ifndef LOG_COLOR_WARN
#define LOG_COLOR_WARN        "\033[33m"
#endif

#ifndef LOG_COLOR_ERROR
#define LOG_COLOR_ERROR       "\033[31m"
#endif

#ifndef LOG_COLOR_FATAL
#define LOG_COLOR_FATAL       "\033[35m"
#endif

#ifndef LOG_COLOR_DEFAULT
#define LOG_COLOR_DEFAULT      "\033[0m"
#endif

typedef enum {
    LOG_LEVEL_DEBUG = LOG_LVL_DEBUG,
    LOG_LEVEL_INFO  = LOG_LVL_INFO,
    LOG_LEVEL_WARN  = LOG_LVL_WARN,
    LOG_LEVEL_ERROR = LOG_LVL_ERROR,
    LOG_LEVEL_FATAL = LOG_LVL_FATAL,
    LOG_LEVEL_NONE  = LOG_LVL_NONE,
} log_level_t;

typedef struct {
    uint8_t     in_isr;
    uint8_t     task_id;
} log_context_t;

void log_system_init(void);
void log_system_deinit(void);

void log_set_level(log_level_t level);
log_level_t log_get_level(void);
void log_set_enable(int enable);
int  log_get_enable(void);

void log_write(log_level_t level,
               const char *file, int line, const char *func,
               const char *fmt, ...);

void log_flush(void);
void log_flush_all(void);

void log_set_osal(const void *osal);
void log_get_context(log_context_t *ctx);

#if LOG_ASYNC
void log_flush_task(void);
#endif

#if LOG_DIRECT

#if LOG_COLOR_ENABLE

#define _LOG_DIRECT_PRINT_D(fmt, ...)  LOG_DIRECT_PRINTF(LOG_COLOR_DEBUG "[D] " fmt "\r\n" LOG_COLOR_RESET, ##__VA_ARGS__)
#define _LOG_DIRECT_PRINT_I(fmt, ...)  LOG_DIRECT_PRINTF(LOG_COLOR_INFO  "[I] " fmt "\r\n" LOG_COLOR_RESET, ##__VA_ARGS__)
#define _LOG_DIRECT_PRINT_W(fmt, ...)  LOG_DIRECT_PRINTF(LOG_COLOR_WARN  "[W] " fmt "\r\n" LOG_COLOR_RESET, ##__VA_ARGS__)
#define _LOG_DIRECT_PRINT_E(fmt, ...)  LOG_DIRECT_PRINTF(LOG_COLOR_ERROR "[E] " fmt "\r\n" LOG_COLOR_RESET, ##__VA_ARGS__)
#define _LOG_DIRECT_PRINT_F(fmt, ...)  LOG_DIRECT_PRINTF(LOG_COLOR_FATAL "[F] " fmt "\r\n" LOG_COLOR_RESET, ##__VA_ARGS__)

#else

#define _LOG_DIRECT_PRINT_D(fmt, ...)  LOG_DIRECT_PRINTF("[D] " fmt "\r\n", ##__VA_ARGS__)
#define _LOG_DIRECT_PRINT_I(fmt, ...)  LOG_DIRECT_PRINTF("[I] " fmt "\r\n", ##__VA_ARGS__)
#define _LOG_DIRECT_PRINT_W(fmt, ...)  LOG_DIRECT_PRINTF("[W] " fmt "\r\n", ##__VA_ARGS__)
#define _LOG_DIRECT_PRINT_E(fmt, ...)  LOG_DIRECT_PRINTF("[E] " fmt "\r\n", ##__VA_ARGS__)
#define _LOG_DIRECT_PRINT_F(fmt, ...)  LOG_DIRECT_PRINTF("[F] " fmt "\r\n", ##__VA_ARGS__)

#endif

#if LOG_LEVEL <= LOG_LVL_DEBUG
#define LOG_D(fmt, ...)  _LOG_DIRECT_PRINT_D(fmt, ##__VA_ARGS__)
#else
#define LOG_D(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_INFO
#define LOG_I(fmt, ...)  _LOG_DIRECT_PRINT_I(fmt, ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_WARN
#define LOG_W(fmt, ...)  _LOG_DIRECT_PRINT_W(fmt, ##__VA_ARGS__)
#else
#define LOG_W(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_ERROR
#define LOG_E(fmt, ...)  _LOG_DIRECT_PRINT_E(fmt, ##__VA_ARGS__)
#else
#define LOG_E(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_FATAL
#define LOG_F(fmt, ...)  _LOG_DIRECT_PRINT_F(fmt, ##__VA_ARGS__)
#else
#define LOG_F(fmt, ...)  ((void)0)
#endif

#else

#if LOG_LEVEL <= LOG_LVL_DEBUG
#define LOG_D(fmt, ...)  log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#else
#define LOG_D(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_INFO
#define LOG_I(fmt, ...)  log_write(LOG_LEVEL_INFO,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_WARN
#define LOG_W(fmt, ...)  log_write(LOG_LEVEL_WARN,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#else
#define LOG_W(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_ERROR
#define LOG_E(fmt, ...)  log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#else
#define LOG_E(fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_FATAL
#define LOG_F(fmt, ...)  log_write(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#else
#define LOG_F(fmt, ...)  ((void)0)
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
