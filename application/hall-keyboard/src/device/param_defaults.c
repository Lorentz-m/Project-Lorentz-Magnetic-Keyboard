#include <stddef.h>
#include <string.h>

#include "device/param_defaults.h"
#include "layout/layout.h"

#define KB_CFG_STATIC_ASSERT(cond, name) typedef char name[(cond) ? 1 : -1]

KB_CFG_STATIC_ASSERT(offsetof(st_keyboard_params, check_flag) == 0, kb_cfg_off_check_flag);
KB_CFG_STATIC_ASSERT(offsetof(st_keyboard_params, performance) == 4, kb_cfg_off_performance);
KB_CFG_STATIC_ASSERT(offsetof(st_key_value_params, advanced_mode) == 0, kb_cfg_off_adv);
KB_CFG_STATIC_ASSERT(
    offsetof(st_keyboard_params, key_value)
        == (4u + sizeof(((const st_keyboard_params *)0)->performance)),
    kb_cfg_off_key_value);
KB_CFG_STATIC_ASSERT(
    offsetof(st_keyboard_params, lighting_mode)
        == (offsetof(st_keyboard_params, win_mac_mode) + sizeof(EN_WINDOW_MAC_MODE)),
    kb_cfg_off_lighting_mode);

#define KEY_PARAMS_INIT(data, i1, i2, i3) do { \
    data.performance[i1][i2][i3].rt_mode = HALL_KEY_RT_MODE_DEFAULT; \
    data.performance[i1][i2][i3].normal_mode_travel_um = HALL_KEY_NORMAL_TRAVEL_UM_DEFAULT; \
    data.performance[i1][i2][i3].rt_press_um = (uint16_t)((float)HALL_KEY_PRESS_THRESHOLD_MM * 1000.0f); \
    data.performance[i1][i2][i3].rt_release_um = (uint16_t)((float)HALL_KEY_RELEASE_THRESHOLD_MM * 1000.0f); \
    data.performance[i1][i2][i3].top_dead_zone_um = (uint16_t)((float)HALL_KEY_IDLE_DEAD_ZONE_MM_DEFAULT * 1000.0f); \
    data.performance[i1][i2][i3].bottom_dead_zone_um = (uint16_t)((float)HALL_KEY_BOTTOM_DEAD_ZONE_MM_DEFAULT * 1000.0f); \
    data.key_value[i1][i2][i3].base.mode = KEY_VAULE_MODE_NORMAL; \
    data.key_value[i1][i2][i3].base.value.hid_value = KeyboardLayout_GetKeyDefaultHidByCoord((st_key_coord){i1, i2, i3}); \
    data.key_value[i1][i2][i3].fn1.mode = KEY_VAULE_MODE_NORMAL; \
    data.key_value[i1][i2][i3].fn1.value.hid_value = 0; \
    data.key_value[i1][i2][i3].fn2.mode = KEY_VAULE_MODE_NORMAL; \
    data.key_value[i1][i2][i3].fn2.value.hid_value = 0; \
    data.key_value[i1][i2][i3].fn3.mode = KEY_VAULE_MODE_NORMAL; \
    data.key_value[i1][i2][i3].fn3.value.hid_value = 0; \
} while (0)

uint32_t DeviceParams_GetDataCheckFlag(void)
{
    return (uint32_t)KEYBOARD_DATA_CHECK_FLAG;
}

void DeviceParams_FillFactoryDefaults(st_keyboard_params *out)
{
    uint8_t i1;
    uint8_t i2;
    uint8_t i3;
    uint8_t lighting_color[3] = LIGHTING_COLOR_DEFAULT;

    if (out == NULL) {
        return;
    }

    (void)memset(out, 0, sizeof(*out));
    for (i1 = 0u; i1 < (uint8_t)KEY_GROUP_INDEX_1; i1++) {
        for (i2 = 0u; i2 < (uint8_t)KEY_GROUP_INDEX_2; i2++) {
            for (i3 = 0u; i3 < (uint8_t)KEY_GROUP_INDEX_3; i3++) {
                KEY_PARAMS_INIT((*out), i1, i2, i3);
                if (out->key_value[i1][i2][i3].base.value.hid_value == Keyboard_FN1) {
                    out->key_value[i1][i2][i3].base.mode = KEY_VAULE_MODE_CUSTOM;
                    out->key_value[i1][i2][i3].base.value.custom_func.mode = CUSTOM_FUNC_ON_FN1; 
                    out->key_value[i1][i2][i3].base.value.custom_func.param.u8_value = 1;
                }
            }
        }
    }
    out->check_flag = DeviceParams_GetDataCheckFlag();
    out->lighting_toggle = LIGHTING_TOGGLE_DEFAULT;
    out->lighting_mode = LIGHTING_MODE_DEFAULT;
    (void)memcpy(out->lighting_color, lighting_color, sizeof(lighting_color));
    out->lighting_brightness = BRIGHTNESS_DEFAULT;
    out->usb_report_rate = USB_REPORT_RATE_DEFAULT;
    out->sleep_time_min = SLEEP_TIME_MIN_DEFAULT;
}
