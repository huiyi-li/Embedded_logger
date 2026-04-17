#include "log_system.h"
#include "log_osal.h"
#include "log_output.h"
#include "ring_buffer.h"

#include <stdio.h>
#include <string.h>

static log_level_t  g_log_level    = LOG_LEVEL;
static int          g_log_enabled  = 1;
static log_osal_t   g_log_osal;

#if LOG_ASYNC
static uint8_t      g_ring_buf_mem[LOG_BUFFER_SIZE];
static ring_fifo_t  g_ring_fifo;
#endif

static char         g_fmt_buf[LOG_FORMAT_BUF_SIZE];

static const char * const g_level_str[] = {
    "D", "I", "W", "E", "F", "N"
};

static const char * const _level_color_map[] = {
    LOG_COLOR_DEBUG,
    LOG_COLOR_INFO,
    LOG_COLOR_WARN,
    LOG_COLOR_ERROR,
    LOG_COLOR_FATAL,
    LOG_COLOR_RESET,
};

static const char * const _color_reset = LOG_COLOR_RESET;

static const char *_basename(const char *path)
{
    const char *p = strrchr(path, '/');
    const char *q = strrchr(path, '\\');
    if (p && q) return (p > q ? p : q) + 1;
    if (p) return p + 1;
    if (q) return q + 1;
    return path;
}

static int _append_color_reset(char *buf, int pos, size_t buf_size, log_level_t level)
{
#if LOG_COLOR_ENABLE
    (void)level;
    if (level <= LOG_LEVEL_FATAL) {
        pos += snprintf(buf + pos, buf_size - pos, "%s", _color_reset);
    }
#else
    (void)buf;
    (void)pos;
    (void)buf_size;
    (void)level;
#endif
    return pos;
}

void log_system_init(void)
{
    log_osal_default_init(&g_log_osal);

#if LOG_ASYNC
    ring_fifo_init_static(&g_ring_fifo, g_ring_buf_mem, LOG_BUFFER_SIZE);
#endif
}

void log_system_deinit(void)
{
    log_flush_all();
}

void log_set_level(log_level_t level)
{
    g_log_level = level;
}

log_level_t log_get_level(void)
{
    return g_log_level;
}

void log_set_enable(int enable)
{
    g_log_enabled = enable;
}

int log_get_enable(void)
{
    return g_log_enabled;
}

void log_set_osal(const void *osal)
{
    if (osal) {
        memcpy(&g_log_osal, osal, sizeof(log_osal_t));
    }
}

void log_get_context(log_context_t *ctx)
{
    if (!ctx) return;
    ctx->in_isr = 0;
    ctx->task_id = 0;
    if (g_log_osal.in_isr) {
        ctx->in_isr = (uint8_t)g_log_osal.in_isr();
    }
}

static int _format_header(
    char *buf, size_t buf_size,
    log_level_t level,
    const char *file, int line, const char *func)
{
    int pos = 0;
    (void)func;

#if LOG_COLOR_ENABLE
    if (level <= LOG_LEVEL_FATAL) {
        pos += snprintf(buf + pos, buf_size - pos, "%s", _level_color_map[level]);
    }
#endif

#if LOG_ENABLE_TIMESTAMP
    {
        uint32_t ms = 0;
        if (g_log_osal.get_tick_ms) {
            ms = g_log_osal.get_tick_ms();
        } else if (g_log_osal.get_tick) {
            ms = g_log_osal.get_tick();
        }
        pos += snprintf(buf + pos, buf_size - pos, "[%08lu] ", (unsigned long)ms);
    }
#endif

    if (level <= LOG_LEVEL_FATAL) {
        pos += snprintf(buf + pos, buf_size - pos, "[%s] ", g_level_str[level]);
    }

#if LOG_ENABLE_CONTEXT
    {
        log_context_t ctx;
        log_get_context(&ctx);
        if (ctx.in_isr) {
            pos += snprintf(buf + pos, buf_size - pos, "[ISR] ");
        } else {
            pos += snprintf(buf + pos, buf_size - pos, "[T%u] ", ctx.task_id);
        }
    }
#endif

#if LOG_ENABLE_LOCATION
    pos += snprintf(buf + pos, buf_size - pos, "%s:%d ", _basename(file), line);
#endif

#if LOG_ENABLE_FUNC
    pos += snprintf(buf + pos, buf_size - pos, "%s() ", func);
#endif

    return pos;
}

static void _log_output_sync(const char *data, size_t len)
{
    log_output_broadcast((const uint8_t *)data, len);
}

#if LOG_ASYNC
static void _log_output_async(const char *data, size_t len)
{
    uint32_t written = ring_fifo_put(&g_ring_fifo, (const uint8_t *)data, (uint32_t)len);
    (void)written;
}
#endif

void log_write(log_level_t level,
               const char *file, int line, const char *func,
               const char *fmt, ...)
{
    if (!g_log_enabled) return;
    if (level < g_log_level) return;

    int pos = _format_header(g_fmt_buf, LOG_FORMAT_BUF_SIZE, level, file, line, func);

    va_list ap;
    va_start(ap, fmt);
    pos += vsnprintf(g_fmt_buf + pos, LOG_FORMAT_BUF_SIZE - pos, fmt, ap);
    va_end(ap);

    if (pos >= LOG_FORMAT_BUF_SIZE - 2) {
        pos = LOG_FORMAT_BUF_SIZE - 2;
    }

    pos = _append_color_reset(g_fmt_buf, pos, LOG_FORMAT_BUF_SIZE, level);

    g_fmt_buf[pos++] = '\r';
    g_fmt_buf[pos++] = '\n';

    int is_sync = 0;

#if LOG_SYNC_ERROR
    if (level >= LOG_LEVEL_ERROR) {
        is_sync = 1;
    }
#endif

    if (is_sync) {
        g_log_osal.lock ? g_log_osal.lock() : (void)0;
        _log_output_sync(g_fmt_buf, (size_t)pos);
        g_log_osal.unlock ? g_log_osal.unlock() : (void)0;
    } else {
#if LOG_ASYNC
        g_log_osal.lock ? g_log_osal.lock() : (void)0;
        _log_output_async(g_fmt_buf, (size_t)pos);
        g_log_osal.unlock ? g_log_osal.unlock() : (void)0;
#else
        g_log_osal.lock ? g_log_osal.lock() : (void)0;
        _log_output_sync(g_fmt_buf, (size_t)pos);
        g_log_osal.unlock ? g_log_osal.unlock() : (void)0;
#endif
    }
}

#if LOG_ASYNC
void log_flush_task(void)
{
    uint8_t tmp[64];
    while (!ring_fifo_is_empty(&g_ring_fifo)) {
        uint32_t len = ring_fifo_get(&g_ring_fifo, tmp, sizeof(tmp));
        if (len > 0) {
            _log_output_sync((const char *)tmp, (size_t)len);
        }
    }
}
#endif

void log_flush(void)
{
#if LOG_ASYNC
    log_flush_task();
#endif
}

void log_flush_all(void)
{
#if LOG_ASYNC
    int max_iter = LOG_BUFFER_SIZE + 1;
    while (!ring_fifo_is_empty(&g_ring_fifo) && max_iter-- > 0) {
        log_flush_task();
    }
#endif
}
