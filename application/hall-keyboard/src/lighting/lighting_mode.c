/**
 * @file lighting_mode.c
 * @brief 灯效模式状态机：切换、开关、DeviceParams 同步与 g_rgb_configs 下发
 */
#include <stdio.h>

#include "lighting/lighting_mode.h"
#include "lighting/lighting_color.h"
#include "lighting/lighting_speed.h"
#include "device/device_params.h"
#include "lighting/rgb_keyboard_shim.h"
#include "lighting/rgb.h"

/** 当前用户模式索引（EN_LIGHTING_MODE） */
static EN_LIGHTING_MODE s_mode;
/** 灯效总开关：0 关屏（RGB_BASE_MODE_OFF），非 0 时应用 s_mode */
static uint8_t s_toggle;

/**
 * @brief 当 params->speed 为 0 时，按 RGBMode 填默认 int16 速度
 * @param mode  目标 RGBMode
 * @param speed 输出速度指针
 */
static void Lighting_SetModeSpeedDefault(RGBMode mode, int16_t *speed)
{
    switch (mode) {
    case RGB_MODE_CYCLE:
        *speed = Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE_CYCLE);
        break;
    case RGB_MODE_TRIGGER:
        *speed = Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE_TRIGGER);
        break;
    case RGB_MODE_STRING:
        *speed = Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE_STRING);
        break;
    case RGB_MODE_FADING_STRING:
        *speed = Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE_FADING_STRING);
        break;
    case RGB_MODE_DIAMOND_RIPPLE:
        *speed = Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE_DIAMOND_RIPPLE);
        break;
    case RGB_MODE_FADING_DIAMOND_RIPPLE:
        *speed = Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE);
        break;
    case RGB_MODE_BUBBLE:
        *speed = Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE_BUBBLE);
        break;
    default:
        *speed = 0;
        break;
    }
}

/**
 * @brief 将全部 slot 设为同一 RGBMode 与 speed
 * @param mode             RGBMode
 * @param speed            为 0 则用模式默认
 * @param reset_begin_tick 是否清零 begin_tick
 */
static void Lighting_ApplyModeSpeedToAllSlots(RGBMode mode, int16_t speed, bool reset_begin_tick)
{
    uint16_t i;

    if (speed == 0) {
        Lighting_SetModeSpeedDefault(mode, &speed);
    }
    for (i = 0u; i < RGB_NUM; i++) {
        g_rgb_configs[i].mode = mode;
        g_rgb_configs[i].speed = speed;
        if (reset_begin_tick) {
            g_rgb_configs[i].begin_tick = 0u;
        }
    }
}

#if RGB_MODE_USE_LINEAR
/** @brief 仅改 RGBMode 与 begin_tick，不改 speed（渐消类除外） */
static void Lighting_ApplyModeOnlyToAllSlots(RGBMode mode)
{
    uint16_t i;

    for (i = 0u; i < RGB_NUM; i++) {
        g_rgb_configs[i].mode = mode;
        g_rgb_configs[i].begin_tick = 0u;
    }
}
#endif

void Lighting_SetGlobalPerKeyMode(RGBMode mode, const un_light_per_key_mode_params *params)
{
    ColorHSV hsv;  /**< STATIC 叠加色 HSV 缓存 */
    uint16_t i;

    if (params == NULL) {
        return;
    }

    rgb_clear_ripple_list();
    RgbKeyboardShim_ClearAllKeyStates();

    switch (mode) {
    case RGB_MODE_FIXED:
        for (i = 0u; i < RGB_NUM; i++) {
            g_rgb_configs[i].mode = RGB_MODE_FIXED;
        }
        break;
#if RGB_MODE_USE_STATIC
    case RGB_MODE_STATIC:
        rgb_to_hsv(&hsv, &params->static_overlay);
        for (i = 0u; i < RGB_NUM; i++) {
            g_rgb_configs[i].mode = RGB_MODE_STATIC;
            g_rgb_configs[i].rgb = params->static_overlay;
            g_rgb_configs[i].hsv = hsv;
        }
        break;
#endif
#if RGB_MODE_USE_CYCLE
    case RGB_MODE_CYCLE:
        Lighting_ApplyModeSpeedToAllSlots(RGB_MODE_CYCLE, params->speed, true);
        break;
#endif
#if RGB_MODE_USE_LINEAR
    case RGB_MODE_LINEAR:
        Lighting_ApplyModeOnlyToAllSlots(RGB_MODE_LINEAR);
        break;
#endif
#if RGB_MODE_USE_TRIGGER
    case RGB_MODE_TRIGGER:
        Lighting_ApplyModeSpeedToAllSlots(RGB_MODE_TRIGGER, params->speed, true);
        break;
#endif
#if RGB_MODE_USE_JELLY
    case RGB_MODE_JELLY:
        Lighting_ApplyModeOnlyToAllSlots(RGB_MODE_JELLY);
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
        Lighting_ApplyModeSpeedToAllSlots(mode, params->speed, true);
        break;
#endif
    default:
        break;
    }
}

