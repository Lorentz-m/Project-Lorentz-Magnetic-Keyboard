/**
 * @file lighting_mode.h
 * @brief 灯效模式切换、开关与全局 per-key 模式下发
 */
#ifndef LIGHTING_MODE_H
#define LIGHTING_MODE_H

#include <stdint.h>
#include "lighting/rgb.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 用户可见灯效模式，与 keyboard_config.lighting_mode 对应（速度存 DeviceParams.lighting_speed[10]，原值 int16，无档位映射） */
typedef enum {
    EN_LIGHTING_MODE_FIXED,                 /**< 固定色（per-key FIXED） */
    EN_LIGHTING_MODE_CYCLE,                 /**< 循环变色 */
    EN_LIGHTING_MODE_RAINBOW,               /**< 底座彩虹 */
    EN_LIGHTING_MODE_WAVE,                  /**< 底座波浪 */
    EN_LIGHTING_MODE_TRIGGER,               /**< 触发灯 */
    EN_LIGHTING_MODE_STRING,                /**< 串灯 */
    EN_LIGHTING_MODE_FADING_STRING,         /**< 渐灭串灯 */
    EN_LIGHTING_MODE_DIAMOND_RIPPLE,        /**< 菱形波纹 */
    EN_LIGHTING_MODE_FADING_DIAMOND_RIPPLE, /**< 渐灭菱形波纹 */
    EN_LIGHTING_MODE_BUBBLE,                /**< 泡泡 */
    EN_LIGHTING_MODE_MAX                    /**< 模式数量上界 */
} EN_LIGHTING_MODE;

/** 切换全局 per-key RGB 模式时的参数（按 RGBMode 选用其一） */
typedef union {
    ColorRGB static_overlay; /**< RGB_MODE_STATIC：叠加静态色 */
    int16_t speed;           /**< 带 speed 的 per-key 模式初始速度，0 表示用默认 */
} un_light_per_key_mode_params;

/** @brief 从 DeviceParams 恢复 lighting_mode / lighting_toggle 并应用 */
void Lighting_Mode_Init(void);

/** @brief 切换到下一模式（循环） */
void Lighting_Mode_SwitchNext(void);

/** @brief 切换到上一模式（循环） */
void Lighting_Mode_SwitchPrev(void);

/**
 * @brief 切换到指定模式；灯效关闭时先自动打开再应用
 * @param mode 目标 EN_LIGHTING_MODE
 */
bool Lighting_Mode_SwitchTo(EN_LIGHTING_MODE mode);

/** @brief 获取当前用户模式索引 */
EN_LIGHTING_MODE Lighting_Mode_GetCurrent(void);

/** @brief 获取灯效总开关状态 */
uint8_t Lighting_Mode_GetToggle(void);

/** @brief 设置 灯效的开关状态 */
bool Lighting_Mode_SetToggle(uint8_t toggle);

/** @brief 通知 lighting_toggle 对应的灯效状态机 */
bool Lighting_Mode_OnToggleNotify(void);

/** @brief 自动模式开始时的临时关灯 */
void Lighting_Mode_OnHallCalibAutoModeSwitch(uint8_t status);

/**
 * @brief 将全部灯珠 slot 设为同一 RGBMode，并清空波纹链表与 shim 键态
 * @param mode   目标 RGBMode
 * @param params 模式参数，NULL 时直接返回
 */
void Lighting_SetGlobalPerKeyMode(RGBMode mode, const un_light_per_key_mode_params *params);

#ifdef __cplusplus
}
#endif

#endif /* LIGHTING_MODE_H */
