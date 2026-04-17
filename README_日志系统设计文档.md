# 日志系统（log_output）设计文档

## 1. 组件功能概述

`log_output` 是一套轻量级日志系统，依赖以下文件：

| 文件 | 说明 |
|------|------|
| `log_system.h` | 主头文件，包含宏定义、API 声明、日志宏 |
| `log_system.c` | 核心实现，同步/异步日志、格式化、日志级别过滤 |
| `log_osal.h` / `log_osal.c` | OS 抽象层，时间戳、锁、ISR 判定接口 |
| `log_output.h` / `log_output.c` | 输出介质抽象，UART/USB 广播 |
| `ring_buffer.h` / `ring_buffer.c` | 环形缓冲（异步模式使用） |

核心特性：

- **同步/异步双模式**：ERROR 及以上级别强制同步输出，低级别异步缓冲
- **编译期裁剪**：可关闭时间戳、函数名、上下文信息
- **颜色输出**：`LOG_COLOR_ENABLE=1` 时各日志级别自动匹配 ANSI 颜色
- **环形缓冲**：异步模式使用静态 ring buffer，无动态内存分配
- **多 OS 支持**：通过 `log_osal_t` 结构注入平台相关实现

---

## 2. 接口使用说明

### 2.1 日志宏

```c
LOG_D(fmt, ...)  // DEBUG 级别，编译期过滤：LOG_LEVEL <= LOG_LVL_DEBUG 时有效
LOG_I(fmt, ...)  // INFO  级别，编译期过滤：LOG_LEVEL <= LOG_LVL_INFO  时有效
LOG_W(fmt, ...)  // WARN  级别，编译期过滤：LOG_LEVEL <= LOG_LVL_WARN  时有效
LOG_E(fmt, ...)  // ERROR 级别，编译期过滤：LOG_LEVEL <= LOG_LVL_ERROR 时有效
LOG_F(fmt, ...)  // FATAL 级别，始终同步输出
```

**编译期过滤**：设置 `LOG_LEVEL` 可在编译时剔除不需要的日志宏，零运行时开销。

### 2.2 API 函数

```c
void log_system_init(void);           // 初始化日志系统（环形缓冲/osal）
void log_system_deinit(void);         // 去初始化，冲洗所有缓冲日志

void log_set_level(log_level_t level);     // 运行时设置日志级别阈值
log_level_t log_get_level(void);           // 获取当前日志级别

void log_set_enable(int enable);      // 运行时全局开关（0 = 禁用所有输出）
int  log_get_enable(void);

void log_set_osal(const log_osal_t *osal);  // 注入 OSAL 实现
void log_get_context(log_context_t *ctx);    // 获取 ISR/任务上下文

void log_flush(void);                 // 手动冲洗当前缓冲日志
void log_flush_all(void);             // 强制冲洗所有缓冲（包括未满的缓冲）

#if LOG_ASYNC
void log_flush_task(void);            // 异步任务入口，供后台线程/定时器调用
#endif
```

### 2.3 OSAL 接口

```c
typedef struct {
    uint32_t (*get_tick)(void);       // 获取系统节拍
    uint32_t (*get_tick_ms)(void);    // 获取系统节拍（毫秒）
    uint8_t  (*in_isr)(void);         // 判断当前是否在 ISR 上下文中
    void     (*lock)(void);            // 获取锁（可选）
    void     (*unlock)(void);          // 释放锁（可选）
} log_osal_t;

void log_osal_default_init(log_osal_t *osal); // 使用默认值填充
```

### 2.4 编译配置宏

| 宏名 | 默认值 | 说明 |
|------|--------|------|
| `LOG_LEVEL` | `LOG_LVL_INFO` | 编译期日志级别阈值 |
| `LOG_ASYNC` | 1 | 1 = 异步模式（ring buffer），0 = 同步模式 |
| `LOG_SYNC_ERROR` | 1 | ERROR 及以上级别强制同步输出 |
| `LOG_BUFFER_SIZE` | 1024 | 异步模式环形缓冲大小（字节） |
| `LOG_FORMAT_BUF_SIZE` | 128 | 单条日志格式化缓冲区大小 |
| `LOG_ENABLE_TIMESTAMP` | 1 | 1 = 输出时间戳 |
| `LOG_ENABLE_LOCATION` | 1 | 1 = 输出 `file:line` 位置 |
| `LOG_ENABLE_FUNC` | 0 | 1 = 输出函数名 |
| `LOG_ENABLE_CONTEXT` | 1 | 1 = 输出 ISR/任务上下文标记 |
| `LOG_COLOR_ENABLE` | 1 | 1 = 启用日志颜色 |

### 2.5 颜色配置宏

| 宏名 | 默认值 | 说明 |
|------|--------|------|
| `LOG_COLOR_ENABLE` | 1 | 主开关，0 = 关闭所有颜色 |
| `LOG_COLOR_RESET` | `"\033[0m"` | 重置为终端默认颜色 |
| `LOG_COLOR_DEBUG` | `"\033[36m"` | 青色（Cyan） |
| `LOG_COLOR_INFO` | `"\033[32m"` | 绿色（Green） |
| `LOG_COLOR_WARN` | `"\033[33m"` | 黄色（Yellow） |
| `LOG_COLOR_ERROR` | `"\033[31m"` | 红色（Red） |
| `LOG_COLOR_FATAL` | `"\033[35m"` | 紫色（Magenta） |
| `LOG_COLOR_DEFAULT` | `"\033[0m"` | 默认颜色（同 RESET） |