/** @brief 应用固定色 per-key 模式 */
static void Lighting_FixedModeFunc(void)
{
    un_light_per_key_mode_params params; /**< 切换 per-key 模式时的参数（栈上） */

    s_mode = EN_LIGHTING_MODE_FIXED;
    rgb_set_per_key_layer_enabled(true);
    rgb_set_base_mode(RGB_BASE_MODE_BLANK);
    Lighting_SetGlobalPerKeyMode(RGB_MODE_FIXED, &params);
}

/** @brief 应用循环变色 per-key 模式 */
static void Lighting_CycleModeFunc(void)
{
    un_light_per_key_mode_params params; /**< 切换 per-key 模式时的参数（栈上） */

    s_mode = EN_LIGHTING_MODE_CYCLE;
    params.speed = Lighting_Speed_GetForMode(EN_LIGHTING_MODE_CYCLE);
    rgb_set_per_key_layer_enabled(true);
    rgb_set_base_mode(RGB_BASE_MODE_BLANK);
    Lighting_SetGlobalPerKeyMode(RGB_MODE_CYCLE, &params);
    Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE_CYCLE);
}

/** @brief 应用底座彩虹模式（关闭 per-key 层） */
static void Lighting_RainbowModeFunc(void)
{
    s_mode = EN_LIGHTING_MODE_RAINBOW;
    rgb_clear_ripple_list();
    RgbKeyboardShim_ClearAllKeyStates();
    rgb_set_per_key_layer_enabled(false);
    rgb_set_base_mode(RGB_BASE_MODE_RAINBOW);
}

/** @brief 应用底座波浪模式（关闭 per-key 层） */
static void Lighting_WaveModeFunc(void)
{
    s_mode = EN_LIGHTING_MODE_WAVE;
    rgb_clear_ripple_list();
    RgbKeyboardShim_ClearAllKeyStates();
    rgb_set_per_key_layer_enabled(false);
    rgb_set_base_mode(RGB_BASE_MODE_WAVE);
}

/** @brief 应用触发灯 per-key 模式 */
static void Lighting_TriggerModeFunc(void)
{
    un_light_per_key_mode_params params; /**< 切换 per-key 模式时的参数（栈上） */

    s_mode = EN_LIGHTING_MODE_TRIGGER;
    params.speed = Lighting_Speed_GetForMode(EN_LIGHTING_MODE_TRIGGER);
    rgb_set_per_key_layer_enabled(true);
    rgb_set_base_mode(RGB_BASE_MODE_BLANK);
    Lighting_SetGlobalPerKeyMode(RGB_MODE_TRIGGER, &params);
    Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE_TRIGGER);
}

/** @brief 应用串灯 per-key 模式 */
static void Lighting_StringModeFunc(void)
{
    un_light_per_key_mode_params params; /**< 切换 per-key 模式时的参数（栈上） */

    s_mode = EN_LIGHTING_MODE_STRING;
    params.speed = Lighting_Speed_GetForMode(EN_LIGHTING_MODE_STRING);
    rgb_set_per_key_layer_enabled(true);
    rgb_set_base_mode(RGB_BASE_MODE_BLANK);
    Lighting_SetGlobalPerKeyMode(RGB_MODE_STRING, &params);
    Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE_STRING);
}

