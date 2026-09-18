/*
 * Copyright (c) 2024 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * @file rgb_engine.c
 * @brief RGB 灯效渲染引擎：底座层、per-key 层、波纹链表与出帧
 */
 #include <stdio.h>
#include "lighting/rgb.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"
#include "lighting/rgb_platform.h"

#define RGB_EFFECT_DISPLAY_S  78u
#define RGB_EFFECT_DISPLAY_V  99u

/** 遍历环形队列中有效元素的宏 */
#define rgb_loop_queue_foreach(q, type, item) for (uint16_t __index = (q)->front; __index != (q)->rear; __index = (__index + 1) % (q)->len)\
                                              for (type *item = &((q)->data[__index]); item; item = NULL)

/** 两点曼哈顿距离（微米坐标） */
#define MANHATTAN_DISTANCE(m, n) (abs((m)->x - (n)->x) + abs((m)->y - (n)->y))
#define MANHATTAN_DISTANCE_DIRECT(x1, y1, x2, y2) (abs((x1) - (x2)) + abs((y1) - (y2)))
/** 两点欧氏距离 */
#define EUCLIDEAN_DISTANCE(m, n) sqrtf((float)(((m)->x) - (float)((n)->x))*((float)((m)->x) - (float)((n)->x)) + ((float)((m)->y) - (float)((n)->y))*((float)((m)->y) - (float)((n)->y)))

#ifndef RGB_CUSTOM_INVERSE_MAPPING
/** key_id → 代表灯珠 slot 逆映射表 */
uint16_t g_rgb_inverse_mapping[TOTAL_KEY_NUM];
#else
__WEAK const uint16_t g_rgb_inverse_mapping[TOTAL_KEY_NUM];
#endif

// /** HID 接管灯效时为 true，rgb_process 跳过渲染 */
// volatile bool g_rgb_hid_mode;
/** 底座灯效全局配置 */
static RGBBaseConfig g_rgb_base_config;
/** 各灯珠 slot 的 per-key 配置 */
RGBConfig g_rgb_configs[RGB_NUM];
/** 本帧各 slot 合成后的 RGB 缓冲 */
ColorRGB g_rgb_colors[RGB_NUM];
/** false 时 rgb_process 仅渲染底座层，不跑 per-key/波纹 */
static bool s_rgb_per_key_layer_enabled = true;
/** true 表示静态帧已出，无需再 poll（直至 invalidate） */
static bool s_rgb_static_frame_valid;

/** 波纹/串灯等待渲染的链表头 */
static RGBArgumentList rgb_argument_list;
/** 波纹链表节点池 */
static RGBArgumentListNode RGB_Argument_List_Buffer[RGB_ARGUMENT_LIST_BUFFER_LENGTH];

static bool rgb_has_active_ripple(void)
{
    if (rgb_argument_list.head < 0) {
        return false;
    }
    return rgb_argument_list.data[rgb_argument_list.head].next >= 0;
}

static bool rgb_slot_needs_continuous_animation(RGBMode mode)
{
    switch (mode) {
    case RGB_MODE_FIXED:
    case RGB_MODE_STATIC:
        return false;
#if RGB_MODE_USE_CYCLE
    case RGB_MODE_CYCLE:
        return true;
#endif
#if RGB_MODE_USE_LINEAR
    case RGB_MODE_LINEAR:
        return true;
#endif
#if RGB_MODE_USE_TRIGGER
    case RGB_MODE_TRIGGER:
        return true;
#endif
#if RGB_MODE_USE_JELLY
    case RGB_MODE_JELLY:
        return true;
#endif
    default:
        return false;
    }
}