---

## 3. 测试用例说明

### 3.1 测试场景：基础日志输出

**代码**：
```c
log_system_init();

LOG_I("System initialized, version %s", "1.0.0");
LOG_W("Warning: temperature %d exceeds threshold %d", 85, 80);
LOG_E("Error: peripheral 0x%08X not responding", 0x40002000);
```

**预期输出**（颜色开启时）：
```
[00000000] [I] test_log.c:22 main() System initialized, version 1.0.0
[00000000] [W] test_log.c:23 main() Warning: temperature 85 exceeds threshold 80
[00000000] [E] test_log.c:24 main() Error: peripheral 0x40002000 not responding
```
（INFO 为绿色，WARN 为黄色，ERROR 为红色）

### 3.2 测试场景：运行时日志级别调整

**代码**：
```c
log_system_init();

LOG_I("This INFO should appear");
LOG_W("This WARN should appear");
LOG_E("This ERROR should appear");

log_set_level(LOG_LEVEL_WARN);  // 提升阈值

LOG_I("This INFO should NOT appear");
LOG_W("This WARN should appear");
LOG_E("This ERROR should appear");
```

**预期输出**：
```
This INFO should appear
This WARN should appear
This ERROR should appear
This WARN should appear
This ERROR should appear
```

### 3.3 测试场景：颜色关闭

**编译**：`gcc -DLOG_COLOR_ENABLE=0 ...`

**预期输出**：所有 ANSI 转义序列被剔除，日志为终端默认颜色，无 `\033[36m` 等控制码。

### 3.4 测试场景：异步缓冲与手动冲洗

**代码**：
```c
log_system_init();  // LOG_ASYNC=1 时启用 ring buffer

LOG_D("Debug: trace point 1");
LOG_D("Debug: trace point 2");
// 此时 DEBUG 日志在 ring buffer 中，未输出

log_flush_task();   // 冲洗异步缓冲
```

**预期输出**：冲洗后两个 DEBUG 日志同时出现在终端。

### 3.5 测试场景：ERROR 强制同步输出

**代码**：
```c
// LOG_SYNC_ERROR=1, LOG_ASYNC=1
LOG_D("Debug message");   // 进入 ring buffer，异步
LOG_E("Error message");  // 绕过 ring buffer，同步立即输出
```

**预期**：ERROR 日志不受异步缓冲影响，实时输出。

---

## 4. 使用注意事项

1. **异步模式内存**：`LOG_ASYNC=1` 时需确保 `LOG_BUFFER_SIZE` 足够容纳峰值日志量。环形缓冲满时新日志被丢弃（`ring_fifo_put` 返回写入字节数，可用于监控）。

2. **`LOG_FORMAT_BUF_SIZE`**：单条日志超过此值时被截断。建议至少 128 字节，带长字符串时增大。

3. **多任务环境**：`log_osal.lock/unlock` 应使用任务级锁（如 FreeRTOS 的 `taskENTER_CRITICAL()`）而非中断锁，防止日志输出被任务切换打断。

4. **ISR 上下文输出**：ERROR/FATAL 日志在 ISR 中会被强制同步输出，需确保 `log_output_broadcast` 在 ISR 中可重入。

5. **颜色与终端兼容**：ANSI 颜色转义序列在 Windows 10+ 控制台、VS Code 终端、Linux/macOS 终端均支持。传统串口工具（如超级终端）可能将颜色码显示为乱码，此时设置 `LOG_COLOR_ENABLE=0`。

6. **时间戳来源**：`log_osal.get_tick_ms` 应返回系统启动后的毫秒数。如未提供，`get_tick` 被调用并假定 1 tick = 1ms。

7. **编译期裁剪**：关闭不需要的元数据（`LOG_ENABLE_FUNC=0`、`LOG_ENABLE_CONTEXT=0`）可减少单条日志长度，降低缓冲溢出风险。

---

## 5. CMake 集成示例

```cmake
# 启用日志颜色（默认已开启）
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    LOG_COLOR_ENABLE=1
)

# 设置编译期日志级别（不输出 DEBUG）
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    LOG_LEVEL=LOG_LVL_INFO
)

# 关闭异步模式，使用同步日志
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    LOG_ASYNC=0
)

# 自定义 ERROR 颜色（紫色前景 + 白色背景）
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    LOG_COLOR_ERROR="\"\033[35;47m\""
)
```

## 6. 自定义 OSAL 示例

```c
static log_osal_t my_osal;

void my_lock(void)   { taskENTER_CRITICAL(); }
void my_unlock(void) { taskEXIT_CRITICAL(); }
uint32_t my_tick_ms(void) { return xTaskGetTickCount() * portTICK_PERIOD_MS; }
uint8_t my_in_isr(void) { return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0; }

my_osal.get_tick_ms = my_tick_ms;
my_osal.in_isr      = my_in_isr;
my_osal.lock        = my_lock;
my_osal.unlock      = my_unlock;

log_system_init();
log_set_osal(&my_osal);
```
