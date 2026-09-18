/*
 * Copyright (c) 2024 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RGB_H_
#define RGB_H_

#include "lighting/color.h"
#include "lighting/rgb_config.h"
#include "lighting/rgb_keyboard_shim.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RGB_MAX_DURATION
#define RGB_MAX_DURATION 1000
#endif

#ifndef FADING_DISTANCE
#define FADING_DISTANCE 5.0f
#endif
/** 渐隐波纹距离（微米） */
#define FADING_DISTANCE_UM UNIT_TO_UM(FADING_DISTANCE)

#ifndef JELLY_DISTANCE
#define JELLY_DISTANCE 5.0f
#endif
/** 果冻晕染最大距离（微米） */
#define JELLY_DISTANCE_UM UNIT_TO_UM(JELLY_DISTANCE)

#ifndef BUBBLE_DISTANCE
#define BUBBLE_DISTANCE 2.5f
#endif
/** 气泡涟漪半径（微米） */
#define BUBBLE_DISTANCE_UM UNIT_TO_UM(BUBBLE_DISTANCE)

#ifndef PORT_LOCATION
#define PORT_LOCATION {UNIT_TO_UM(0), UNIT_TO_UM(0)}
#endif

#ifndef RGB_FLASH_MAX_DURATION
#define RGB_FLASH_MAX_DURATION 1000
#endif

#ifndef RGB_FLASH_RIPPLE_SPEED
#define RGB_FLASH_RIPPLE_SPEED 500
#endif

#ifndef RGB_DEFAULT_MODE
#define RGB_DEFAULT_MODE RGB_MODE_FIXED
#endif

#ifndef RGB_DEFAULT_SPEED
#define RGB_DEFAULT_SPEED 50
#endif

/* CYCLE per-key: full hue rotation period (ms) when speed=RGB_CYCLE_PERKEY_SPEED */
#ifndef RGB_CYCLE_HUE_PERIOD_MS
#define RGB_CYCLE_HUE_PERIOD_MS     5000u
#endif

#ifndef RGB_DEFAULT_COLOR_HSV
#define RGB_DEFAULT_COLOR_HSV {240, 78, 99}
#endif

#define RGB_LEFT_UM UNIT_TO_UM(RGB_LEFT)
#define RGB_TOP_UM UNIT_TO_UM(RGB_TOP)
#define RGB_RIGHT_UM UNIT_TO_UM(RGB_RIGHT)
#define RGB_BOTTOM_UM UNIT_TO_UM(RGB_BOTTOM)

#ifndef RGB_DEPTH
#define RGB_DEPTH 0.0f
#endif
#define RGB_DEPTH_UM UNIT_TO_UM(RGB_DEPTH)

#define RGB_WIDTH  ((RGB_RIGHT)-(RGB_LEFT))
#define RGB_WIDTH_UM ((RGB_RIGHT_UM)-(RGB_LEFT_UM))
#define RGB_HEIGHT ((RGB_BOTTOM)-(RGB_TOP))
#define RGB_HEIGHT_UM ((RGB_BOTTOM_UM)-(RGB_TOP_UM))

#ifndef RGB_GAMMA
#define RGB_GAMMA 2.2f
#endif

#ifndef RGB_ARGUMENT_LIST_BUFFER_LENGTH
#define RGB_ARGUMENT_LIST_BUFFER_LENGTH 64
#endif

/** layout unit 转微米的比例系数 */
#define KEY_SWITCH_DISTANCE 19050

#define CM_TO_UM(cm) ((int32_t)((cm)*10000))
#define UNIT_TO_UM(unit) ((int32_t)((unit)*KEY_SWITCH_DISTANCE))

#define GAMMA_CORRECT(value, max) (powf(((float)value)/(max), RGB_GAMMA)*(max))

/** 底座灯效模式 */
typedef enum __RGBBaseMode
{
    RGB_BASE_MODE_OFF,      /* 关闭：底座不渲染，整帧熄灭 */
    RGB_BASE_MODE_BLANK,    /* 空白：仅保留 per-key 层，无底座动画 */
    RGB_BASE_MODE_RAINBOW,  /* 彩虹：按位置与方向滚动色相 */
    RGB_BASE_MODE_WAVE,     /* 波浪：双色在键面方向交替过渡 */
} RGBBaseMode;

/** 底座灯效全局配置 */
typedef struct __RGBBaseConfig
{
    RGBBaseMode mode;           /* 底座灯效模式 */
    ColorRGB rgb;               /* 主色（RGB，Wave 等） */
    ColorHSV hsv;               /* 主色（HSV，Rainbow 等） */
    ColorRGB secondary_rgb;     /* 副色 RGB（Wave 第二色） */
    ColorHSV secondary_hsv;     /* 副色 HSV */
    int16_t speed;              /* 底座动画速度系数 */
    uint32_t begin_tick;        /* 底座效果起始 tick（ms） */
    uint16_t direction;         /* 渐变方向，单位：度（0~360） */
    uint8_t density;            /* 色相/条纹密度 */
    uint8_t brightness;         /* 全局亮度（0~255，作用于 rgb_set） */
} RGBBaseConfig;