static bool rgb_has_continuous_animation(void)
{
    RGBBaseMode base;

    base = g_rgb_base_config.mode;
#if RGB_BASE_MODE_USE_RAINBOW
    if (base == RGB_BASE_MODE_RAINBOW) {
        return true;
    }
#endif
#if RGB_BASE_MODE_USE_WAVE
    if (base == RGB_BASE_MODE_WAVE) {
        return true;
    }
#endif
    if (rgb_has_active_ripple()) {
        return true;
    }
    if (!s_rgb_per_key_layer_enabled) {
        return false;
    }
    for (uint16_t i = 0u; i < RGB_NUM; i++) {
        if (rgb_slot_needs_continuous_animation(g_rgb_configs[i].mode)) {
            return true;
        }
    }
    return false;
}

void rgb_invalidate(void)
{
    s_rgb_static_frame_valid = false;
}

bool rgb_needs_process(void)
{
    if (g_rgb_base_config.mode == RGB_BASE_MODE_OFF) {
        return false;
    }
    if (rgb_has_continuous_animation()) {
        return true;
    }
    return !s_rgb_static_frame_valid;
}

/** @brief 初始化波纹链表与 slot→key 逆映射（FinalizeInverseMapping 会覆盖 primary） */
void rgb_init(void)
{
    rgb_forward_list_init(&rgb_argument_list, RGB_Argument_List_Buffer, RGB_ARGUMENT_LIST_BUFFER_LENGTH);
#ifndef RGB_CUSTOM_INVERSE_MAPPING
    for (int i = 0; i < RGB_NUM; i++)
    {
        if (g_rgb_mapping[i] < TOTAL_KEY_NUM)
        {
            g_rgb_inverse_mapping[g_rgb_mapping[i]] = i;
        }
    }
#endif
}

void rgb_set_base_mode(RGBBaseMode mode)
{
    if (mode > RGB_BASE_MODE_WAVE) {
        printf("rgb_set_base_mode: invalid mode %d\r\n", mode);
        return;
    }
    g_rgb_base_config.mode = mode;
    rgb_invalidate();
}

void rgb_set_base_color(const ColorRGB *rgb)
{
    if (rgb == NULL) {
        printf("rgb_set_base_color: invalid rgb\r\n");
        return;
    }
    g_rgb_base_config.rgb = *rgb;
    rgb_to_hsv(&g_rgb_base_config.hsv, rgb);
    rgb_invalidate();
}

void rgb_set_base_brightness(uint8_t brightness)
{
    g_rgb_base_config.brightness = brightness;
    rgb_invalidate();
}

void rgb_set_per_key_layer_enabled(bool enabled)
{
    s_rgb_per_key_layer_enabled = enabled;
    rgb_invalidate();
}

bool rgb_is_per_key_layer_enabled(void)
{
    return s_rgb_per_key_layer_enabled;
}

/** 按行程比例在 [low, up] 间线性插值到 uint8 */
#define COLOR_INTERVAL(key, low, up) (uint8_t)((key) < 0 ? (low) : ((key) > ANALOG_VALUE_MAX ? (up) : (key) * (up)))
/** tick 差 × speed 得到效果时间跨度 */
#define CALC_SPAN(tick, speed) ((KEYBOARD_TICK_TO_TIME(tick)) * (speed))

