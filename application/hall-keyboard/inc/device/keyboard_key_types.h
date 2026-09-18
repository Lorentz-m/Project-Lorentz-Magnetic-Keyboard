#ifndef KEYBOARD_KEY_TYPES_H
#define KEYBOARD_KEY_TYPES_H

#include <stdint.h>

#pragma pack(push, 1)

//按键坐标结构体（用于唯一标识一个按键）
typedef struct {
    uint8_t adc_value;   /* ADC槽位索引 (0~ADC_VALUE_MAXNUM-1) */
    uint8_t adc_channel; /* ADC通道号 (0~ADC_CHANNEL_MAXNUM-1) */
    uint8_t group;       /* 按键分组号 (0~GROUP_MAXNUM-1) */
} st_key_coord;

typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_PRESS,
    KEY_EVENT_RELEASE,
} EN_KEY_EVENT;

#define KEY_PLATFORM_DIST_JUDGE_STEP_UM  (100)

typedef struct {
    uint8_t group;
    uint8_t adc_idx;
    uint8_t ch;
    EN_KEY_EVENT event;
} st_key_event_t;

typedef enum {
    /* Normal mode: absolute threshold; shared normal_mode_travel_um (see device/param_defaults.h). */
    HALL_KEY_RT_OFF = 0,
    /* RT: relative_threshold anchors */
    HALL_KEY_RT_ON = 1
} EN_HALL_KEY_RT_MODE;

typedef enum {
    CHECK_STATE_UNCALIBRATED = 0,
    CHECK_STATE_CALIBRATED = 1,
    CHECK_STATE_ERROR = 2,
} EN_HALL_KEY_CHECK_STATE;

// 性能参数结构体
typedef struct {
    EN_HALL_KEY_RT_MODE rt_mode;     // 运行模式（RT模式开与否）
    uint8_t is_dist_monitor_mode;    // 是否开启行程监测模式
    uint16_t normal_mode_travel_um;  // 非RT模式行程um
    uint16_t rt_press_um;            // RT模式按下阈值um
    uint16_t rt_release_um;          // RT模式释放阈值um
    uint16_t top_dead_zone_um;       // 上死区(0=0um)
    uint16_t bottom_dead_zone_um;    // 下死区(0=0um)
    uint8_t check_state;             // @EN_HALL_KEY_CHECK_STATE校准状态(0=未校准，1=已校准，2=按键状态异常(校准模式下识别为异常状态))
} st_key_performance_params;

typedef enum {
    // 纯粹按键值模式（默认+自定义键值）
    KEY_VAULE_MODE_NORMAL = 0,
    // 多媒体层键值模式
    KEY_VAULE_MODE_CONSUMER,
    // 组合按键模式
    KEY_VAULE_MODE_VENDOR,
    // 宏模式
    KEY_VAULE_MODE_MACRO,
    // 自定义功能模式
    KEY_VAULE_MODE_CUSTOM,
} EN_KEY_VAULE_MODE;

//自定义功能枚举
typedef enum {
    // 空键
    CUSTOM_FUNC_EMPTY = 0,
    // FN层开关功能
    CUSTOM_FUNC_ON_FN1,
    CUSTOM_FUNC_ON_FN2,
    CUSTOM_FUNC_ON_FN3,
    // window模式切换功能
    CUSTOM_FUNC_SET_WINDOW_MODE,
    // mac模式切换功能
    CUSTOM_FUNC_SET_MAC_MODE,
    // 恢复出厂设置
    CUSTOM_FUNC_FACTORY_RESET,
    // 灯效模式切换
    CUSTOM_FUNC_LIGHTING_MODE_SWITCH,
    // 灯效颜色切换
    CUSTOM_FUNC_LIGHTING_COLOR_SWITCH,
    // 灯效亮度+
    CUSTOM_FUNC_LIGHTING_BRIGHTNESS_UP,
    // 灯效亮度-
    CUSTOM_FUNC_LIGHTING_BRIGHTNESS_DOWN,
    // 灯效速度+
    CUSTOM_FUNC_LIGHTING_SPEED_UP,
    // 灯效速度-
    CUSTOM_FUNC_LIGHTING_SPEED_DOWN,
    // 灯效上一个模式
    CUSTOM_FUNC_LIGHTING_LAST_MODE,
    // 开关灯效
    CUSTOM_FUNC_LIGHTING_TOGGLE,
    // 穿透键
    CUSTOM_FUNC_THROUGH_KEY,
} EN_CUSTOM_FUNCTION_MODE;

typedef union {
    uint8_t u8_value;
    uint8_t u8_value_array[2];
    uint16_t u16_value;
} un_custom_func_params;

typedef struct {
    EN_CUSTOM_FUNCTION_MODE mode;
    un_custom_func_params param;
} st_custom_func;

typedef enum {
    ADVANCED_FUNC_NONE = 0,            // 无功能 | None
    ADVANCED_FUNC_KEY_CONFLICT,        // 双键智能冲突处理 | Key Conflict Arbitration
    ADVANCED_FUNC_DYNAMIC_STROKE,      // 动态行程指令触发 | Dynamic Stroke Trigger
    ADVANCED_FUNC_SEGMENTED_STROKE,    // 行程分段触发 | Segmented Stroke Trigger
    ADVANCED_FUNC_TAP_HOLD,            // 长短按双功能映射 | Tap-Hold Dual Function
    ADVANCED_FUNC_FIRE_KEY,            // 火力键 | Fire Key
    ADVANCED_FUNC_REBOUND,             // 回弹触发 | Rebound Trigger
    ADVANCED_FUNC_RAPPY_SNAPPY,        // 迅洁(深度优先键) | Rappy Snappy
    ADVANCED_FUNC_MAX,                 // 功能总数上限
} EN_ADVANCED_MODE;