/** 单灯珠 per-key 灯效模式 */
typedef enum __RGBMode
{
    RGB_MODE_FIXED,                 /* 固定：静态显示底座主色 */
    RGB_MODE_STATIC,                /* 静态：与底色混合叠加 overlay 色（非底座色） */
    RGB_MODE_CYCLE,                 /* 循环：色相随时间旋转 */
    RGB_MODE_LINEAR,                /* 线性：亮度随按键行程 analog 变化 */
    RGB_MODE_TRIGGER,               /* 触发：按下后按速度指数衰减 */
    RGB_MODE_STRING,                /* 弦波涟漪：自触发点横向扩散 */
    RGB_MODE_FADING_STRING,         /* 渐隐弦波：带尾迹的横向涟漪 */
    RGB_MODE_DIAMOND_RIPPLE,        /* 菱形涟漪：曼哈顿距离扩散 */
    RGB_MODE_FADING_DIAMOND_RIPPLE, /* 渐隐菱形涟漪 */
    RGB_MODE_JELLY,                 /* 果冻：邻键随行程联动晕染 */
    RGB_MODE_BUBBLE,                /* 气泡：欧氏距离范围内的涟漪 */
} RGBMode;

/** 单灯珠 slot 的 per-key 配置 */
typedef struct __RGBConfig
{
    RGBMode mode;       /**< 该 slot 灯效模式 */
    ColorRGB rgb;       /**< 配置色 RGB */
    ColorHSV hsv;       /**< 配置色 HSV（CYCLE 等） */
    int16_t speed;      /**< 效果速度系数 */
    uint32_t begin_tick; /**< 效果起始 tick（TRIGGER/波纹） */
} RGBConfig;

/** 灯珠在键面上的物理坐标（微米） */
typedef struct __RGBLocation
{
    int32_t x;  /**< 横坐标 */
    int32_t y;  /**< 纵坐标 */
}RGBLocation;

/** 波纹/串灯链表节点携带的参数 */
typedef struct __RGBArgument
{
    uint32_t begin_tick; /**< 触发时刻 tick */
    uint8_t rgb_ptr;     /**< 触发点灯珠 slot 索引 */
}RGBArgument;

/** 波纹链表节点 */
typedef struct __RGBArgumentListNode
{
    RGBArgument data; /**< 节点数据 */
    int16_t next;     /**< 下一节点索引，-1 表示无 */
} RGBArgumentListNode;

/** 波纹参数单向链表（索引式） */
typedef struct __RGBArgumentList
{
    RGBArgumentListNode *data; /**< 节点池指针 */
    int16_t head;              /**< 头结点索引 */
    int16_t tail;              /**< 尾索引（预留） */
    int16_t len;               /**< 节点池长度 */
    int16_t free_node;         /**< 空闲链头索引 */
} RGBArgumentList;

void rgb_forward_list_init(RGBArgumentList* list, RGBArgumentListNode*data, uint16_t len);
void rgb_forward_list_erase_after(RGBArgumentList* list, RGBArgumentListNode*data);
void rgb_forward_list_insert_after(RGBArgumentList* list, RGBArgumentListNode* data, RGBArgument t);
void rgb_forward_list_push_front(RGBArgumentList* list, RGBArgument t);

/** 本帧各 slot 合成 RGB 缓冲 */
extern ColorRGB g_rgb_colors[RGB_NUM];
// /** HID 接管灯效标志 */
// extern volatile bool g_rgb_hid_mode;

#ifndef RGB_CUSTOM_INVERSE_MAPPING
/** key_id → 代表 slot */
extern uint16_t g_rgb_inverse_mapping[TOTAL_KEY_NUM];
#else
extern const uint16_t g_rgb_inverse_mapping[TOTAL_KEY_NUM];
#endif
/** 各 slot per-key 配置 */
extern RGBConfig g_rgb_configs[RGB_NUM];
/** slot → 稠密 key_id */
extern uint16_t g_rgb_mapping[RGB_NUM];
/** 各 slot 物理坐标（微米） */
extern RGBLocation g_rgb_locations[RGB_NUM];

void rgb_init(void);
void rgb_process(void);

/** 配置变更后请求下一帧刷新（静态帧缓存失效） */
void rgb_invalidate(void);

/** 是否仍需 rgb_process（动画进行中或静态帧未出） */
bool rgb_needs_process(void);

void rgb_update_callback(void);
void rgb_set(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
void rgb_init_flash(void);
void rgb_flash(void);
void rgb_turn_off(void);
void rgb_flush(void);
void rgb_factory_reset(void);
void rgb_activate(uint16_t id, uint32_t tick);
void rgb_clear_ripple_list(void);

void rgb_set_base_mode(RGBBaseMode mode);
void rgb_set_base_color(const ColorRGB *rgb);
/**
 * @brief 设置底座全局亮度 0~255，在 rgb_set 出帧时缩放 RGB
 * @param brightness 亮度值，0 为最暗，255 为满挡（出帧时约 ×255/256）
 */
void rgb_set_base_brightness(uint8_t brightness);
/** 开关 per-key/波纹层；false 时 rgb_process 只渲染底座 */
void rgb_set_per_key_layer_enabled(bool enabled);
bool rgb_is_per_key_layer_enabled(void);

#ifdef __cplusplus
}
#endif

#endif /* RGB_H_ */