/** @brief 灯效主流程 */
void rgb_process(void)
{
    if (!g_rgb_base_config.mode)
    {
        rgb_turn_off();
        return;
    }

    // if (g_rgb_hid_mode)
    // {
    //     return;
    // }
    ColorHSV temp_hsv;
    ColorRGB temp_rgb;
    float intensity;
    UNUSED(temp_hsv);
    memset(g_rgb_colors, 0, sizeof(g_rgb_colors));

    switch (g_rgb_base_config.mode)
    {
#if RGB_BASE_MODE_USE_RAINBOW
    case RGB_BASE_MODE_RAINBOW:
        {
            float direction_c = g_rgb_base_config.direction * M_PI / 180;
            float direction_sin = sinf(direction_c);
            float direction_cos = cosf(direction_c);

            int64_t total_offset = (int64_t)KEYBOARD_TICK_TO_TIME(g_keyboard_tick) * g_rgb_base_config.speed;
            int32_t wrapped_offset = total_offset % 360000;
            if (wrapped_offset < 0) wrapped_offset += 360000;
            float safe_time_offset = wrapped_offset / 1000.0f;
            temp_hsv.s = RGB_EFFECT_DISPLAY_S;
            temp_hsv.v = RGB_EFFECT_DISPLAY_V;
            for (uint8_t i = 0; i < RGB_NUM; i++)
            {
                const RGBLocation* location = &g_rgb_locations[i];
                float vertical_distance = (location->x * direction_cos + location->y * direction_sin)/(float)KEY_SWITCH_DISTANCE;
                temp_hsv.h = (uint16_t)(((uint32_t)(vertical_distance * g_rgb_base_config.density + safe_time_offset)) % 360u);
                color_set_hsv(&temp_rgb, &temp_hsv);
                color_mix(&g_rgb_colors[i], &temp_rgb);
            }
        }
        break;
#endif
#if RGB_BASE_MODE_USE_WAVE
    case RGB_BASE_MODE_WAVE:
        {
            float direction_c = g_rgb_base_config.direction * M_PI / 180;
            float direction_sin = sinf(direction_c);
            float direction_cos = cosf(direction_c);

            int64_t total_offset = (int64_t)KEYBOARD_TICK_TO_TIME(g_keyboard_tick) * g_rgb_base_config.speed;
            int32_t wrapped_offset = total_offset % 360000;
            if (wrapped_offset < 0) wrapped_offset += 360000;
            float safe_time_offset = wrapped_offset / 1000.0f;

            const ColorRGB wave_pri_rgb = {10, 20, 235};
            const ColorRGB wave_sec_rgb = {55, 252, 55};

            for (uint8_t i = 0; i < RGB_NUM; i++)
            {
                const RGBLocation* location = &g_rgb_locations[i];
                float vertical_distance = (location->x * direction_cos + location->y * direction_sin)/(float)KEY_SWITCH_DISTANCE;
                float intensity = fmodf(((vertical_distance * g_rgb_base_config.density + safe_time_offset) / 180.0f), 2.0f);
                intensity -= 1.0f;
                float secondary_intensity;
                if (intensity<0) { intensity = -intensity; }
                secondary_intensity = 1 - intensity;

                temp_rgb.r = (uint8_t)(intensity * ((float)(wave_pri_rgb.r)) + secondary_intensity * ((float)(wave_sec_rgb.r)));
                temp_rgb.g = (uint8_t)(intensity * ((float)(wave_pri_rgb.g)) + secondary_intensity * ((float)(wave_sec_rgb.g)));
                temp_rgb.b = (uint8_t)(intensity * ((float)(wave_pri_rgb.b)) + secondary_intensity * ((float)(wave_sec_rgb.b)));
                color_mix(&g_rgb_colors[i], &temp_rgb);
            }
        }
        break;
#endif
    default:
        break;
    }

    if (s_rgb_per_key_layer_enabled) {
        /* 波纹/串灯：遍历活跃 RGBArgument 链表 */
        for (int16_t* iterator_ptr = &rgb_argument_list.data[rgb_argument_list.head].next; *iterator_ptr >= 0;)
        {
            RGBArgumentListNode* node = &(rgb_argument_list.data[*iterator_ptr]);
            RGBArgument * item = &(node->data);
            RGBConfig *config = &g_rgb_configs[item->rgb_ptr];
            RGBLocation *location = (RGBLocation *)&g_rgb_locations[item->rgb_ptr];
            float distance = CALC_SPAN(g_keyboard_tick - item->begin_tick, config->speed);
            if (MANHATTAN_DISTANCE_DIRECT(location->x, RGB_LEFT_UM, location->y, RGB_TOP_UM) < distance - FADING_DISTANCE_UM &&
                MANHATTAN_DISTANCE_DIRECT(location->x, RGB_LEFT_UM, location->y, RGB_BOTTOM_UM) < distance - FADING_DISTANCE_UM &&
                MANHATTAN_DISTANCE_DIRECT(location->x, RGB_RIGHT_UM, location->y, RGB_TOP_UM) < distance - FADING_DISTANCE_UM &&
                MANHATTAN_DISTANCE_DIRECT(location->x, RGB_RIGHT_UM, location->y, RGB_BOTTOM_UM) < distance - FADING_DISTANCE_UM)
            {
                int16_t free_node = *iterator_ptr;
                *iterator_ptr = node->next;
                node->next = (&rgb_argument_list)->free_node;
                (&rgb_argument_list)->free_node = free_node;
                continue;
            }
            switch (config->mode)
            {
            case RGB_MODE_FIXED:
                break;
            case RGB_MODE_STATIC:
                break;
            case RGB_MODE_CYCLE:
                break;
            case RGB_MODE_LINEAR:
                break;
            case RGB_MODE_TRIGGER:
                break;
            case RGB_MODE_STRING:
            case RGB_MODE_FADING_STRING:
            case RGB_MODE_DIAMOND_RIPPLE:
            case RGB_MODE_FADING_DIAMOND_RIPPLE:
            case RGB_MODE_BUBBLE:
                for (int8_t j = 0; j < RGB_NUM; j++)
                {
                    switch (config->mode)
                    {
    #if RGB_MODE_USE_STRING
                    case RGB_MODE_STRING:
                        intensity = (UNIT_TO_UM(1.0) - fabsf(distance - abs(location->x - g_rgb_locations[j].x)));
                        intensity = intensity > 0 ? intensity : 0;
                        intensity = abs(location->y - g_rgb_locations[j].y) < UNIT_TO_UM(0.5) ? intensity : 0;
                        intensity /= UNIT_TO_UM(1.0);
                        break;
    #endif
    #if RGB_MODE_USE_FADING_STRING
                    case RGB_MODE_FADING_STRING:
                        intensity = (distance - abs(location->x - g_rgb_locations[j].x));
                        if (intensity > 0)
                        {
                            intensity = FADING_DISTANCE_UM - intensity > 0 ? FADING_DISTANCE_UM - intensity : 0;
                            intensity /= FADING_DISTANCE_UM;
                        }
                        else
                        {
                            intensity = UNIT_TO_UM(1.0) + intensity > 0 ? UNIT_TO_UM(1.0) + intensity : 0;
                            intensity /= UNIT_TO_UM(1.0);
                        }
                        intensity = abs(location->y - g_rgb_locations[j].y) < UNIT_TO_UM(0.5) ? intensity : 0;
                        break;
    #endif
    #if RGB_MODE_USE_DIAMOND_RIPPLE
                    case RGB_MODE_DIAMOND_RIPPLE:
                        intensity = (UNIT_TO_UM(1.0) - fabsf(distance - MANHATTAN_DISTANCE(location, &g_rgb_locations[j])));
                        intensity = intensity > 0 ? intensity : 0;
                        intensity /= UNIT_TO_UM(1.0);
                        break;
    #endif
    #if RGB_MODE_USE_FADING_DIAMOND_RIPPLE
                    case RGB_MODE_FADING_DIAMOND_RIPPLE:
                        intensity = (distance - MANHATTAN_DISTANCE(location, &g_rgb_locations[j]));
                        if (intensity > 0)
                        {
                            intensity = FADING_DISTANCE_UM - intensity > 0 ? FADING_DISTANCE_UM - intensity : 0;
                            intensity /= FADING_DISTANCE_UM;
                            break;
                        }
                        else
                        {
                            intensity = UNIT_TO_UM(1.0) + intensity > 0 ? UNIT_TO_UM(1.0) + intensity : 0;
                            intensity /= UNIT_TO_UM(1.0);
                        }
                        break;
    #endif
    #if RGB_MODE_USE_BUBBLE
                    case RGB_MODE_BUBBLE:
                        {
                            float e_distance = EUCLIDEAN_DISTANCE(location, &g_rgb_locations[j]);
                            if (e_distance > BUBBLE_DISTANCE_UM)
                            {
                                intensity = 0;
                                continue;
                            }
                            intensity = (distance - e_distance);
                            if (intensity > 0)
                            {
                                intensity = FADING_DISTANCE_UM - intensity > 0 ? FADING_DISTANCE_UM - intensity : 0;
                                intensity /= FADING_DISTANCE_UM;
                                break;
                            }
                            else
                            {
                                intensity = UNIT_TO_UM(1.0) + intensity > 0 ? UNIT_TO_UM(1.0) + intensity : 0;
                                intensity /= UNIT_TO_UM(1.0);
                            }
                        }
                        break;
    #endif
                    default:
                        intensity = 0;
                        break;
                    }
                    temp_rgb.r = ((uint8_t)(intensity * ((float)(g_rgb_base_config.rgb.r)))) >> 1;
                    temp_rgb.g = ((uint8_t)(intensity * ((float)(g_rgb_base_config.rgb.g)))) >> 1;
                    temp_rgb.b = ((uint8_t)(intensity * ((float)(g_rgb_base_config.rgb.b)))) >> 1;
                    color_mix(&g_rgb_colors[j], &temp_rgb);
                }
                break;
            case RGB_MODE_JELLY:
                break;
            default:
                break;
            }
            iterator_ptr = &(&rgb_argument_list)->data[*iterator_ptr].next;
        }

        /* per-key 层：按 slot 读取按键状态并叠加模式 */
        for (uint16_t i = 0; i < RGB_NUM; i++)
        {
            bool report_state = false;
            Color* target_color = &g_rgb_colors[i];
            RGBConfig* rgb_config = &g_rgb_configs[i];
            Key* key = keyboard_get_key(g_rgb_mapping[i]);
            if (key != NULL)
            {
                intensity = keyboard_get_key_effective_analog_value(key)/((float)ANALOG_VALUE_RANGE);
                report_state = key->report_state;
            }
            else
            {
                intensity = 0.0f;
                report_state = false;
            }
            UNUSED(report_state);
            switch (rgb_config->mode)
            {
    #if RGB_MODE_USE_LINEAR
            case RGB_MODE_LINEAR:
                temp_rgb.r = COLOR_INTERVAL(intensity, 0, (float)(g_rgb_base_config.rgb.r));
                temp_rgb.g = COLOR_INTERVAL(intensity, 0, (float)(g_rgb_base_config.rgb.g));
                temp_rgb.b = COLOR_INTERVAL(intensity, 0, (float)(g_rgb_base_config.rgb.b));
                color_mix(target_color, &temp_rgb);
                break;
    #endif
    #if RGB_MODE_USE_TRIGGER
            case RGB_MODE_TRIGGER:
                if (report_state)
                {
                    rgb_config->begin_tick = g_keyboard_tick;
                }
                intensity = powf(0.9999, CALC_SPAN(g_keyboard_tick - rgb_config->begin_tick, rgb_config->speed));
                temp_rgb.r = (uint8_t)((float)(g_rgb_base_config.rgb.r) * intensity);
                temp_rgb.g = (uint8_t)((float)(g_rgb_base_config.rgb.g) * intensity);
                temp_rgb.b = (uint8_t)((float)(g_rgb_base_config.rgb.b) * intensity);
                color_mix(target_color, &temp_rgb);
                break;
    #endif
            case RGB_MODE_FIXED:
                color_set_rgb(target_color, &g_rgb_base_config.rgb);
                break;
    #if RGB_MODE_USE_STATIC
            case RGB_MODE_STATIC:
                if (g_rgb_base_config.mode == RGB_BASE_MODE_BLANK) {
                    color_set_rgb(target_color, &g_rgb_base_config.rgb);
                }
                color_mix(target_color, &rgb_config->rgb);
                break;
    #endif
    #if RGB_MODE_USE_CYCLE
            case RGB_MODE_CYCLE:
                {
                    uint32_t span = (uint32_t)CALC_SPAN(g_keyboard_tick, rgb_config->speed);
                    uint32_t hue_delta = (span % RGB_CYCLE_HUE_PERIOD_MS) * 360u / RGB_CYCLE_HUE_PERIOD_MS;

                    temp_hsv.s = RGB_EFFECT_DISPLAY_S;
                    temp_hsv.v = RGB_EFFECT_DISPLAY_V;
                    if (g_rgb_base_config.hsv.s > 0u) {
                        temp_hsv.h = (uint16_t)(((uint32_t)g_rgb_base_config.hsv.h + hue_delta) % 360u);
                    } else {
                        temp_hsv.h = (uint16_t)hue_delta;
                    }
                }
                color_set_hsv(&temp_rgb, &temp_hsv);
                color_mix(target_color, &temp_rgb);
                break;
    #endif
    #if RGB_MODE_USE_JELLY
            case RGB_MODE_JELLY:
                for (int8_t j = 0; j < RGB_NUM; j++)
                {
                    float intensity_jelly = (JELLY_DISTANCE_UM * intensity) - MANHATTAN_DISTANCE(&g_rgb_locations[j], &g_rgb_locations[i]);
                    intensity_jelly = intensity_jelly > 0 ? intensity_jelly > UNIT_TO_UM(1) ? UNIT_TO_UM(1) : intensity_jelly : 0;
                    intensity_jelly /= UNIT_TO_UM(1);
                    temp_rgb.r = ((uint8_t)(intensity_jelly * ((float)(g_rgb_base_config.rgb.r)))) >> 1;
                    temp_rgb.g = ((uint8_t)(intensity_jelly * ((float)(g_rgb_base_config.rgb.g)))) >> 1;
                    temp_rgb.b = ((uint8_t)(intensity_jelly * ((float)(g_rgb_base_config.rgb.b)))) >> 1;
                    color_mix(&g_rgb_colors[j], &temp_rgb);
                }
                break;
    #endif
            default:
                break;
            }
        }
    }

    rgb_flush();
    if (!rgb_has_continuous_animation()) {
        s_rgb_static_frame_valid = true;
    }
}

