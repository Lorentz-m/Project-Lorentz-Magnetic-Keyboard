/**
 * @file lighting_brightness.h
 * @brief 全局亮度 0~255，步进调节并同步 DeviceParams.lighting_brightness
 */
#ifndef LIGHTING_BRIGHTNESS_H
#define LIGHTING_BRIGHTNESS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 从参数区加载亮度并写入 rgb_set_base_brightness */
void Lighting_Brightness_Init(void);

/** @brief 亮度增加一步（+10，封顶 255） */
void Lighting_Brightness_Add(void);

/** @brief 亮度减少一步（-10，不低于 0） */
void Lighting_Brightness_Sub(void);

/** @brief 设置亮度 */
bool Lighting_Brightness_Set(uint8_t brightness);

#ifdef __cplusplus
}
#endif

#endif /* LIGHTING_BRIGHTNESS_H */
