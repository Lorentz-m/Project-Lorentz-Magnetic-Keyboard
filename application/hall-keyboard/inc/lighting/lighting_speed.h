/**
 * @file lighting_speed.h
 * @brief Per-mode int16 speed; DeviceParams.lighting_speed[10] stores raw uint8 per mode
 */
#ifndef LIGHTING_SPEED_H
#define LIGHTING_SPEED_H

#include "lighting/lighting_mode.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Load from DeviceParams, fill defaults, write back */
void Lighting_Speed_Init(void);

/** @brief Current mode speed += step (clamped) */
void Lighting_Speed_Add(void);

/** @brief Current mode speed -= step (clamped) */
void Lighting_Speed_Sub(void);

/** @brief Factory default int16 speed for mode */
int16_t Lighting_Speed_GetDefaultForMode(EN_LIGHTING_MODE mode);

/** @brief Current managed int16 speed for mode */
int16_t Lighting_Speed_GetForMode(EN_LIGHTING_MODE mode);

/** @brief Write s_speed[mode] to all g_rgb_configs[].speed */
void Lighting_Speed_ApplyForMode(EN_LIGHTING_MODE mode);

#ifdef __cplusplus
}
#endif

#endif /* LIGHTING_SPEED_H */