/** @brief 出帧前弱符号回调（板级可覆写） */
__WEAK void rgb_update_callback(void)
{

}

/**
 * @brief 写单颗灯珠 RGB（含亮度缩放）并转调 led_display
 * @param index 灯珠 slot
 */
void rgb_set(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
#ifdef RGB_GAMMA_ENABLE
    r = GAMMA_CORRECT(r, 255)*(g_rgb_base_config.brightness / 255.0f) + 0.5;
    g = GAMMA_CORRECT(g, 255)*(g_rgb_base_config.brightness / 255.0f) + 0.5;
    b = GAMMA_CORRECT(b, 255)*(g_rgb_base_config.brightness / 255.0f) + 0.5;
#else
    r = (r * g_rgb_base_config.brightness) >> 8;
    g = (g * g_rgb_base_config.brightness) >> 8;
    b = (b * g_rgb_base_config.brightness) >> 8;
#endif
    led_set(index, r, g, b);
}

/** @brief 上电/插拔时的端口位置波纹闪灯动画（阻塞直到结束） */
void rgb_init_flash(void)
{
    float intensity;
    ColorRGB temp_rgb;
    uint32_t begin_tick = g_keyboard_tick;
    RGBLocation location = PORT_LOCATION;
    bool animation_playing = false;
    while (KEYBOARD_TICK_TO_TIME(g_keyboard_tick - begin_tick) < RGB_FLASH_MAX_DURATION)
    {
        float distance = KEYBOARD_TICK_TO_TIME(g_keyboard_tick - begin_tick) * RGB_FLASH_RIPPLE_SPEED;
        memset(g_rgb_colors, 0, sizeof(g_rgb_colors));
        animation_playing = false;
        for (int8_t i = 0; i < RGB_NUM; i++)
        {
            intensity = (distance - EUCLIDEAN_DISTANCE(&location, &g_rgb_locations[i]));

            if (intensity > 0)
            {
                intensity = UNIT_TO_UM(10) - intensity > 0 ? UNIT_TO_UM(10) - intensity : 0;
                intensity /= UNIT_TO_UM(10);
            }
            else
            {
                intensity = UNIT_TO_UM(1.0f) + intensity > 0 ? UNIT_TO_UM(1.0f) + intensity : 0;
                intensity /= UNIT_TO_UM(1.0f);
            }
            if (intensity > 0 || distance < UNIT_TO_UM(10))
            {
                animation_playing = true;
            }

            temp_rgb.r = intensity * 255;
            temp_rgb.g = intensity * 255;
            temp_rgb.b = intensity * 255;
            color_mix(&g_rgb_colors[i], &temp_rgb);
        }
        if (!animation_playing)
        {
            break;
        }
        for (uint8_t i = 0; i < RGB_NUM; i++)
        {
            rgb_set(i, g_rgb_colors[i].r, g_rgb_colors[i].g, g_rgb_colors[i].b);
        }
        led_flush();
    }
    rgb_turn_off();
}

