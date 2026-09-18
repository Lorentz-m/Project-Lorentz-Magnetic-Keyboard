/**
 * @file lighting_event.c
 * @brief 订阅物理键与自定义键灯效事件，转调 mode/color/brightness/speed/key
 */
#include <stdio.h>

#include "lighting/lighting_event.h"
#include "lighting/lighting_key.h"
#include "lighting/lighting_mode.h"
#include "lighting/lighting_color.h"
#include "lighting/lighting_brightness.h"
#include "lighting/lighting_speed.h"
#include "lighting/color.h"
#include "lighting/rgb.h"

#include "event/event_bus.h"
#include "event/event_params.h"

/**
 * @brief 将事件行程限制在灯效可用范围
 * @param dist_um 事件携带的行程（微米，有符号）
 * @return 钳位后的无符号行程
 */
static uint16_t lighting_clamp_dist_um(int32_t dist_um)
{
    if (dist_um <= 0) {
        return 0u;
    }
    if (dist_um >= (int32_t)LIGHTING_KEY_DIST_UM_MAX) {
        return (uint16_t)LIGHTING_KEY_DIST_UM_MAX;
    }
    return (uint16_t)dist_um;
}

/** @brief 物理键按下/抬起：转 coord+dist 调用 Lighting_SetKeyStateAtCoord */
static void Lighting_OnPhysicalKeyEvent(const Event *e)
{
    const st_physical_key_event_param *p; /**< 物理键事件载荷 */
    st_key_coord coord;                   /**< 转 layout 用的坐标 */

    if (e == NULL || e->param.ptr == NULL) {
        return;
    }
    p = (const st_physical_key_event_param *)e->param.ptr;

    coord.adc_value = p->adc_idx;
    coord.adc_channel = p->ch;
    coord.group = p->group;

    if (e->type == EVT_PHYSICAL_KEY_PRESS || e->type == EVT_KEY_LIGHTING_PRESS) {
        Lighting_SetKeyStateAtCoord(coord, lighting_clamp_dist_um(p->dist_um), 1);
    } else if (e->type == EVT_PHYSICAL_KEY_RELEASE || e->type == EVT_KEY_LIGHTING_RELEASE) {
        Lighting_SetKeyStateAtCoord(coord, lighting_clamp_dist_um(p->dist_um), 0);
    }
}

/** @brief 自定义键：下一灯效模式（忽略事件参数） */
static void Lighting_OnModeSwitch(const Event *e)
{
    (void)e;
    Lighting_Mode_SwitchNext();
}

/** @brief 自定义键：下一预设色 */
static void Lighting_OnColorSwitch(const Event *e)
{
    (void)e;
    Lighting_Color_SwitchNext();
}

/** @brief 自定义键：亮度加 */
static void Lighting_OnBrightAdd(const Event *e)
{
    (void)e;
    Lighting_Brightness_Add();
}

/** @brief 自定义键：亮度减 */
static void Lighting_OnBrightSub(const Event *e)
{
    (void)e;
    Lighting_Brightness_Sub();
}

/** @brief 自定义键：当前模式速度加 */
static void Lighting_OnSpeedAdd(const Event *e)
{
    (void)e;
    Lighting_Speed_Add();
}

/** @brief 自定义键：当前模式速度减 */
static void Lighting_OnSpeedSub(const Event *e)
{
    (void)e;
    Lighting_Speed_Sub();
}

/** @brief 自定义键：上一灯效模式 */
static void Lighting_OnLastMode(const Event *e)
{
    (void)e;
    Lighting_Mode_SwitchPrev();
}

/** @brief 自定义键：灯效总开关翻转 */
static void Lighting_OnToggle(const Event *e)
{
    (void)e;
    Lighting_Mode_OnToggleNotify();
}

static void Lighting_OnModeSetting(const Event *e)
{
    st_lighting_set_event_param *param;

    if (e == NULL || e->param.ptr == NULL) {
        return;
    }
    param = (st_lighting_set_event_param *)e->param.ptr;
    param->ok = false;

    EN_LIGHTING_MODE mode = (EN_LIGHTING_MODE)param->light_data.switch_mode;
    if (mode < EN_LIGHTING_MODE_MAX) {
        param->ok = Lighting_Mode_SwitchTo(mode);
    }
}

static void Lighting_OnColorSetting(const Event *e)
{
    st_lighting_set_event_param *param;
    if (e == NULL || e->param.ptr == NULL) {
        return;
    }
    param = (st_lighting_set_event_param *)e->param.ptr;
    param->ok = false;

    param->ok = Lighting_Color_SetBaseColor(param->light_data.color_rgb);
}

