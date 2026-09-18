#ifndef RGB_LAYOUT_H
#define RGB_LAYOUT_H

#include <stdint.h>
#include "device/keyboard_config.h"
#include "lighting/rgb.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 从 layout 构建灯珠坐标与 slot/key 映射 */
void RgbLayout_Init(void);
/** @brief 按 primary slot 写入 g_rgb_inverse_mapping */
void RgbLayout_FinalizeInverseMapping(void);
/** @brief 霍尔坐标转稠密 key_id */
uint16_t RgbLayout_CoordToKeyId(st_key_coord coord);
/** @brief 获取 key_id 的代表灯珠 slot */
uint16_t RgbLayout_GetPrimarySlot(uint16_t key_id);

#ifdef __cplusplus
}
#endif

#endif