/** @brief 全键同步呼吸闪（阻塞直到结束） */
void rgb_flash(void)
{
    float intensity;
    ColorRGB temp_rgb;
    uint32_t begin_time = g_keyboard_tick;
    while (g_keyboard_tick - begin_time < RGB_FLASH_MAX_DURATION)
    {
        float distance = (g_keyboard_tick - begin_time);
        memset(g_rgb_colors, 0, sizeof(g_rgb_colors));
        intensity = (RGB_FLASH_MAX_DURATION/2 - fabsf(distance - (RGB_FLASH_MAX_DURATION/2)))/((float)(RGB_FLASH_MAX_DURATION/2));
        for (int8_t i = 0; i < RGB_NUM; i++)
        {
            temp_rgb.r = (intensity * 255);
            temp_rgb.g = (intensity * 255);
            temp_rgb.b = (intensity * 255);
            color_mix(&g_rgb_colors[i], &temp_rgb);
        }
        for (uint8_t i = 0; i < RGB_NUM; i++)
        {
            rgb_set(i, g_rgb_colors[i].r, g_rgb_colors[i].g, g_rgb_colors[i].b);
        }
        led_flush();
    }
    rgb_turn_off();
}

/** @brief 熄灭全部灯珠并 commit */
void rgb_turn_off(void)
{
    for (uint8_t i = 0; i < RGB_NUM; i++)
    {
        rgb_set(i, 0, 0, 0);
    }
    led_flush();
}

