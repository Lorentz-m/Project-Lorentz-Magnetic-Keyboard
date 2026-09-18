/**
 * @file lighting_color.h
 * @brief RGB color sync with DeviceParams.lighting_color
 */
#ifndef LIGHTING_COLOR_H
#define LIGHTING_COLOR_H

#include <stdint.h>
#include "lighting/color.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 从参数区恢复 RGB 并应用到 RGB 底座 */
void Lighting_Color_Init(void);

/** @brief 轮换到下一预设色并写回参数区 */
void Lighting_Color_SwitchNext(void);

/** @brief 将当前 RGB 应用到 rgb_set_base_color / 彩虹底座模式 */
void Lighting_Color_Apply(void);

/** @brief 拷贝当前 RGB 到 out_rgb[3] */
void Lighting_Color_GetRGB(uint8_t out_rgb[3]);

/**
 * @brief 设置 RGB 并同步到参数区
 * @param rgb 3 字节 RGB 数组，NULL 时忽略
 */
bool Lighting_Color_SetBaseColor(const uint8_t rgb[3]);

#ifdef __cplusplus
}
#endif

#endif /* LIGHTING_COLOR_H */
