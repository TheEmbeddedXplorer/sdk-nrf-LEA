/*
 * File: tap_channel.h
 * Description: Declaration of user tap command ZBus channel and tap message type.
 */

 #ifndef __TAP_CHANNEL_H__
 #define __TAP_CHANNEL_H__
  
  #include <zephyr/kernel.h>
  #include <zephyr/device.h>
  #include <zephyr/drivers/uart.h>
 
 #include <zephyr/zbus/zbus.h>
 
 #define TAP_CMD_MAX_LEN 128
 
 typedef struct {
     char cmd[TAP_CMD_MAX_LEN];
 } tap_cmd_msg_t;
 
 // Declare a zbus channel for tap commands.
 ZBUS_CHAN_DECLARE(tap_cmd_chan);
 
 #endif  /* __TAP_CHANNEL_H__ */
 