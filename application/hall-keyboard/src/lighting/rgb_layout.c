/**
 * @file rgb_layout.c
 * @brief 从 layout 构建灯珠坐标、slot 与 key 映射及 primary slot 逆映射
 */
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "lighting/rgb_layout.h"
#include "lighting/rgb.h"
#include "lighting/rgb_keyboard_shim.h"
#include "layout/layout.h"

static const st_keyboard_layout_mapping *g_pkb_layout = NULL;

/** 空格霍尔坐标（MUX8_K5），primary slot 选 x≈6.3 那颗 */
static const st_key_coord s_space_center_coord = {0u, 0u, 3u};

/** 各灯珠在键面上的物理坐标（微米，layout unit × KEY_SWITCH_DISTANCE） */
RGBLocation g_rgb_locations[RGB_NUM];

/** 灯珠 slot → 稠密 key_id；无效 coord 或 reserved 为 TOTAL_KEY_NUM */
uint16_t g_rgb_mapping[RGB_NUM];

/** 去重后的霍尔坐标表，下标即稠密 key_id */
static st_key_coord s_unique_coords[TOTAL_KEY_NUM];

/** coord 注册表（与 s_unique_coords 下标一一对应，当前恒等） */
static uint16_t s_coord_to_key_id[TOTAL_KEY_NUM];

/** 已注册的不重复 coord 数量 */
static uint16_t s_unique_coord_count;

/** 每个 key_id 的代表灯珠 slot（多灯共一键时取几何中心最近） */
static uint16_t s_primary_slot[TOTAL_KEY_NUM];

/** @brief 比较两个霍尔坐标是否相同 */
static int RgbLayout_CoordEqual(st_key_coord a, st_key_coord b)
{
    return (a.adc_value == b.adc_value)
        && (a.adc_channel == b.adc_channel)
        && (a.group == b.group);
}

/** @brief coord 是否有效（adc_value 非 0xFF） */
static uint8_t RgbLayout_CoordValid(st_key_coord c)
{
    return (c.adc_value != 0xFFu) ? 1u : 0u;
}

/**
 * @brief 注册 coord，返回稠密 key_id；已存在则返回已有 id
 * @param c 霍尔坐标
 */
static uint16_t RgbLayout_RegisterCoord(st_key_coord c)
{
    uint16_t i;

    for (i = 0u; i < s_unique_coord_count; i++) {
        if (RgbLayout_CoordEqual(s_unique_coords[i], c)) {
            return i;
        }
    }
    if (s_unique_coord_count >= TOTAL_KEY_NUM) {
        return TOTAL_KEY_NUM;
    }
    s_unique_coords[s_unique_coord_count] = c;
    s_coord_to_key_id[s_unique_coord_count] = s_unique_coord_count;
    s_unique_coord_count++;
    return (uint16_t)(s_unique_coord_count - 1u);
}

/**
 * @brief 霍尔坐标 → 稠密 key_id
 * @return key_id；无效或未找到返回 TOTAL_KEY_NUM
 */
uint16_t RgbLayout_CoordToKeyId(st_key_coord coord)
{
    uint16_t i;

    if (RgbLayout_CoordValid(coord) == 0u) {
        return TOTAL_KEY_NUM;
    }
    for (i = 0u; i < s_unique_coord_count; i++) {
        if (RgbLayout_CoordEqual(s_unique_coords[i], coord)) {
            return i;
        }
    }
    return TOTAL_KEY_NUM;
}

/**
 * @brief 获取 key_id 的代表灯珠 slot
 * @return slot 索引；无效返回 RGB_NUM
 */
uint16_t RgbLayout_GetPrimarySlot(uint16_t key_id)
{
    if (key_id >= TOTAL_KEY_NUM) {
        return RGB_NUM;
    }
    return s_primary_slot[key_id];
}

