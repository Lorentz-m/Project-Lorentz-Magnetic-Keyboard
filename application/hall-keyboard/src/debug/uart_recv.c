#include <stdio.h>

#include "debug/uart_recv.h"
#include "debug/uart_cmd_debug.h"

static void UartRecv_OnDemoHello(const char *line, uint16_t len)
{
    (void)line;
    (void)len;
    printf("[uart_recv] DEMO_HELLO ack\r\n");
}

static const st_uart_cmd_entry s_uart_recv_table[] = {
    {"DEMO_HELLO", UartRecv_OnDemoHello},
};

void UartRecv_Init(void)
{
    (void)UartCmdDebug_RegisterTable(
        s_uart_recv_table,
        (uint32_t)(sizeof(s_uart_recv_table) / sizeof(s_uart_recv_table[0])));
}
