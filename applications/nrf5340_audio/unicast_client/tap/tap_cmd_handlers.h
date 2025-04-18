/*
 * File: tap_cmd_handlers.h
 * Description: Subscribe to tap command ZBus channel, implementation of tap_command message handlers.
 */

 #ifndef __TAP_CMD_HANDLERS_H__
 #define __TAP_CMD_HANDLERS_H__
 
 #include "tap_channel.h"
 
 /* Function pointer for TAP command handler. */
 typedef void (*tapCommandHandler_t)(int argc, char *argv[]);
 
 typedef struct
 {
     tap_cmd_msg_t message;
     tapCommandHandler_t handler;
 } tap_cmd_registry_t;
 
 // Initialize and start tap command zbus channel subscriber thread.
 void tap_cmd_handlers_init(void);
 
 // API to register list of TAP command and its handler.
 void register_tap_command(tap_cmd_registry_t *entry);
 
 #endif /* __TAP_CMD_HANDLERS_H__ */
 