// 方向键冲突处理模式
typedef enum {
    DIR_CONFLICT_MODE_NONE = 0,      // 无冲突处理
    DIR_CONFLICT_MODE_LATER_COVER,   // 后覆盖：以后按下为主
    DIR_CONFLICT_MODE_A_PRIORITY,    // A优先：只输出A
    DIR_CONFLICT_MODE_B_PRIORITY,    // B优先：只输出B  
    DIR_CONFLICT_MODE_CANCEL,        // 相抵消：都按下时都不输出
    DIR_CONFLICT_MODE_MAX
} EN_DIR_CONFLICT_MODE;

// 方向键冲突处理参数
typedef struct {
    st_key_coord a_key;              // A键坐标
    st_key_coord b_key;              // B键坐标
    uint8_t a_key_hid;               // A键HID键码
    uint8_t b_key_hid;               // B键HID键码
    EN_DIR_CONFLICT_MODE conflict_mode;  // 冲突处理模式
    uint8_t delay_ms;                // 切换按键时的延时确认时间
} st_dir_conflict_inline;

#define ST_DYNAMIC_STROKE_GROUP_MAX 4u

typedef struct {
    uint8_t hid_key;
    uint8_t sel_mask;
} st_dynamic_stroke_group_item;

typedef struct {
    uint16_t dist_um[3];
    uint8_t group_counts;
    st_dynamic_stroke_group_item group[ST_DYNAMIC_STROKE_GROUP_MAX];
} st_dynamic_stroke_inline;

/** 行程分段高级键：最多 3 档，存于键值镜像（无 profile_id） */
#define ST_SEGMENTED_STROKE_ACTION_MAX 3u

/** 单档：触发行程与虚拟 HID 为一组 */
typedef struct {
    uint16_t dist_um;  /* 触发行程 μm，绑定时按升序写入 */
    uint8_t hid_key;   /* 虚拟输出 HID 键码 */
} st_segmented_stroke_action_item;

typedef struct {
    uint8_t action_nums;   /* 有效档位数，1~ST_SEGMENTED_STROKE_ACTION_MAX */
    st_segmented_stroke_action_item action[ST_SEGMENTED_STROKE_ACTION_MAX];
} st_segmented_stroke_inline;

typedef struct {
    uint8_t tap_key;
    uint8_t hold_key;
    uint16_t touch_time_ms;
} st_tap_hold_dual_inline;

typedef struct {
    uint8_t fire_key;
} st_fire_key_inline;

typedef struct {
    uint8_t press_key;
    uint8_t rebound_key;
    uint8_t rebound_delay_ms;
} st_rebound_inline;

typedef struct {
    st_key_coord a_key;      /* A 侧物理坐标 */
    st_key_coord b_key;      /* B 侧物理坐标 */
    uint8_t a_key_hid;       /* A 侧虚拟输出 HID */
    uint8_t b_key_hid;       /* B 侧虚拟输出 HID */
    uint8_t delay_ms;        /* 切换前延时确认 ms */
} st_rappy_snappy_inline;

typedef struct {
    st_key_coord a_key;      /* A 侧物理坐标 */
    st_key_coord b_key;      /* B 侧物理坐标 */
} st_support_two_key_inline;

typedef union {
    st_support_two_key_inline support_two_key; // 该位作为快速获取支持双键高级模式的双键参数，故双键高级模式参数中必须包含该字段
    st_dir_conflict_inline dir_conflict;  // 方向键冲突处理参数
    st_dynamic_stroke_inline dynamic_stroke;
    st_segmented_stroke_inline segmented_stroke;
    st_tap_hold_dual_inline tap_hold_dual;
    st_fire_key_inline fire_key;
    st_rebound_inline rebound;
    st_rappy_snappy_inline rappy_snappy;
} un_advanced_func_params;

typedef struct {
    EN_ADVANCED_MODE mode;
    un_advanced_func_params param;
} st_advanced_func;

/* 宏按键配置结构体 - 只保存宏标识符，模式和重复次数由Macro_SetData设置 */
typedef struct {
    uint8_t macro_id;     /* 宏标识符（通过macro_id查找对应的宏） */
} st_macro_key_cfg;

// 键值联合体
typedef union {
    // HID 键值
    uint8_t hid_value;
    // Consumer 键值
    uint8_t consumer_value[3]; //@st_consumer_report
    // 组合按键 修饰键+键值
    uint8_t modifier_value[2];
    // 自定义功能
    st_custom_func custom_func;
    // 宏按键配置（包含索引、模式、重复次数）
    st_macro_key_cfg macro_cfg;
} un_key_value;

// 通用键值配置：支持任意模式（HID/Consumer/Custom等）
typedef struct {
    EN_KEY_VAULE_MODE mode;      // 键值模式
    un_key_value value;          // 键值联合体
} st_key_value_config;

// 键值结构体（4层配置：base + FN1/2/3）
typedef struct {
    st_advanced_func advanced_mode;
    st_key_value_config base;    // 基础层（普通层）键值
    st_key_value_config fn1;     // FN1层键值配置
    st_key_value_config fn2;     // FN2层键值配置
    st_key_value_config fn3;     // FN3层键值配置
} st_key_value_params;

// 按键运行时状态（记录本次按下的实际键值配置）
typedef struct {
    st_key_value_config pressed_cfg;
    uint8_t is_pressed;     /* 是否当前处于按下状态 */
} st_key_runtime_state;

typedef enum {
    FN_MODE_BASE = 0,  // 主层输出，也是Fn0层
    FN_MODE_FN1 = 1,
    FN_MODE_FN2 = 2,
    FN_MODE_FN3 = 3,
} EN_FN_MODE;

#pragma pack(pop)

#endif
