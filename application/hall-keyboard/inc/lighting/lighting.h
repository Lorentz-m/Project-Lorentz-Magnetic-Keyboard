/**
 * @file lighting.h
 * @brief 灯效门面：初始化、轮询与对外兼容封装
 */
#ifndef LIGHTING_H
#define LIGHTING_H

#include <stdint.h>
#include "device/keyboard_config.h"
#include "lighting/rgb.h"
#include "lighting/lighting_mode.h"
#include "lighting/lighting_color.h"
#include "lighting/lighting_brightness.h"
#include "lighting/lighting_speed.h"
#include "lighting/lighting_key.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIGHTING_BOOT_ON_DELAY_MS
/** ms after power-on before Lighting_Poll may drive LEDs */
#define LIGHTING_BOOT_ON_DELAY_MS  (2300u)
#endif

/** @brief 灯效子系统初始化（布局、引擎、设置子模块、事件订阅） */
void Lighting_Init(void);

/** @brief 主循环轮询：同步 tick 并按约 15ms 节流调用 rgb_process */
void Lighting_Poll(void);

#ifdef __cplusplus
}
#endif

#endif /* LIGHTING_H */
