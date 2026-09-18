/**
 * @file lighting_speed.c
 * @brief 各模式灯效速度 int16：与 DeviceParams.lighting_speed[10] 同步（原值，无档位映射）
 */
#include <string.h>
#include <stdio.h>

#include "lighting/lighting_speed.h"
#include "device/device_params.h"
#include "lighting/rgb.h"

/** 循环变色模式默认引擎速度 */
#define LIGHT_MODE_DEFAULT_SPEED_CYCLE                   1
/** 触发灯模式默认引擎速度 */
#define LIGHT_MODE_DEFAULT_SPEED_TRIGGER                 20
/** 串灯模式默认引擎速度 */
#define LIGHT_MODE_DEFAULT_SPEED_STRING                  150
/** 渐灭串灯模式默认引擎速度 */
#define LIGHT_MODE_DEFAULT_SPEED_FADING_STRING           150
/** 菱形波纹模式默认引擎速度 */
#define LIGHT_MODE_DEFAULT_SPEED_DIAMOND_RIPPLE          150
/** 渐灭菱形波纹模式默认引擎速度 */
#define LIGHT_MODE_DEFAULT_SPEED_FADING_DIAMOND_RIPPLE   150
/** 泡泡模式默认引擎速度 */
#define LIGHT_MODE_DEFAULT_SPEED_BUBBLE                  100

/** 循环变色速度上限 */
#define LIGHT_MODE_MAX_SPEED_CYCLE                       10
/** 触发灯速度上限 */
#define LIGHT_MODE_MAX_SPEED_TRIGGER                     60
/** 串灯速度上限 */
#define LIGHT_MODE_MAX_SPEED_STRING                      250
/** 渐灭串灯速度上限 */
#define LIGHT_MODE_MAX_SPEED_FADING_STRING               250
/** 菱形波纹速度上限 */
#define LIGHT_MODE_MAX_SPEED_DIAMOND_RIPPLE              250
/** 渐灭菱形波纹速度上限 */
#define LIGHT_MODE_MAX_SPEED_FADING_DIAMOND_RIPPLE       250
/** 泡泡速度上限 */
#define LIGHT_MODE_MAX_SPEED_BUBBLE                      200

/** 循环变色速度下限 */
#define LIGHT_MODE_MIN_SPEED_CYCLE                       1
/** 触发灯速度下限 */
#define LIGHT_MODE_MIN_SPEED_TRIGGER                     5
/** 串灯速度下限 */
#define LIGHT_MODE_MIN_SPEED_STRING                      50
/** 渐灭串灯速度下限 */
#define LIGHT_MODE_MIN_SPEED_FADING_STRING               50
/** 菱形波纹速度下限 */
#define LIGHT_MODE_MIN_SPEED_DIAMOND_RIPPLE              50
/** 渐灭菱形波纹速度下限 */
#define LIGHT_MODE_MIN_SPEED_FADING_DIAMOND_RIPPLE       50
/** 泡泡速度下限 */
#define LIGHT_MODE_MIN_SPEED_BUBBLE                      50

/** 循环变色 Add/Sub 步进 */
#define LIGHT_MODE_STEP_SPEED_CYCLE                      1
/** 触发灯 Add/Sub 步进 */
#define LIGHT_MODE_STEP_SPEED_TRIGGER                    5
/** 串灯 Add/Sub 步进 */
#define LIGHT_MODE_STEP_SPEED_STRING                     10
/** 渐灭串灯 Add/Sub 步进 */
#define LIGHT_MODE_STEP_SPEED_FADING_STRING              10
/** 菱形波纹 Add/Sub 步进 */
#define LIGHT_MODE_STEP_SPEED_DIAMOND_RIPPLE             10
/** 渐灭菱形波纹 Add/Sub 步进 */
#define LIGHT_MODE_STEP_SPEED_FADING_DIAMOND_RIPPLE      10
/** 泡泡 Add/Sub 步进 */
#define LIGHT_MODE_STEP_SPEED_BUBBLE                     10

/** 各 EN_LIGHTING_MODE 当前引擎速度（int16），与 DeviceParams.lighting_speed[] 一一对应 */
static int16_t s_speed[EN_LIGHTING_MODE_MAX];

/**
 * 各模式是否支持调速：0 表示不支持；非 0 为 Lighting_Speed_Add/Sub 的步进值
 * FIXED / RAINBOW / WAVE 为 0
 */
