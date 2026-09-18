/**
 * @file lighting_key.h
 * @brief 灯效物理键输入：坐标/行程馈入与按键边沿触发
 */
#ifndef LIGHTING_KEY_H
#define LIGHTING_KEY_H

#include <stdint.h>
#include "device/keyboard_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 行程用于灯效强度的上限（微米），超过按此值按满强度计算 */
#define LIGHTING_KEY_DIST_UM_MAX  3500u

/**
 * @brief 按 ADC 坐标与行程更新灯效键态（shim + 边沿触发）
 * @param coord   物理键 ADC 坐标（layout 去重前）
 * @param dist_um 当前行程（微米），用于 LINEAR/JELLY 等强度
 * @param pressed 是否按下（0/1）
 */
void Lighting_SetKeyStateAtCoord(st_key_coord coord, uint16_t dist_um, uint8_t pressed);

#ifdef __cplusplus
}
#endif

#endif /* LIGHTING_KEY_H */
