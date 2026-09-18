#ifndef KEYBOARD_CONFIG_H
#define KEYBOARD_CONFIG_H

#include <stdint.h>
#include "device/keyboard_matrix.h"
#include "device/keyboard_key_types.h"

#pragma pack(push, 1)

typedef enum {
    WINDOW_MODE_ON_WIN = 0,
    WINDOW_MODE_ON_MAC,
} EN_WINDOW_MAC_MODE;

/* USB轮询率枚举：对应bInterval值(1..4)，值=8kHz/2^n */
typedef enum {
    USB_REPORT_RATE_8K = 0,  /* 8kHz, bInterval=1 */
    USB_REPORT_RATE_4K,      /* 4kHz, bInterval=2 */
    USB_REPORT_RATE_2K,      /* 2kHz, bInterval=3 */
    USB_REPORT_RATE_1K,      /* 1kHz, bInterval=4 */
    USB_REPORT_RATE_MAX
} EN_USB_REPORT_RATE;

// 键盘参数结构体
typedef struct {
    uint32_t check_flag;
    st_key_performance_params performance[KEY_GROUP_INDEX_1][KEY_GROUP_INDEX_2][KEY_GROUP_INDEX_3];
    st_key_value_params key_value[KEY_GROUP_INDEX_1][KEY_GROUP_INDEX_2][KEY_GROUP_INDEX_3];
    EN_WINDOW_MAC_MODE win_mac_mode;
    uint8_t lighting_mode;       // 参考lighting模块的EN_LIGHTING_MODE枚举
    uint8_t lighting_color[3];   // RGB
    uint8_t lighting_brightness; // 0-255
    uint8_t lighting_speed[10];  // 大小需与EN_LIGHTING_MODE_MAX一致
    uint8_t lighting_toggle;     // 0-关闭，1-打开
    EN_USB_REPORT_RATE usb_report_rate;
    EN_FN_MODE fn_mode;
    uint8_t sleep_time_min; // 0-60分钟
    uint8_t bat_percent;    // 0-100%电池电量
    uint8_t usb_device_suspended; // usb设备是否挂起
    uint8_t open_calibrated; // 打开自动校准模式
} st_keyboard_params, st_keyboard_config;

// 统一参数索引枚举（按键参数 + 键盘全局设置参数）
typedef enum {
    /*========== 按键性能与键值参数区域 (用于 DeviceParams_SetKeyParams) ==========*/
    KEY_PARAMS_RT_MODE = 0,
    KEY_PARAMS_NORMAL_MODE_TRAVEL_UM,
    KEY_PARAMS_RT_PRESS_UM,
    KEY_PARAMS_RT_RELEASE_UM,
    KEY_PARAMS_TOP_DEAD_ZONE_UM,
    KEY_PARAMS_BOTTOM_DEAD_ZONE_UM,
    KEY_PARAMS_CALIBRATED_STATE,
    KEY_PARAMS_DIST_MONITOR_MODE,
    KEY_PARAMS_KEY_VALUE,
    KEY_PARAMS_FN1_VALUE,
    KEY_PARAMS_FN2_VALUE,
    KEY_PARAMS_FN3_VALUE,
    KEY_PARAMS_ADVANCED_MODE,
    KEY_PARAMS_MAX,
    /*========== 界限 ==========*/
    KEYBOARD_SETTING_START = KEY_PARAMS_MAX,
    /*========== 键盘全局设置参数区域 (用于 DeviceParams_SetKeyboardParam) ==========*/
    KEYBOARD_SETTING_WIN_MAC_MODE = KEYBOARD_SETTING_START,
    KEYBOARD_SETTING_LIGHTING_MODE,
    KEYBOARD_SETTING_LIGHTING_COLOR,
    KEYBOARD_SETTING_LIGHTING_BRIGHTNESS,
    KEYBOARD_SETTING_LIGHTING_SPEED,
    KEYBOARD_SETTING_LIGHTING_TOGGLE,
    KEYBOARD_SETTING_USB_REPORT_RATE,
    KEYBOARD_SETTING_MAX,
    /*========== 界限 ==========*/
    KEYBOARD_RUNTIME_START = KEYBOARD_SETTING_MAX,   
    /*========== 键盘运行时参数区域 (用于 DeviceParams_GetKeyboardParam) ==========*/
    KEYBOARD_RUNTIME_FN_MODE = KEYBOARD_SETTING_MAX,
    KEYBOARD_RUNTIME_SLEEP_TIME_MIN,
    KEYBOARD_RUNTIME_BAT_PERCENT,
    KEYBOARD_RUNTIME_USBD_SUSPENDED,
    KEYBOARD_RUNTIME_OPEN_CALIBRATED, // 打开自动校准模式
    KEYBOARD_RUNTIME_MAX,
    EN_PARAMS_MAX = KEYBOARD_RUNTIME_MAX,
} EN_PARAMS_INDEX, EN_KEY_PARAMS_INDEX, EN_KEYBOARD_SETTING_INDEX;

#pragma pack(pop)

#endif
