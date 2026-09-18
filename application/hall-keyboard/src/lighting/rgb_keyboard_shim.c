/**
 * @file rgb_keyboard_shim.c
 * @brief 灯效用模拟按键状态表，供 LINEAR/JELLY/TRIGGER 等读取行程与按下态
 */
#include <string.h>

#include "lighting/rgb_keyboard_shim.h"

/** 稠密 key_id 对应的按键状态（行程 + 报告按下态） */
static Key s_keys[TOTAL_KEY_NUM];

/** 灯效引擎使用的全局毫秒 tick，由 Lighting_Poll 同步 */
uint32_t g_keyboard_tick;

/** @brief 清零全部模拟按键状态与 tick */
void RgbKeyboardShim_Init(void)
{
    memset(s_keys, 0, sizeof(s_keys));
    g_keyboard_tick = 0u;
}

/** @brief 更新灯效引擎全局 tick（毫秒） */
void RgbKeyboardShim_SetTick(uint32_t tick_ms)
{
    g_keyboard_tick = tick_ms;
}

/**
 * @brief 按稠密 key_id 获取按键状态指针
 * @param key_id 布局去重后的键索引
 * @return 有效指针；越界返回 NULL
 */
Key *keyboard_get_key(uint16_t key_id)
{
    if (key_id >= TOTAL_KEY_NUM) {
        return NULL;
    }
    return &s_keys[key_id];
}

/**
 * @brief 读取按键有效模拟量（行程）
 * @param key 按键状态；NULL 时返回最小值
 */
AnalogValue keyboard_get_key_effective_analog_value(const Key *key)
{
    if (key == NULL) {
        return ANALOG_VALUE_MIN;
    }
    return key->effective_analog;
}

/**
 * @brief 写入指定 key_id 的模拟行程与按下状态
 * @param key_id 稠密键索引
 * @param intensity_0_1 行程比例 0.0~1.0
 * @param pressed 是否按下
 */
void RgbKeyboardShim_SetKeyState(uint16_t key_id, float intensity_0_1, uint8_t pressed)
{
    AnalogValue val;

    if (key_id >= TOTAL_KEY_NUM) {
        return;
    }
    if (intensity_0_1 < 0.f) {
        intensity_0_1 = 0.f;
    }
    if (intensity_0_1 > 1.f) {
        intensity_0_1 = 1.f;
    }
    val = (AnalogValue)((uint32_t)(intensity_0_1 * (float)ANALOG_VALUE_RANGE));
    if (val > ANALOG_VALUE_MAX) {
        val = ANALOG_VALUE_MAX;
    }
    s_keys[key_id].effective_analog = val;
    s_keys[key_id].report_state = (pressed != 0u) ? true : false;
}

/** @brief 清除指定 key_id 的模拟状态（行程归零、未按下） */
void RgbKeyboardShim_ClearKeyState(uint16_t key_id)
{
    if (key_id >= TOTAL_KEY_NUM) {
        return;
    }
    s_keys[key_id].effective_analog = ANALOG_VALUE_MIN;
    s_keys[key_id].report_state = false;
}

/** @brief 清除全部模拟按键状态 */
void RgbKeyboardShim_ClearAllKeyStates(void)
{
    memset(s_keys, 0, sizeof(s_keys));
}
