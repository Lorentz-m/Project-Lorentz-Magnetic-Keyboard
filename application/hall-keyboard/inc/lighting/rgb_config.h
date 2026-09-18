#ifndef RGB_CONFIG_H
#define RGB_CONFIG_H

#include "layout/layout.h"

#ifndef RGB_NUM
/** 串联灯珠数量 */
#define RGB_NUM                 (EN_LAYOUT_KEY_RGB_SLOT_MAX)
#endif

#ifndef TOTAL_KEY_NUM
/** 稠密 key_id 表最大键数 */
#define TOTAL_KEY_NUM           (EN_LAYOUT_KEY_HALL_SLOT_MAX)
#endif

#ifndef ANALOG_VALUE_MIN
#define ANALOG_VALUE_MIN        (0u)
#endif
#ifndef ANALOG_VALUE_MAX
#define ANALOG_VALUE_MAX        (1023u)
#endif
#ifndef ANALOG_VALUE_RANGE
#define ANALOG_VALUE_RANGE      (1024u)
#endif

/** 磁轴行程模拟量类型 */
typedef uint16_t AnalogValue;

/** 编译开关：各底座/per-key 模式是否编入 rgb_process */
#define RGB_BASE_MODE_USE_RAINBOW           1
#define RGB_BASE_MODE_USE_WAVE              1
#define RGB_MODE_USE_STRING                 1
#define RGB_MODE_USE_FADING_STRING          1
#define RGB_MODE_USE_DIAMOND_RIPPLE         1
#define RGB_MODE_USE_FADING_DIAMOND_RIPPLE  1
#define RGB_MODE_USE_BUBBLE                 1
#define RGB_MODE_USE_LINEAR                 0
#define RGB_MODE_USE_TRIGGER                1
#define RGB_MODE_USE_STATIC                 1
#define RGB_MODE_USE_CYCLE                  1
#define RGB_MODE_USE_JELLY                  0

#ifndef RGB_LEFT
/** 键面布局左边界（layout unit） */
#define RGB_LEFT                (-0.5f)
#endif
#ifndef RGB_TOP
#define RGB_TOP                 (-0.5f)
#endif
#ifndef RGB_RIGHT
#define RGB_RIGHT               (14.5f)
#endif
#ifndef RGB_BOTTOM
#define RGB_BOTTOM              (4.5f)
#endif

#ifndef M_PI
#define M_PI                    (3.14159265358979323846f)
#endif

#endif

