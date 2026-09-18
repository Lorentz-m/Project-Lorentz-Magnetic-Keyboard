/**
 * @file lighting_key.c
 * @brief 灯效物理键输入：coord → key_id → shim 行程；边沿触发波纹/串灯等
 */
#include "lighting/lighting_key.h"
#include "lighting/rgb_layout.h"
#include "lighting/rgb_keyboard_shim.h"
#include "lighting/rgb.h"
#include "systick.h"

/**
 * @brief 按下/抬起边沿处理
 * @param key_id      布局键索引
 * @param tick_ms     当前毫秒 tick
 * @param pressed     本次是否按下
 * @param was_pressed 边沿前 shim 中的按下态
 */
static void Lighting_OnKeyEdge(uint16_t key_id, uint32_t tick_ms, uint8_t pressed, bool was_pressed)
{
    uint16_t slot;        /**< 该键主灯珠 slot（逆映射） */
    bool press_edge;      /**< 本帧按下边沿 */
    bool release_edge;    /**< 本帧抬起边沿 */

    if (key_id >= TOTAL_KEY_NUM) {
        return;
    }
    press_edge = (pressed != 0u) && !was_pressed;
    release_edge = (pressed == 0u) && was_pressed;
    if (!press_edge && !release_edge) {
        return;
    }
    slot = g_rgb_inverse_mapping[key_id];
    if (slot >= RGB_NUM) {
        return;
    }

    switch (g_rgb_configs[slot].mode) {
#if RGB_MODE_USE_TRIGGER
        case RGB_MODE_TRIGGER:
            if (press_edge || release_edge) {
                g_rgb_configs[slot].begin_tick = tick_ms;
                rgb_invalidate();
            }
            break;
#endif
#if RGB_MODE_USE_STRING
        case RGB_MODE_STRING:
#endif
#if RGB_MODE_USE_FADING_STRING
        case RGB_MODE_FADING_STRING:
#endif
#if RGB_MODE_USE_DIAMOND_RIPPLE
        case RGB_MODE_DIAMOND_RIPPLE:
#endif
#if RGB_MODE_USE_FADING_DIAMOND_RIPPLE
        case RGB_MODE_FADING_DIAMOND_RIPPLE:
#endif
#if RGB_MODE_USE_BUBBLE
        case RGB_MODE_BUBBLE:
#endif
#if RGB_MODE_USE_STRING || RGB_MODE_USE_FADING_STRING || RGB_MODE_USE_DIAMOND_RIPPLE \
    || RGB_MODE_USE_FADING_DIAMOND_RIPPLE || RGB_MODE_USE_BUBBLE
            if (press_edge) {
                rgb_activate(key_id, tick_ms);
            }
            break;
#endif
        default:
            break;
    }
}

void Lighting_SetKeyStateAtCoord(st_key_coord coord, uint16_t dist_um, uint8_t pressed)
{
    uint16_t key_id;           /**< 坐标对应的布局键索引 */
    uint32_t tick_ms;          /**< 本帧毫秒 tick */
    const Key *key;            /**< shim 中该键状态（读边沿前按下态） */
    bool was_pressed;          /**< 边沿前是否已按下 */
    float intensity_0_1;       /**< 行程归一化强度 0.0~1.0 */

    key_id = RgbLayout_CoordToKeyId(coord);
    if (key_id >= TOTAL_KEY_NUM) {
        return;
    }
    if (dist_um >= LIGHTING_KEY_DIST_UM_MAX) {
        intensity_0_1 = 1.f;
    } else {
        intensity_0_1 = (float)dist_um / (float)LIGHTING_KEY_DIST_UM_MAX;
    }

    key = keyboard_get_key(key_id);
    was_pressed = (key != NULL) ? key->report_state : false;
    tick_ms = systick_get_ms();
    RgbKeyboardShim_SetTick(tick_ms);
    RgbKeyboardShim_SetKeyState(key_id, intensity_0_1, pressed);
    Lighting_OnKeyEdge(key_id, tick_ms, pressed, was_pressed);
}
