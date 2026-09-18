#ifndef UART_CMD_DEBUG_H
#define UART_CMD_DEBUG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UART_CMD_USER_MAX
#define UART_CMD_USER_MAX (16u)
#endif

typedef void (*pf_uart_cmd_handler_t)(const char *line, uint16_t len);

typedef struct {
    const char *prefix;
    pf_uart_cmd_handler_t handler;
} st_uart_cmd_entry;

void UartCmdDebug_Init(void);
void UartCmdDebug_DispatchLine(uint8_t *buf, uint16_t len);

int UartCmdDebug_Register(const char *prefix, pf_uart_cmd_handler_t handler);
int UartCmdDebug_RegisterTable(const st_uart_cmd_entry *table, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* UART_CMD_DEBUG_H */
