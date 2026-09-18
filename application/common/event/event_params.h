#ifndef EVENT_PARAMS_H_
#define EVENT_PARAMS_H_

#include <stdint.h>
#include <stdbool.h>
#include "event_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOD_KEY         0x20
#define MOD_LIGHT       0x50
#define MOD_CUSTOM_KEY  0x70
#define MOD_HALL        0x80

#define MAKE_EVENT(mod, sub) ((mod) | (sub))

enum {
    EVT_PHYSICAL_KEY_PRESS   = MAKE_EVENT(MOD_KEY, 0x01),
    EVT_PHYSICAL_KEY_RELEASE = MAKE_EVENT(MOD_KEY, 0x02),
    EVT_PHYSICAL_KEY_DIST_MONITOR = MAKE_EVENT(MOD_KEY, 0x05),
    EVT_KEY_LIGHTING_PRESS   = MAKE_EVENT(MOD_KEY, 0x06),
    EVT_KEY_LIGHTING_RELEASE = MAKE_EVENT(MOD_KEY, 0x07),
};

enum {
    EVT_CUSTOM_KEY_LIGHTING_MODE_SWITCH = MAKE_EVENT(MOD_CUSTOM_KEY, 0x01),
    EVT_CUSTOM_KEY_LIGHTING_COLOR_SWITCH = MAKE_EVENT(MOD_CUSTOM_KEY, 0x02),
    EVT_CUSTOM_KEY_LIGHTING_BRIGHT_ADD = MAKE_EVENT(MOD_CUSTOM_KEY, 0x03),
    EVT_CUSTOM_KEY_LIGHTING_BRIGHT_SUB = MAKE_EVENT(MOD_CUSTOM_KEY, 0x04),
    EVT_CUSTOM_KEY_LIGHTING_SPEED_ADD = MAKE_EVENT(MOD_CUSTOM_KEY, 0x05),
    EVT_CUSTOM_KEY_LIGHTING_SPEED_SUB = MAKE_EVENT(MOD_CUSTOM_KEY, 0x06),
    EVT_CUSTOM_KEY_LIGHTING_LAST_MODE = MAKE_EVENT(MOD_CUSTOM_KEY, 0x07),
    EVT_CUSTOM_KEY_LIGHTING_TOGGLE = MAKE_EVENT(MOD_CUSTOM_KEY, 0x08),
};

enum {
    EVT_LIGHTING_MODE_SETTING = MAKE_EVENT(MOD_LIGHT, 0x01),
    EVT_LIGHTING_COLOR_SETTING = MAKE_EVENT(MOD_LIGHT, 0x02),
    EVT_LIGHTING_BRIGHT_SETTING = MAKE_EVENT(MOD_LIGHT, 0x03),
    EVT_LIGHTING_OPEN_SETTING = MAKE_EVENT(MOD_LIGHT, 0x04),
};

enum {
    EVT_HALL_AUTO_CALIB_MODE_START = MAKE_EVENT(MOD_HALL, 0x02),
    EVT_HALL_AUTO_CALIB_MODE_STOP  = MAKE_EVENT(MOD_HALL, 0x03),
};

typedef struct {
    uint8_t adc_idx;
    uint8_t ch;
    uint8_t group;
    int32_t dist_um;
} st_physical_key_event_param;

typedef struct {
    union {
        uint8_t switch_mode;
        uint8_t color_rgb[3];
        uint8_t brightness;
        uint8_t is_open;
    } light_data;
    bool ok;
} st_lighting_set_event_param;

#ifdef __cplusplus
}
#endif

#endif /* EVENT_PARAMS_H_ */