static void Lighting_OnBrightSetting(const Event *e)
{
    st_lighting_set_event_param *param;
    uint8_t brightness;

    if (e == NULL || e->param.ptr == NULL) {
        return;
    }
    param = (st_lighting_set_event_param *)e->param.ptr;
    param->ok = false;

    brightness = param->light_data.brightness;
    if (brightness > 100) {
        brightness = 100;
    }
    if (brightness > 0) {
        brightness = (uint8_t)((uint16_t)brightness * 255 / 100);
    }
    param->ok = Lighting_Brightness_Set(brightness);
}

static void Lighting_OnOpenSetting(const Event *e)
{
    st_lighting_set_event_param *param;
    if (e == NULL || e->param.ptr == NULL) {
        return;
    }
    param = (st_lighting_set_event_param *)e->param.ptr;
    param->ok = false;

    param->ok = Lighting_Mode_SetToggle(param->light_data.is_open);
}

static void Lighting_OnHallCalibAutoMode(const Event *e)
{
    if (e->type == EVT_HALL_AUTO_CALIB_MODE_START) {
        Lighting_Mode_OnHallCalibAutoModeSwitch(1);
    }
    else if (e->type == EVT_HALL_AUTO_CALIB_MODE_STOP) {
        Lighting_Mode_OnHallCalibAutoModeSwitch(0);
    }
}

/** 灯效事件订阅表；slot_id 由 event_bus_subscribe 回填 */
static st_common_event_subs_params s_event_entries[] = {
    {EVT_PHYSICAL_KEY_PRESS,               Lighting_OnPhysicalKeyEvent, 0xFF},
    {EVT_PHYSICAL_KEY_RELEASE,             Lighting_OnPhysicalKeyEvent, 0xFF},
    {EVT_KEY_LIGHTING_PRESS,               Lighting_OnPhysicalKeyEvent, 0xFF},
    {EVT_KEY_LIGHTING_RELEASE,             Lighting_OnPhysicalKeyEvent, 0xFF},
    {EVT_CUSTOM_KEY_LIGHTING_MODE_SWITCH,  Lighting_OnModeSwitch,       0xFF},
    {EVT_CUSTOM_KEY_LIGHTING_COLOR_SWITCH, Lighting_OnColorSwitch,      0xFF},
    {EVT_CUSTOM_KEY_LIGHTING_BRIGHT_ADD,   Lighting_OnBrightAdd,        0xFF},
    {EVT_CUSTOM_KEY_LIGHTING_BRIGHT_SUB,   Lighting_OnBrightSub,        0xFF},
    {EVT_CUSTOM_KEY_LIGHTING_SPEED_ADD,    Lighting_OnSpeedAdd,         0xFF},
    {EVT_CUSTOM_KEY_LIGHTING_SPEED_SUB,    Lighting_OnSpeedSub,         0xFF},
    {EVT_CUSTOM_KEY_LIGHTING_LAST_MODE,    Lighting_OnLastMode,         0xFF},
    {EVT_CUSTOM_KEY_LIGHTING_TOGGLE,       Lighting_OnToggle,           0xFF},
    {EVT_LIGHTING_MODE_SETTING,           Lighting_OnModeSetting,      0xFF},
    {EVT_LIGHTING_COLOR_SETTING,          Lighting_OnColorSetting,     0xFF},
    {EVT_LIGHTING_BRIGHT_SETTING,         Lighting_OnBrightSetting,    0xFF},
    {EVT_LIGHTING_OPEN_SETTING,           Lighting_OnOpenSetting,      0xFF},
    {EVT_HALL_AUTO_CALIB_MODE_START,      Lighting_OnHallCalibAutoMode, 0xFF},
    {EVT_HALL_AUTO_CALIB_MODE_STOP,       Lighting_OnHallCalibAutoMode, 0xFF},
};

/** 表项个数 */
#define LIGHTING_EVENT_COUNT (sizeof(s_event_entries) / sizeof(s_event_entries[0]))

bool Lighting_EventInit(void)
{
    uint8_t i;      /**< 遍历下标 */
    bool result;    /**< 是否全部订阅成功 */

    result = true;
    for (i = 0; i < LIGHTING_EVENT_COUNT; i++) {
        if (!event_bus_subscribe(s_event_entries[i].event_type,
                                 s_event_entries[i].handler,
                                 &s_event_entries[i].slot_id)) {
            printf("[Lighting_Event] subscribe failed, type=0x%02X\r\n",
                          s_event_entries[i].event_type);
            result = false;
        }
    }

    return result;
}

void Lighting_EventDeinit(void)
{
    uint8_t i;

    for (i = 0; i < LIGHTING_EVENT_COUNT; i++) {
        if (s_event_entries[i].slot_id != 0xFF) {
            event_bus_unsubscribe(s_event_entries[i].event_type,
                                  s_event_entries[i].slot_id);
            s_event_entries[i].slot_id = 0xFF;
        }
    }
}
