#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include <stdint.h>
#include <stdbool.h>

/* 初始化 */
void systick_init(void);

/* UART 就绪后打印 systick_init 读到的 RCC 频率（只读，不改 SystemCoreClock） */
void systick_print_clock_info(void);

/* 获取当前时间计数 */
uint64_t systick_get_us(void);  /* ms*1000, 1ms resolution */
uint64_t systick_get_ms(void);  /* 毫秒 */

/* 延时接口 */
void systick_delay_ms(uint32_t ms);

bool systick_counts_judge_over(uint32_t judge_ms, uint32_t target_ms);
void systick_system_reset(void);

#endif /* __SYSTICK_H__ */
