#ifndef LOG_OSAL_H
#define LOG_OSAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOG_OSAL_CUSTOM

typedef void (*log_osal_lock_t)(void);
typedef void (*log_osal_unlock_t)(void);
typedef uint32_t (*log_osal_tick_t)(void);
typedef uint32_t (*log_osal_tick_ms_t)(void);
typedef int (*log_osal_in_isr_t)(void);
typedef void (*log_osal_flush_task_t)(void);

typedef struct {
    log_osal_lock_t     lock;
    log_osal_unlock_t   unlock;
    log_osal_tick_t     get_tick;
    log_osal_tick_ms_t  get_tick_ms;
    log_osal_in_isr_t   in_isr;
    log_osal_flush_task_t flush_task_hook;
} log_osal_t;

void log_osal_default_init(log_osal_t *osal);

#else

typedef struct log_osal log_osal_t;

#endif

#ifdef __cplusplus
}
#endif

#endif