/** @brief 恢复底座/per-key 默认配置（不重建 layout 映射） */
void rgb_factory_reset(void)
{
    g_rgb_base_config.mode = RGB_BASE_MODE_BLANK;
    g_rgb_base_config.brightness = 255;
    g_rgb_base_config.density = 27;
    g_rgb_base_config.direction = 0;
    g_rgb_base_config.speed = RGB_DEFAULT_SPEED;
    ColorHSV temphsv = RGB_DEFAULT_COLOR_HSV;
    g_rgb_base_config.hsv = temphsv;
    color_set_hsv(&g_rgb_base_config.rgb, &temphsv);
    memset(&g_rgb_base_config.secondary_rgb,0,sizeof(g_rgb_base_config.secondary_rgb));
    memset(&g_rgb_base_config.secondary_hsv,0,sizeof(g_rgb_base_config.secondary_hsv));
    for (uint8_t i = 0; i < RGB_NUM; i++)
    {
        g_rgb_configs[i].mode = RGB_DEFAULT_MODE;
        g_rgb_configs[i].hsv = temphsv;
        g_rgb_configs[i].speed = RGB_DEFAULT_SPEED;
        color_set_hsv(&g_rgb_configs[i].rgb, &temphsv);
    }
}

/** @brief 将 g_rgb_colors 写入 led_display 并 commit */
void rgb_flush(void)
{
    rgb_update_callback();
    for (uint8_t i = 0; i < RGB_NUM; i++)
    {
        rgb_set(i, g_rgb_colors[i].r, g_rgb_colors[i].g, g_rgb_colors[i].b);
    }
    led_flush();
}

