#ifndef RGB_PLATFORM_H
#define RGB_PLATFORM_H

#include "led/led_display.h"

#ifndef UNUSED
/** 标记有意未使用的变量/参数 */
#define UNUSED(x) ((void)(x))
#endif

#ifndef __WEAK
/** 弱符号，板级可覆写 */
#define __WEAK __attribute__((weak))
#endif

/** 写单颗灯珠 RGB（经 led_display，含占空比缩放） */
#define led_set(index, r, g, b)   LED_Display_SetPixelRgb((index), (r), (g), (b))
/** 提交本帧到 WS2812 */
#define led_flush()               LED_Display_Commit()

#endif