/** @brief 应用渐灭串灯 per-key 模式 */
static void Lighting_FadingStringModeFunc(void)
{
    un_light_per_key_mode_params params; /**< 切换 per-key 模式时的参数（栈上） */

    s_mode = EN_LIGHTING_MODE_FADING_STRING;
    params.speed = Lighting_Speed_GetForMode(EN_LIGHTING_MODE_FADING_STRING);
    rgb_set_per_key_layer_enabled(true);
    rgb_set_base_mode(RGB_BASE_MODE_BLANK);
    Lighting_SetGlobalPerKeyMode(RGB_MODE_FADING_STRING, &params);
    Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE_FADING_STRING);
}

/** @brief 应用菱形波纹 per-key 模式 */
static void Lighting_DiamondRippleModeFunc(void)
{
    un_light_per_key_mode_params params; /**< 切换 per-key 模式时的参数（栈上） */

    s_mode = EN_LIGHTING_MODE_DIAMOND_RIPPLE;
    params.speed = Lighting_Speed_GetForMode(EN_LIGHTING_MODE_DIAMOND_RIPPLE);
    rgb_set_per_key_layer_enabled(true);
    rgb_set_base_mode(RGB_BASE_MODE_BLANK);
    Lighting_SetGlobalPerKeyMode(RGB_MODE_DIAMOND_RIPPLE, &params);
    Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE_DIAMOND_RIPPLE);
}

/** @brief 应用渐灭菱形波纹 per-key 模式 */
static void Lighting_FadingDiamondRippleModeFunc(void)
{
    un_light_per_key_mode_params params; /**< 切换 per-key 模式时的参数（栈上） */

    s_mode = EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE;
    params.speed = Lighting_Speed_GetForMode(EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE);
    rgb_set_per_key_layer_enabled(true);
    rgb_set_base_mode(RGB_BASE_MODE_BLANK);
    Lighting_SetGlobalPerKeyMode(RGB_MODE_FADING_DIAMOND_RIPPLE, &params);
    Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE);
}

/** @brief 应用泡泡 per-key 模式 */
static void Lighting_BubbleModeFunc(void)
{
    un_light_per_key_mode_params params; /**< 切换 per-key 模式时的参数（栈上） */

    s_mode = EN_LIGHTING_MODE_BUBBLE;
    params.speed = Lighting_Speed_GetForMode(EN_LIGHTING_MODE_BUBBLE);
    rgb_set_per_key_layer_enabled(true);
    rgb_set_base_mode(RGB_BASE_MODE_BLANK);
    Lighting_SetGlobalPerKeyMode(RGB_MODE_BUBBLE, &params);
    Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE_BUBBLE);
}

/** 模式应用函数指针类型 */
typedef void (*lighting_mode_func)(void);

/** 按 EN_LIGHTING_MODE 索引的模式应用函数表 */
static lighting_mode_func s_mode_func_array[EN_LIGHTING_MODE_MAX] = {
    [EN_LIGHTING_MODE_FIXED] = Lighting_FixedModeFunc,
    [EN_LIGHTING_MODE_CYCLE] = Lighting_CycleModeFunc,
    [EN_LIGHTING_MODE_RAINBOW] = Lighting_RainbowModeFunc,
    [EN_LIGHTING_MODE_WAVE] = Lighting_WaveModeFunc,
    [EN_LIGHTING_MODE_TRIGGER] = Lighting_TriggerModeFunc,
    [EN_LIGHTING_MODE_STRING] = Lighting_StringModeFunc,
    [EN_LIGHTING_MODE_FADING_STRING] = Lighting_FadingStringModeFunc,
    [EN_LIGHTING_MODE_DIAMOND_RIPPLE] = Lighting_DiamondRippleModeFunc,
    [EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE] = Lighting_FadingDiamondRippleModeFunc,
    [EN_LIGHTING_MODE_BUBBLE] = Lighting_BubbleModeFunc,
};

/** @brief 将 s_mode 写入 KEYBOARD_SETTING_LIGHTING_MODE */
static bool lighting_mode_sync_mode_param(void)
{
    uint8_t v;

    v = (uint8_t)s_mode;
    return DeviceParams_SetKeyboardParam(KEYBOARD_SETTING_LIGHTING_MODE, &v, sizeof(v), true);
}

/** @brief 将 s_toggle 写入 KEYBOARD_SETTING_LIGHTING_TOGGLE */
static bool lighting_mode_sync_toggle_param(void)
{
    return DeviceParams_SetKeyboardParam(KEYBOARD_SETTING_LIGHTING_TOGGLE, &s_toggle, sizeof(s_toggle), true);
}

