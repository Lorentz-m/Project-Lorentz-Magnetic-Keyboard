/**
 * @file lighting_brightness.c
 * @brief 全局亮度：读写 lighting_brightness，经 rgb_set_base_brightness 作用于出帧
 */
#include <stdio.h>

#include "lighting/lighting_brightness.h"
#include "device/device_params.h"
#include "lighting/rgb.h"

/** 亮度调节步进 */
#define LIGHTING_BRIGHTNESS_STEP  20u
/** 亮度最大值（与 rgb_set 缩放一致） */
#define LIGHTING_BRIGHTNESS_MAX   255u

/** 当前亮度缓存 0~255 */
static uint8_t s_brightness;

/** @brief 将 s_brightness 写入 KEYBOARD_SETTING_LIGHTING_BRIGHTNESS */
static bool lighting_brightness_sync_params(void)
{
    return DeviceParams_SetKeyboardParam(KEYBOARD_SETTING_LIGHTING_BRIGHTNESS,
        &s_brightness, sizeof(s_brightness), true);
}

/** @brief 将 s_brightness 应用到 g_rgb_base_config.brightness */
static void lighting_brightness_apply(void)
{
    rgb_set_base_brightness(s_brightness);
}

void Lighting_Brightness_Init(void)
{
    uint8_t stored = 0; /**< 从参数区读出的亮度 */

    if (!DeviceParams_GetKeyboardParam(KEYBOARD_SETTING_LIGHTING_BRIGHTNESS,
            &stored, sizeof(stored))) {
        stored = LIGHTING_BRIGHTNESS_MAX;
    }
    if (stored > LIGHTING_BRIGHTNESS_MAX) {
        stored = LIGHTING_BRIGHTNESS_MAX;
    }
    s_brightness = stored;
    lighting_brightness_apply();
    printf("[Lighting] Init brightness: %d\r\n", s_brightness);
}

void Lighting_Brightness_Add(void)
{
    uint16_t next; /**< 加步进后的中间值 */

    next = (uint16_t)s_brightness + LIGHTING_BRIGHTNESS_STEP;
    s_brightness = (next > LIGHTING_BRIGHTNESS_MAX) ? LIGHTING_BRIGHTNESS_MAX : (uint8_t)next;
    lighting_brightness_apply();
    lighting_brightness_sync_params();
    printf("[Lighting] Brightness Add: %d\r\n", s_brightness);
}

void Lighting_Brightness_Sub(void)
{
    if (s_brightness < LIGHTING_BRIGHTNESS_STEP) {
        s_brightness = 0;
    } else {
        s_brightness = (uint8_t)(s_brightness - LIGHTING_BRIGHTNESS_STEP);
    }
    lighting_brightness_apply();
    lighting_brightness_sync_params();
    printf("[Lighting] Brightness Sub: %d\r\n", s_brightness);
}

bool Lighting_Brightness_Set(uint8_t brightness)
{
    uint8_t last = s_brightness;
    s_brightness = brightness;
    bool ret = false;
    lighting_brightness_apply();
    ret = lighting_brightness_sync_params();
    if (!ret) {
        printf("Failed to set brightness to %d\r\n", brightness);
        s_brightness = last;
    }
    return ret;
}
