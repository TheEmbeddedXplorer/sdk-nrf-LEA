/*
 * File: tap_uart.c
 * Description: Implementation of UART handler to receive TAP commands.
 */

 #include <zephyr/kernel.h>
 #include <zephyr/devicetree.h>
 #include <zephyr/drivers/uart.h>
 #include <zephyr/device.h>
 #include <zephyr/logging/log.h>
 #include <zephyr/sys/printk.h>
 
 #include <string.h>
 
 #include "tap_channel.h"
 #include "tap_uart.h"
 
 LOG_MODULE_REGISTER(tap_uart, LOG_LEVEL_INF);
 
 /* Using uart0 node since uart2 could not be mapped to VCOM1 as it(VCOM1) is used by network core for logging. */
 #define TAP_UART_NODE DT_NODELABEL(uart0)
 
 #define BUF_SIZE TAP_CMD_MAX_LEN
 
 // Receive timeout
 #define RECEIVE_TIMEOUT 100
 
 static const struct device *tap_uart_dev = DEVICE_DT_GET(TAP_UART_NODE);
 
 static uint8_t intro_buf[] =   {"--- CUSTOMER TAP COMMAND WINDOW ---\r\n"
     "Supported cmds: test_led_<on/off>\r\n"};
 
 /* Define the receive buffer */
 static uint8_t rx_buf[BUF_SIZE] = {0};
 static uint8_t cmd_buf[BUF_SIZE];
 static size_t cmd_len = 0;
 
 /* Declare a K-Work instance. */
 static struct k_work dispatch_tap_cmd_work;
 
 static void dispatch_tap_cmd_fn(struct k_work *work)
 {
     printk("TAP Command received: %s\n", (char *)cmd_buf);
 
     tap_cmd_msg_t msg;
     // size_t copyLen = (cmd_len > TAP_CMD_MAX_LEN-1) ? TAP_CMD_MAX_LEN-1 : cmd_len;
     strncpy(msg.cmd, (char *)cmd_buf, cmd_len);
     msg.cmd[cmd_len] = '\0';
     zbus_chan_pub(&tap_cmd_chan, &msg, K_NO_WAIT);
     cmd_len = 0;
     memset(cmd_buf, 0, sizeof(cmd_buf));
 }
 
 /* Define the callback function for UART */
 static void uart_device_cb(const struct device *dev, struct uart_event *evt, void *user_data)
 {
     switch (evt->type)
     {
         case UART_RX_RDY:
             /* Traverse through the received buffer, keep saving data/bytes to cmd_buf until enter is pressed or buffer overflows.*/
             for (size_t i = evt->data.rx.offset; i < (evt->data.rx.offset + evt->data.rx.len); i++)
             {
                 uint8_t c = evt->data.rx.buf[i];
 
                 // Echo back (optional)
                 uart_tx(dev, &c, 1, SYS_FOREVER_MS);
 
                 if (c == '\r' || c == '\n')
                 {
                     if (cmd_len > 0)
                     {
                         // Submit work.
                         // printk("Got full TAP Command.\n");
                         k_work_submit(&dispatch_tap_cmd_work);
                     }
                 }
                 else if (cmd_len < BUF_SIZE - 1)
                 {
                     cmd_buf[cmd_len++] = c;
                 }
                 else
                 {
                     printk("Buffer full, resetting data...\n");
                     cmd_len = 0;
                     memset(cmd_buf, 0, sizeof(cmd_buf));
                 }
             }
             break;
         case UART_RX_DISABLED:
             uart_rx_enable(dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
             break;
 
         default:
             // Unhandled.
             break;
         }
 }
 
 void tap_uart_init(void)
 {
     if (!device_is_ready(tap_uart_dev)) {
         printk("TAP UART not ready.\n");
         return;
     }
 
     /* Init k_work handler. */
     k_work_init(&dispatch_tap_cmd_work, dispatch_tap_cmd_fn);
 
     /* Register the UART callback function */
     int ret = uart_callback_set(tap_uart_dev, uart_device_cb, NULL);
     if (ret < 0)
     {
         printk("TAP UART callback set err (%d).\n", ret);
         return;
     }
 
     /* Send the data over UART by calling uart_tx() */
     ret = uart_tx(tap_uart_dev, intro_buf, sizeof(intro_buf), SYS_FOREVER_US);
     if (ret < 0)
     {
         printk("TAP UART TX err (%d).\n", ret);
         return;
     }
 
     /* Start receiving by calling uart_rx_enable() and pass it the address of the
      * receive  buffer */
     ret = uart_rx_enable(tap_uart_dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
     if (ret < 0)
     {
         printk("TAP UART RX enable err (%d).\n", ret);
         return;
     }
 
     printk("Async TAP UART ready !!! Type your TAP command and press Enter:\n");
 }
 