static const uint16_t s_speed_step[EN_LIGHTING_MODE_MAX] = {
    [EN_LIGHTING_MODE_FIXED] = 0,
    [EN_LIGHTING_MODE_CYCLE] = LIGHT_MODE_STEP_SPEED_CYCLE,
    [EN_LIGHTING_MODE_RAINBOW] = 0,
    [EN_LIGHTING_MODE_WAVE] = 0,
    [EN_LIGHTING_MODE_TRIGGER] = LIGHT_MODE_STEP_SPEED_TRIGGER,
    [EN_LIGHTING_MODE_STRING] = LIGHT_MODE_STEP_SPEED_STRING,
    [EN_LIGHTING_MODE_FADING_STRING] = LIGHT_MODE_STEP_SPEED_FADING_STRING,
    [EN_LIGHTING_MODE_DIAMOND_RIPPLE] = LIGHT_MODE_STEP_SPEED_DIAMOND_RIPPLE,
    [EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE] = LIGHT_MODE_STEP_SPEED_FADING_DIAMOND_RIPPLE,
    [EN_LIGHTING_MODE_BUBBLE] = LIGHT_MODE_STEP_SPEED_BUBBLE,
};

/**
 * @brief 查询指定模式的 int16 速度上下界
 * @param mode    用户灯效模式
 * @param min_out 输出下限
 * @param max_out 输出上限
 */
static void lighting_speed_get_limits(EN_LIGHTING_MODE mode, int16_t *min_out, int16_t *max_out)
{
    switch (mode) {
    case EN_LIGHTING_MODE_CYCLE:
        *min_out = LIGHT_MODE_MIN_SPEED_CYCLE;
        *max_out = LIGHT_MODE_MAX_SPEED_CYCLE;
        break;
    case EN_LIGHTING_MODE_TRIGGER:
        *min_out = LIGHT_MODE_MIN_SPEED_TRIGGER;
        *max_out = LIGHT_MODE_MAX_SPEED_TRIGGER;
        break;
    case EN_LIGHTING_MODE_STRING:
        *min_out = LIGHT_MODE_MIN_SPEED_STRING;
        *max_out = LIGHT_MODE_MAX_SPEED_STRING;
        break;
    case EN_LIGHTING_MODE_FADING_STRING:
        *min_out = LIGHT_MODE_MIN_SPEED_FADING_STRING;
        *max_out = LIGHT_MODE_MAX_SPEED_FADING_STRING;
        break;
    case EN_LIGHTING_MODE_DIAMOND_RIPPLE:
        *min_out = LIGHT_MODE_MIN_SPEED_DIAMOND_RIPPLE;
        *max_out = LIGHT_MODE_MAX_SPEED_DIAMOND_RIPPLE;
        break;
    case EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE:
        *min_out = LIGHT_MODE_MIN_SPEED_FADING_DIAMOND_RIPPLE;
        *max_out = LIGHT_MODE_MAX_SPEED_FADING_DIAMOND_RIPPLE;
        break;
    case EN_LIGHTING_MODE_BUBBLE:
        *min_out = LIGHT_MODE_MIN_SPEED_BUBBLE;
        *max_out = LIGHT_MODE_MAX_SPEED_BUBBLE;
        break;
    default:
        *min_out = 0;
        *max_out = 0;
        break;
    }
}

/** @brief 将 s_speed[mode] 钳位到该模式 min/max（不可调模式直接返回） */
static void lighting_speed_clamp_mode(EN_LIGHTING_MODE mode)
{
    int16_t min_spd;
    int16_t max_spd;

    if (s_speed_step[mode] == 0) {
        return;
    }
    lighting_speed_get_limits(mode, &min_spd, &max_spd);
    if (s_speed[mode] < min_spd) {
        s_speed[mode] = min_spd;
    }
    if (s_speed[mode] > max_spd) {
        s_speed[mode] = max_spd;
    }
}

/** @brief 将全部灯珠 slot 的 g_rgb_configs[].speed 设为同一 int16 值 */
static void lighting_speed_apply_int16_to_slots(int16_t speed)
{
    uint16_t i;

    for (i = 0u; i < RGB_NUM; i++) {
        g_rgb_configs[i].speed = speed;
    }
}