/** @brief 清空波纹/串灯活跃链表（模式切换时丢弃在途动画） */
void rgb_clear_ripple_list(void)
{
    rgb_forward_list_init(&rgb_argument_list, RGB_Argument_List_Buffer,
                          RGB_ARGUMENT_LIST_BUFFER_LENGTH);
    rgb_invalidate();
}

/**
 * @brief 在指定 key_id 触发波纹类效果（插入 RGBArgument 链表）
 * @param id 稠密 key_id
 * @param tick 触发时刻 tick
 */
void rgb_activate(uint16_t id, uint32_t tick)
{
    if (id >= RGB_NUM || g_rgb_inverse_mapping[id] == 0xFFFF)
    {
        return;
    }
    RGBArgument a;
    a.rgb_ptr = g_rgb_inverse_mapping[id];
    a.begin_tick = tick;
    g_rgb_configs[a.rgb_ptr].begin_tick = tick;
    switch (g_rgb_configs[a.rgb_ptr].mode)
    {
    case RGB_MODE_STRING:
    case RGB_MODE_FADING_STRING:
    case RGB_MODE_DIAMOND_RIPPLE:
    case RGB_MODE_FADING_DIAMOND_RIPPLE:
    case RGB_MODE_BUBBLE:
        rgb_forward_list_insert_after(&rgb_argument_list,&rgb_argument_list.data[rgb_argument_list.head], a);
        rgb_invalidate();
        break;
    default:
        break;
    }
}

