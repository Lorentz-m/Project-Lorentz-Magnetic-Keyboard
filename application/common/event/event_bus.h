#ifndef EVENT_BUS_H_
#define EVENT_BUS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "event_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EVENT_BUS_QUEUE_SIZE
#define EVENT_BUS_QUEUE_SIZE 16
#endif

#ifndef EVENT_BUS_MAX_SUBSCRIBERS
#define EVENT_BUS_MAX_SUBSCRIBERS 64
#endif

/* 订阅表项（固定槽 id == 下标） */
typedef struct {
    uint8_t      used;        /* 非 0 = 槽位占用 */
    uint8_t      type_mask;   /* 监听的事件类型掩码 */
    EventHandler handler;     /* 回调函数 */
} st_event_subscription;

/* 初始化事件总线 */
void event_bus_init(void);

/* 发布事件（同步） - 立即分发，支持值或指针传递 */
bool event_bus_publish_sync(uint8_t type, un_event_param param);
/* 发布事件（异步） - 入队等待dispatch，建议只使用param.value */
bool event_bus_publish_async(uint8_t type, un_event_param param);

/* 订阅：成功写入 out_id（槽位下标）；out_id 可为 NULL */
bool event_bus_subscribe(uint8_t type_mask, EventHandler handler, uint8_t *out_id);

/* 取消订阅：type_mask 与 subscribe 一致（如 MOD_KEY / EVT_*）；id 为返回的槽位 */
bool event_bus_unsubscribe(uint8_t type_mask, uint8_t id);

/* 分发事件 - 主循环调用 */
void event_bus_dispatch(void);

/* 获取待处理事件数 */
uint8_t event_bus_pending_count(void);

#ifdef __cplusplus
}
#endif

#endif /* EVENT_BUS_H_ */