/** @brief 为每个 key_id 计算 primary slot（空格特殊：x 最接近 6.3） */
static void RgbLayout_BuildPrimarySlots(void)
{
    uint16_t key_id;
    uint16_t slot;

    for (key_id = 0u; key_id < TOTAL_KEY_NUM; key_id++) {
        s_primary_slot[key_id] = RGB_NUM;
    }

    for (key_id = 0u; key_id < s_unique_coord_count; key_id++) {
        float sum_x = 0.f;
        float sum_y = 0.f;
        uint16_t count = 0u;
        uint16_t best_slot = RGB_NUM;
        float best_score = 1e9f;
        st_key_coord coord = s_unique_coords[key_id];

        for (slot = 0u; slot < RGB_NUM; slot++) {
            if (g_rgb_mapping[slot] != key_id) {
                continue;
            }
            sum_x += g_pkb_layout[slot].location.x;
            sum_y += g_pkb_layout[slot].location.y;
            count++;
        }
        if (count == 0u) {
            continue;
        }

        if (RgbLayout_CoordEqual(coord, s_space_center_coord) != 0) {
            for (slot = 0u; slot < RGB_NUM; slot++) {
                float dx;
                if (g_rgb_mapping[slot] != key_id) {
                    continue;
                }
                dx = fabsf(g_pkb_layout[slot].location.x - 6.3f);
                if (dx < best_score) {
                    best_score = dx;
                    best_slot = slot;
                }
            }
        } else {
            float cx = sum_x / (float)count;
            float cy = sum_y / (float)count;
            for (slot = 0u; slot < RGB_NUM; slot++) {
                float dx;
                float dy;
                float dist;
                if (g_rgb_mapping[slot] != key_id) {
                    continue;
                }
                dx = g_pkb_layout[slot].location.x - cx;
                dy = g_pkb_layout[slot].location.y - cy;
                dist = dx * dx + dy * dy;
                if (dist < best_score) {
                    best_score = dist;
                    best_slot = slot;
                }
            }
        }
        s_primary_slot[key_id] = best_slot;
    }
}

/** @brief 从 g_pkb_layout 构建 locations/mapping 并计算 primary slot */
void RgbLayout_Init(void)
{
    uint16_t slot;
    g_pkb_layout = KeyboardLayout_GetLayoutPtr();
    if (g_pkb_layout == NULL) {
        return;
    }
    s_unique_coord_count = 0u;
    memset(s_coord_to_key_id, 0, sizeof(s_coord_to_key_id));
    memset(s_primary_slot, 0xFF, sizeof(s_primary_slot));

    for (slot = 0u; slot < RGB_NUM; slot++) {
        st_led_rgb_location_unit loc = g_pkb_layout[slot].location;
        st_key_coord coord = g_pkb_layout[slot].key_idx;
        uint16_t key_id;

        g_rgb_locations[slot].x = (int32_t)(loc.x * (float)KEY_SWITCH_DISTANCE);
        g_rgb_locations[slot].y = (int32_t)(loc.y * (float)KEY_SWITCH_DISTANCE);

        if (RgbLayout_CoordValid(coord) == 0u) {
            g_rgb_mapping[slot] = TOTAL_KEY_NUM;
        } else {
            key_id = RgbLayout_RegisterCoord(coord);
            g_rgb_mapping[slot] = key_id;
        }
    }

    RgbLayout_BuildPrimarySlots();
}

/** @brief 用 primary slot 填充 g_rgb_inverse_mapping（须在 rgb_init 之后调用） */
void RgbLayout_FinalizeInverseMapping(void)
{
    uint16_t key_id;

    memset(g_rgb_inverse_mapping, 0xFF, sizeof(g_rgb_inverse_mapping));
    for (key_id = 0u; key_id < s_unique_coord_count; key_id++) {
        if (s_primary_slot[key_id] < RGB_NUM) {
            g_rgb_inverse_mapping[key_id] = s_primary_slot[key_id];
        }
    }
}