/** @brief 初始化单向链表空闲池与哨兵头结点 */
void rgb_forward_list_init(RGBArgumentList* list, RGBArgumentListNode* data, uint16_t len)
{
    list->data = data;
    list->head = -1;
    list->tail = 0;
    list->len = len;
    for (int i = 0; i < len; i++)
    {
        list->data[i].next = i + 1;
    }
    list->data[len - 1].next = -1;
    list->free_node = 0;
    rgb_forward_list_push_front(list, (RGBArgument){0,0});
}

/** @brief 删除 node 的后继节点并回收到空闲链 */
void rgb_forward_list_erase_after(RGBArgumentList* list, RGBArgumentListNode* data)
{
    int16_t target = 0;
    target = data->next;
    data->next = list->data[target].next;
    list->data[target].next = list->free_node;
    list->free_node = target;
}

/** @brief 在 data 之后插入新波纹参数节点 */
void rgb_forward_list_insert_after(RGBArgumentList* list, RGBArgumentListNode* data, RGBArgument t)
{
    if (list->free_node == -1)
    {
        return;
    }
    int16_t new_node = list->free_node;
    list->free_node = list->data[list->free_node].next;

    list->data[new_node].data = t;
    list->data[new_node].next = data->next;

    data->next = new_node;
}

/** @brief 在链表头前插入节点（用于哨兵） */
void rgb_forward_list_push_front(RGBArgumentList* list, RGBArgument t)
{
    if (list->free_node == -1)
    {
        return;
    }
    int16_t new_node = list->free_node;
    list->free_node = list->data[list->free_node].next;

    list->data[new_node].data = t;
    list->data[new_node].next = list->head;

    list->head = new_node;
}
