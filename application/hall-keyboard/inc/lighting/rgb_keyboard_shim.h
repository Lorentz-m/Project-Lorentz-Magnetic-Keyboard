#ifndef RGB_KEYBOARD_SHIM_H
#define RGB_KEYBOARD_SHIM_H

#include <stdbool.h>
#include <stdint.h>
#include "device/keyboard_config.h"
#include "lighting/rgb_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** tick 差值转时间（本工程 tick 即毫秒） */
#define KEYBOARD_TICK_TO_TIME(tick_delta)   ((uint32_t)(tick_delta))

/** 灯效引擎使用的单键状态（不经过 hall_key_scan） */
typedef struct __Key
{
    bool report_state;          /**< 报告层是否按下（TRIGGER 等） */
    AnalogValue effective_analog; /**< 有效行程模拟量 0~ANALOG_VALUE_MAX */
} Key;

/** 灯效全局毫秒 tick，与 systick 同步 */
extern uint32_t g_keyboard_tick;

/** 按 key_id 取按键状态 */
Key *keyboard_get_key(uint16_t key_id);
/** 读取有效行程模拟量 */
AnalogValue keyboard_get_key_effective_analog_value(const Key *key);

void RgbKeyboardShim_Init(void);
void RgbKeyboardShim_SetTick(uint32_t tick_ms);
void RgbKeyboardShim_SetKeyState(uint16_t key_id, float intensity_0_1, uint8_t pressed);
void RgbKeyboardShim_ClearKeyState(uint16_t key_id);
void RgbKeyboardShim_ClearAllKeyStates(void);

#ifdef __cplusplus
}
#endif

#endif