/** @brief 应用当前模式与颜色；关屏时由 Color_Apply 末尾保持 OFF */
static void lighting_mode_apply_toggle_state(void)
{
    if (s_mode < EN_LIGHTING_MODE_MAX && s_mode_func_array[s_mode] != NULL) {
        s_mode_func_array[s_mode]();
    }
    Lighting_Color_Apply();
}

void Lighting_Mode_Init(void)
{
    uint8_t mode_stored = 0;   /**< 参数区模式索引 */
    uint8_t toggle_stored = 1; /**< 参数区开关 */

    if (DeviceParams_GetKeyboardParam(KEYBOARD_SETTING_LIGHTING_MODE, &mode_stored, sizeof(mode_stored))
        && mode_stored < EN_LIGHTING_MODE_MAX) {
        s_mode = (EN_LIGHTING_MODE)mode_stored;
    } else {
        s_mode = EN_LIGHTING_MODE_FIXED;
    }
    if (DeviceParams_GetKeyboardParam(KEYBOARD_SETTING_LIGHTING_TOGGLE, &toggle_stored, sizeof(toggle_stored))) {
        s_toggle = toggle_stored ? 1u : 0u;
    } else {
        s_toggle = 1u;
    }
    lighting_mode_apply_toggle_state();
    printf("[Lighting] Init mode: %d, toggle: %d\r\n", (int)s_mode, s_toggle);
}

bool Lighting_Mode_SwitchTo(EN_LIGHTING_MODE mode)
{
    if (mode >= EN_LIGHTING_MODE_MAX) {
        return false;
    }
    EN_LIGHTING_MODE last_mode = s_mode;
    bool ret = false;
    s_mode = mode;
    ret = lighting_mode_sync_mode_param();
    if (!ret) {
        s_mode = last_mode;
        printf("[Lighting] Switch to mode: %d failed, last mode: %d\r\n", (int)mode, (int)last_mode);
        return false;
    }
    if (s_mode_func_array[mode] != NULL) {
        s_mode_func_array[mode]();
    }
    if (s_toggle == 0u) {
        rgb_set_base_mode(RGB_BASE_MODE_OFF);
    }
    printf("[Lighting] Switch to mode: %d toggle: %d\r\n", (int)mode, s_toggle);
    return true;
}

uint8_t Lighting_Mode_GetToggle(void)
{
    return s_toggle;
}

void Lighting_Mode_SwitchNext(void)
{
    EN_LIGHTING_MODE next;

    next = (EN_LIGHTING_MODE)((s_mode + 1u) % EN_LIGHTING_MODE_MAX);
    Lighting_Mode_SwitchTo(next);
}

void Lighting_Mode_SwitchPrev(void)
{
    EN_LIGHTING_MODE prev;

    if (s_mode == 0) {
        prev = (EN_LIGHTING_MODE)(EN_LIGHTING_MODE_MAX - 1u);
    } else {
        prev = (EN_LIGHTING_MODE)(s_mode - 1u);
    }
    Lighting_Mode_SwitchTo(prev);
}

EN_LIGHTING_MODE Lighting_Mode_GetCurrent(void)
{
    return s_mode;
}

bool Lighting_Mode_SetToggle(uint8_t toggle)
{
    bool ret = false;
    s_toggle = toggle;
    ret = lighting_mode_sync_toggle_param();
    if (ret) {
        lighting_mode_apply_toggle_state();
    }
    printf("[Lighting] Toggle: %d, ret: %d\r\n", s_toggle, ret);
    return ret;
}

bool Lighting_Mode_OnToggleNotify(void)
{ 
    return Lighting_Mode_SetToggle(s_toggle ? 0u : 1u);
}

// 自动模式开始时的临时关灯，不进行参数区写入
void Lighting_Mode_OnHallCalibAutoModeSwitch(uint8_t status)
{
    static uint8_t last_toggle = 0u;
    if (status) {
        last_toggle = s_toggle;
        s_toggle = 0;
    } else {
        s_toggle = last_toggle;
    }
    lighting_mode_apply_toggle_state();
    printf("[Lighting] Auto mode start, toggle: %d\r\n", s_toggle);
}
