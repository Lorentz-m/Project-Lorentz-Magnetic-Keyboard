/**
 * @file lighting.c
 * @brief 灯效核心：初始化、轮询出帧、子模块与事件串联
 */
#include <stdio.h>
#include "lighting/lighting.h"
#include "lighting/rgb_layout.h"
#include "lighting/rgb_keyboard_shim.h"
#include "lighting/lighting_event.h"
#include "lighting/lighting_mode.h"
#include "systick.h"
#include "soft_timer/soft_timer.h"

/** 上次 rgb_process 时间戳（毫秒），用于节流约 15ms */
static uint32_t s_last_poll_ms;
/** 上次灯效开关状态，toggle 切到 0 时强制熄灭一次 */
static uint8_t s_last_lighting_toggle = 1u;
/** 0: boot delay not elapsed, Poll keeps LEDs off */
static uint8_t s_boot_light_ready;

static void lighting_boot_on_delay_cb(soft_timer_id_t id, const void *ctx, uint8_t ctx_len)
{
    (void)id;
    (void)ctx;
    (void)ctx_len;
    s_boot_light_ready = 1u;
    printf("[Lighting] boot on delay done (%u ms)\r\n", (unsigned)LIGHTING_BOOT_ON_DELAY_MS);
}

void Lighting_Init(void)
{
    RgbLayout_Init();
    RgbKeyboardShim_Init();
    rgb_factory_reset();
    rgb_init();
    RgbLayout_FinalizeInverseMapping();
    s_last_poll_ms = 0u;
    s_boot_light_ready = 0u;

    Lighting_Mode_Init();
    Lighting_Speed_Init();
    Lighting_Color_Init();
    Lighting_Brightness_Init();

    if (!Lighting_EventInit()) {
        printf("[Lighting] Lighting_EventInit failed\r\n");
    }
    Lighting_Mode_SwitchTo(Lighting_Mode_GetCurrent());
    rgb_turn_off();
    s_last_lighting_toggle = 0u;
    if (soft_timer_start(LIGHTING_BOOT_ON_DELAY_MS, lighting_boot_on_delay_cb, NULL, 0,
                         EN_SOFT_TIMER_MODE_ONCE) == SOFT_TIMER_INVALID_ID) {
        printf("[Lighting] boot on delay timer start failed\r\n");
        s_boot_light_ready = 1u;
    }
}

void Lighting_Poll(void)
{
    uint32_t now;
    uint8_t toggle;

    if (s_boot_light_ready == 0u) {
        if (s_last_lighting_toggle != 0u) {
            rgb_turn_off();
            s_last_lighting_toggle = 0u;
        }
        return;
    }

    toggle = Lighting_Mode_GetToggle();
    if (toggle == 0u) {
        if (s_last_lighting_toggle != 0u) {
            rgb_turn_off();
            s_last_lighting_toggle = 0u;
        }
        return;
    }
    s_last_lighting_toggle = 1u;

    if (!rgb_needs_process()) {
        return;
    }
    now = systick_get_ms();
    RgbKeyboardShim_SetTick(now);
    if (s_last_poll_ms != 0u && (int32_t)(now - s_last_poll_ms) < 15) {
        return;
    }
    s_last_poll_ms = now;
    rgb_process();
}
