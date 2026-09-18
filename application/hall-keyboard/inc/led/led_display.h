#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * WS2812 逐颗白点跑马自检（LED_Display_VerifyPoll）。
 * 0：关闭（默认，与 lighting 正式灯效并存）；
 * 1：每 50ms 单颗白灯沿串联顺序移动，用于验线序/驱动。
 */
#ifndef LED_DISPLAY_VERIFY_CHASE
#define LED_DISPLAY_VERIFY_CHASE   0
#endif

/** @brief 初始化 GRB 缓冲与 WS2812 硬件（不 commit） */
void LED_Display_Iint(void);

/**
 * @brief 硬件验证轮询（需主循环调用；仅 LED_DISPLAY_VERIFY_CHASE=1 时有动作）
 * @param now_ms 当前毫秒 tick
 */
void LED_Display_VerifyPoll(uint32_t now_ms);

/**
 * @brief 写单颗灯珠 RGB 到帧缓冲（内部 GRB 顺序，含占空比缩放）
 * @param index 灯珠 slot，0 ~ WS2812_PIXEL_COUNT-1
 */
void LED_Display_SetPixelRgb(uint32_t index, uint8_t r, uint8_t g, uint8_t b);

/** @brief 全帧填充同一 RGB（未 commit） */
void LED_Display_FillRgb(uint8_t r, uint8_t g, uint8_t b);

/** @brief 帧缓冲清零（未 commit） */
void LED_Display_Clear(void);

/** @brief 将当前帧缓冲送出 WS2812（开 LED_EN + SendGrbPixels） */
void LED_Display_Commit(void);

/** @brief 测试：slot0 红、slot1 绿、slot2 蓝等，commit 一次 */
void LED_Display_TestRedGreen(void);

/** @brief 测试：前 14 颗按 R/G/B 循环点亮，commit 一次 */
void LED_Display_Test1(void);

#ifdef __cplusplus
}
#endif

#endif
