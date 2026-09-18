#ifndef SOFT_TIMER_H_
#define SOFT_TIMER_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SOFT_TIMER_MAX
#define SOFT_TIMER_MAX 16u
#endif

/** 单次 start 可携带的上下文最大字节数（拷贝进模块内部，回调时只读） */
#ifndef SOFT_TIMER_CTX_BYTES
#define SOFT_TIMER_CTX_BYTES 16u
#endif

/** 无效定时器 ID（soft_timer_start 失败时返回） */
#define SOFT_TIMER_INVALID_ID 0u

typedef uint8_t soft_timer_id_t;

/**
 * 到期回调
 * @param id       定时器 ID
 * @param ctx      启动时传入的上下文副本（只读，ctx_len 为 0 时可为 NULL）
 * @param ctx_len  有效上下文字节数，不超过 SOFT_TIMER_CTX_BYTES
 */
typedef void (*soft_timer_cb_t)(soft_timer_id_t id, const void *ctx, uint8_t ctx_len);

typedef enum {
    EN_SOFT_TIMER_MODE_ONCE = 0,   /* 到期执行一次后自动停止 */
    EN_SOFT_TIMER_MODE_REPEAT,     /* 按周期重复执行 */
} EN_SOFT_TIMER_MODE;

/** 清空全部定时器槽，须在 systick_init 之后、Start 之前调用一次 */
void soft_timer_init(void);

/**
 * 启动软件定时器
 * @param period_ms  周期/延时（毫秒），须 > 0
 * @param cb         到期回调（可在此回调内调用 soft_timer_stop）
 * @param ctx        上下文数据，将拷贝到模块内；可为 NULL（仅当 ctx_len 为 0）
 * @param ctx_len    上下文长度，不得超过 SOFT_TIMER_CTX_BYTES
 * @param mode       单次或循环
 * @return 定时器 ID；失败返回 SOFT_TIMER_INVALID_ID
 */
soft_timer_id_t soft_timer_start(uint32_t period_ms, soft_timer_cb_t cb, const void *ctx,
    uint8_t ctx_len, EN_SOFT_TIMER_MODE mode);

/**
 * 停止指定定时器（可从回调内或外部调用，对已停止的 ID 调用无害）
 * @param id soft_timer_start 返回值
 */
void soft_timer_stop(soft_timer_id_t id);

/** 查询定时器是否仍在运行 */
bool soft_timer_is_running(soft_timer_id_t id);

/**
 * 轮询处理到期定时器，须在主循环中周期性调用（依赖 systick_get_ms）
 */
void soft_timer_process(void);

#ifdef __cplusplus
}
#endif

#endif /* SOFT_TIMER_H_ */
