/**
 * Compile-time defaults for device parameters (ROM / init).
 * Includes device/device_params.h so macros can use EN_HALL_KEY_RT_MODE enumerators.
 * Other modules include this header instead of duplicating default #define.
 */
#ifndef PARAM_DEFAULTS_H
#define PARAM_DEFAULTS_H

#include "device/device_params.h"
#include "device/keyboard_hid_usage.h"

#define KEYBOARD_DATA_CHECK_FLAG 0x11000000
/* 以下默认值，需要调用DeviceParams_FactoryReset或者更改KEYBOARD_DATA_CHECK_FLAG才能生效 */

// 正常模式行程um默认值
#define HALL_KEY_NORMAL_TRAVEL_UM_DEFAULT (2000u)
// 运行模式默认值
#define HALL_KEY_RT_MODE_DEFAULT HALL_KEY_RT_ON
// 运行模式按下RT精度默认值
#define HALL_KEY_PRESS_THRESHOLD_MM (0.1f)
// 运行模式释放RT精度默认值
#define HALL_KEY_RELEASE_THRESHOLD_MM HALL_KEY_PRESS_THRESHOLD_MM
#define HALL_KEY_IDLE_DEAD_ZONE_MM_DEFAULT (0.0f)
#define HALL_KEY_BOTTOM_DEAD_ZONE_MM_DEFAULT HALL_KEY_IDLE_DEAD_ZONE_MM_DEFAULT
#define USB_REPORT_RATE_DEFAULT USB_REPORT_RATE_8K
#define SLEEP_TIME_MIN_DEFAULT 3
#define BRIGHTNESS_DEFAULT 200
#define LIGHTING_MODE_DEFAULT 0u
#define LIGHTING_TOGGLE_DEFAULT 0u
#define LIGHTING_COLOR_DEFAULT {255, 0, 255}

#define LINE1_KEY1_DEFAULT   Keyboard_BoLangXian
#define LINE1_KEY2_DEFAULT   Keyboard_1
#define LINE1_KEY3_DEFAULT   Keyboard_2
#define LINE1_KEY4_DEFAULT   Keyboard_3
#define LINE1_KEY5_DEFAULT   Keyboard_4
#define LINE1_KEY6_DEFAULT   Keyboard_5
#define LINE1_KEY7_DEFAULT   Keyboard_6
#define LINE1_KEY8_DEFAULT   Keyboard_7
#define LINE1_KEY9_DEFAULT   Keyboard_8
#define LINE1_KEY10_DEFAULT  Keyboard_9
#define LINE1_KEY11_DEFAULT  Keyboard_0
#define LINE1_KEY12_DEFAULT  Keyboard_JianHao
#define LINE1_KEY13_DEFAULT  Keyboard_DengHao
#define LINE1_KEY14_DEFAULT  Keyboard_Backspace

#define LINE2_KEY1_DEFAULT   Keyboard_Tab
#define LINE2_KEY2_DEFAULT   Keyboard_q
#define LINE2_KEY3_DEFAULT   Keyboard_w
#define LINE2_KEY4_DEFAULT   Keyboard_e
#define LINE2_KEY5_DEFAULT   Keyboard_r
#define LINE2_KEY6_DEFAULT   Keyboard_t
#define LINE2_KEY7_DEFAULT   Keyboard_y
#define LINE2_KEY8_DEFAULT   Keyboard_u
#define LINE2_KEY9_DEFAULT   Keyboard_i
#define LINE2_KEY10_DEFAULT  Keyboard_o
#define LINE2_KEY11_DEFAULT  Keyboard_p
#define LINE2_KEY12_DEFAULT  Keyboard_ZuoZhongKuoHao
#define LINE2_KEY13_DEFAULT  Keyboard_YouZhongKuoHao
#define LINE2_KEY14_DEFAULT  Keyboard_FanXieGang

#define LINE3_KEY1_DEFAULT   Keyboard_CapsLock
#define LINE3_KEY2_DEFAULT   Keyboard_a
#define LINE3_KEY3_DEFAULT   Keyboard_s
#define LINE3_KEY4_DEFAULT   Keyboard_d
#define LINE3_KEY5_DEFAULT   Keyboard_f
#define LINE3_KEY6_DEFAULT   Keyboard_g
#define LINE3_KEY7_DEFAULT   Keyboard_h
#define LINE3_KEY8_DEFAULT   Keyboard_j
#define LINE3_KEY9_DEFAULT   Keyboard_k
#define LINE3_KEY10_DEFAULT  Keyboard_l
#define LINE3_KEY11_DEFAULT  Keyboard_FenHao
#define LINE3_KEY12_DEFAULT  Keyboard_DanYinHao
#define LINE3_KEY13_DEFAULT  Keyboard_ENTER

#define LINE4_KEY1_DEFAULT   Keyboard_LeftShift
#define LINE4_KEY2_DEFAULT   Keyboard_z
#define LINE4_KEY3_DEFAULT   Keyboard_x
#define LINE4_KEY4_DEFAULT   Keyboard_c
#define LINE4_KEY5_DEFAULT   Keyboard_v
#define LINE4_KEY6_DEFAULT   Keyboard_b
#define LINE4_KEY7_DEFAULT   Keyboard_n
#define LINE4_KEY8_DEFAULT   Keyboard_m
#define LINE4_KEY9_DEFAULT   Keyboard_Douhao
#define LINE4_KEY10_DEFAULT  Keyboard_JuHao
#define LINE4_KEY11_DEFAULT  Keyboard_XieGang_WenHao
#define LINE4_KEY12_DEFAULT  Keyboard_RightShift
#define LINE4_KEY13_DEFAULT  Keyboard_UpArrow
#define LINE4_KEY14_DEFAULT  Keyboard_Delete

#define LINE5_KEY1_DEFAULT   Keyboard_LeftControl
#define LINE5_KEY2_DEFAULT   Keyboard_LeftWindows
#define LINE5_KEY3_DEFAULT   Keyboard_LeftAlt
#define LINE5_KEY4_DEFAULT   Keyboard_KongGe
#define LINE5_KEY5_DEFAULT   Keyboard_RightAlt
#define LINE5_KEY6_DEFAULT   Keyboard_FN1
#define LINE5_KEY7_DEFAULT   Keyboard_LeftArrow
#define LINE5_KEY8_DEFAULT   Keyboard_DownArrow
#define LINE5_KEY9_DEFAULT   Keyboard_RightArrow

#ifdef __cplusplus
extern "C" {
#endif

uint32_t DeviceParams_GetDataCheckFlag(void);
void DeviceParams_FillFactoryDefaults(st_keyboard_params *out);

#ifdef __cplusplus
}
#endif

#endif /* PARAM_DEFAULTS_H */