int16_t Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE mode)
{
    switch (mode) {
    case EN_LIGHTING_MODE_CYCLE:
        return LIGHT_MODE_DEFAULT_SPEED_CYCLE;
    case EN_LIGHTING_MODE_TRIGGER:
        return LIGHT_MODE_DEFAULT_SPEED_TRIGGER;
    case EN_LIGHTING_MODE_STRING:
        return LIGHT_MODE_DEFAULT_SPEED_STRING;
    case EN_LIGHTING_MODE_FADING_STRING:
        return LIGHT_MODE_DEFAULT_SPEED_FADING_STRING;
    case EN_LIGHTING_MODE_DIAMOND_RIPPLE:
        return LIGHT_MODE_DEFAULT_SPEED_DIAMOND_RIPPLE;
    case EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE:
        return LIGHT_MODE_DEFAULT_SPEED_FADING_DIAMOND_RIPPLE;
    case EN_LIGHTING_MODE_BUBBLE:
        return LIGHT_MODE_DEFAULT_SPEED_BUBBLE;
    default:
        return 0;
    }
}

int16_t Lighting_Speed_GetForMode(EN_LIGHTING_MODE mode)
{
    if (mode >= EN_LIGHTING_MODE_MAX) {
        return 0;
    }
    return s_speed[mode];
}

void Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE mode)
{
    if (mode >= EN_LIGHTING_MODE_MAX || s_speed_step[mode] == 0) {
        return;
    }
    lighting_speed_apply_int16_to_slots(s_speed[mode]);
}

/** @brief 从 DeviceParams 加载 lighting_speed[10] 到 s_speed，字节为 0 时用默认速度并钳位 */
static void lighting_speed_load_from_params(void)
{
    uint8_t stored[EN_LIGHTING_MODE_MAX]; /**< 参数区读出的 10 字节原值 */
    EN_LIGHTING_MODE mode;

    if (!DeviceParams_GetKeyboardParam(KEYBOARD_SETTING_LIGHTING_SPEED,
            stored, sizeof(stored))) {
        memset(stored, 0, sizeof(stored));
    }
    for (mode = EN_LIGHTING_MODE_FIXED; mode < EN_LIGHTING_MODE_MAX; mode++) {
        if (s_speed_step[mode] == 0) {
            s_speed[mode] = 0;
            continue;
        }
        if (stored[mode] == 0) {
            s_speed[mode] = Lighting_Speed_GetDefaultForMode(mode);
        } else {
            s_speed[mode] = (int16_t)stored[mode];
        }
        lighting_speed_clamp_mode(mode);
    }
}

/** @brief 将 s_speed[] 写回 KEYBOARD_SETTING_LIGHTING_SPEED（按 uint8 截断） */
static void lighting_speed_sync_params(void)
{
    uint8_t out[EN_LIGHTING_MODE_MAX]; /**< 写入参数区的 10 字节 */
    EN_LIGHTING_MODE mode;

    for (mode = EN_LIGHTING_MODE_FIXED; mode < EN_LIGHTING_MODE_MAX; mode++) {
        if (s_speed[mode] <= 0) {
            out[mode] = 0;
        } else if (s_speed[mode] > 255) {
            out[mode] = 255;
        } else {
            out[mode] = (uint8_t)s_speed[mode];
        }
    }
    DeviceParams_SetKeyboardParam(KEYBOARD_SETTING_LIGHTING_SPEED, out, sizeof(out), true);
}

void Lighting_Speed_Init(void)
{
    lighting_speed_load_from_params();
    lighting_speed_sync_params();
}

/**
 * @brief 调节当前用户模式的速度
 * @param sign 正数加速，负数减速（步进见 s_speed_step）
 */
static void lighting_speed_adjust_current(int8_t sign)
{
    EN_LIGHTING_MODE mode;
    uint16_t step;
    int32_t next;

    mode = Lighting_Mode_GetCurrent();
    step = s_speed_step[mode];
    if (step == 0) {
        printf("[Lighting] Speed Adjust: step is 0\r\n");
        return;
    }
    next = (int32_t)s_speed[mode] + (int32_t)step * (int32_t)sign;
    s_speed[mode] = (int16_t)next;
    lighting_speed_clamp_mode(mode);
    Lighting_Speed_ApplyForMode(mode);
    lighting_speed_sync_params();
    printf("[Lighting] Speed Adjust: %d mode: %d\r\n", s_speed[mode], mode);
}

void Lighting_Speed_Add(void)
{
    lighting_speed_adjust_current(1);
}

void Lighting_Speed_Sub(void)
{
    lighting_speed_adjust_current(-1);
}
