/**
 * @file lighting_event.h
 * @brief 灯效相关事件总线订阅与分发
 */
#ifndef LIGHTING_EVENT_H
#define LIGHTING_EVENT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 注册表中全部灯效事件监听 */
bool Lighting_EventInit(void);

/** @brief 注销表中全部已订阅事件 */
void Lighting_EventDeinit(void);

#ifdef __cplusplus
}
#endif

#endif /* LIGHTING_EVENT_H */
