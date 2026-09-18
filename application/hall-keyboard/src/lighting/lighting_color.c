/**
 * @file lighting_color.c
 * @brief RGB color sync with DeviceParams.lighting_color
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "lighting/lighting_color.h"
#include "lighting/lighting_mode.h"
#include "device/device_params.h"
#include "lighting/rgb.h"

static uint8_t s_color_rgb[3];
static uint8_t s_color_idx;

static const ColorRGB s_color_table[] = {
    {255, 0, 0},
    {255, 128, 0},
    {0, 0, 255},
    {0, 255, 0},
    {0, 255, 255},
    {255, 255, 0},
    {128, 0, 255},
    {255, 255, 255},
};
#define COLOR_TABLE_COUNT (sizeof(s_color_table) / sizeof(s_color_table[0]))

static bool lighting_color_sync_params(void)
{
    return DeviceParams_SetKeyboardParam(KEYBOARD_SETTING_LIGHTING_COLOR,
                                   s_color_rgb, sizeof(s_color_rgb), true);
}

void Lighting_Color_Init(void)
{
    uint8_t stored[3];

    if (!DeviceParams_GetKeyboardParam(KEYBOARD_SETTING_LIGHTING_COLOR,
                                         stored, sizeof(stored))) {
        stored[0] = 255;
        stored[1] = 0;
        stored[2] = 0;
    }
    memcpy(s_color_rgb, stored, 3);
    s_color_idx = 0;
    Lighting_Color_Apply();
    printf("[Lighting] Init color: %02X%02X%02X\r\n",
                  (unsigned)s_color_rgb[0],
                  (unsigned)s_color_rgb[1],
                  (unsigned)s_color_rgb[2]);
}

void Lighting_Color_SwitchNext(void)
{
    s_color_idx = (s_color_idx + 1u) % (uint8_t)COLOR_TABLE_COUNT;
    s_color_rgb[0] = s_color_table[s_color_idx].r;
    s_color_rgb[1] = s_color_table[s_color_idx].g;
    s_color_rgb[2] = s_color_table[s_color_idx].b;
    Lighting_Color_Apply();
    lighting_color_sync_params();
    printf("[Lighting] Color SwitchNext: %02X%02X%02X\r\n",
                  (unsigned)s_color_rgb[0],
                  (unsigned)s_color_rgb[1],
                  (unsigned)s_color_rgb[2]);
}

static void lighting_color_sync_base_layer_for_mode(EN_LIGHTING_MODE mode)
{
    switch (mode) {
    case EN_LIGHTING_MODE_RAINBOW:
        rgb_set_per_key_layer_enabled(false);
        rgb_set_base_mode(RGB_BASE_MODE_RAINBOW);
        break;
    case EN_LIGHTING_MODE_WAVE:
        rgb_set_per_key_layer_enabled(false);
        rgb_set_base_mode(RGB_BASE_MODE_WAVE);
        break;
    default:
        rgb_set_per_key_layer_enabled(true);
        rgb_set_base_mode(RGB_BASE_MODE_BLANK);
        break;
    }
}

void Lighting_Color_Apply(void)
{
    EN_LIGHTING_MODE mode = Lighting_Mode_GetCurrent();
    ColorRGB rgb;

    rgb.r = s_color_rgb[0];
    rgb.g = s_color_rgb[1];
    rgb.b = s_color_rgb[2];
    rgb_set_base_color(&rgb);
    lighting_color_sync_base_layer_for_mode(mode);
    if (Lighting_Mode_GetToggle() == 0u) {
        rgb_set_base_mode(RGB_BASE_MODE_OFF);
    }
}

void Lighting_Color_GetRGB(uint8_t out_rgb[3])
{
    if (out_rgb != NULL) {
        out_rgb[0] = s_color_rgb[0];
        out_rgb[1] = s_color_rgb[1];
        out_rgb[2] = s_color_rgb[2];
    }
}

bool Lighting_Color_SetBaseColor(const uint8_t rgb[3])
{
    if (rgb == NULL) {
        return false;
    }
    bool ret = false;
    uint8_t last_rgb[3];
    memcpy(last_rgb, s_color_rgb, sizeof(s_color_rgb));
    memcpy(s_color_rgb, rgb, sizeof(s_color_rgb));
    ret = lighting_color_sync_params();
    if (!ret) {
        memcpy(s_color_rgb, last_rgb, sizeof(s_color_rgb));
        return false;
    }
    Lighting_Color_Apply();
    return ret;
}
