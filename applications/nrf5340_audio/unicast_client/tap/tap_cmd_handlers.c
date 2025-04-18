/*
 * File: tap_cmd_handlers.c
 * Description: Implementation of tap_command handlers.
 */

 #include <zephyr/kernel.h>
 #include <zephyr/zbus/zbus.h>
 #include <zephyr/logging/log.h>
 
 #include "tap_cmd_handlers.h"
 
 #include "led.h"
 
 /* Maximum tap commands registries. */
 #define MAX_REGISTRY_COUNT  15
 #define MAX_TAP_ARGV    24
 
 LOG_MODULE_REGISTER(tap_cmd_hand, LOG_LEVEL_INF);
 
 /* Tap Command Registration Table. */
 typedef struct
 {
     tap_cmd_registry_t theRegistry[MAX_REGISTRY_COUNT];
     uint8_t registerCount;
 } tapCommandTable_t;
 
 static tapCommandTable_t tapCmdTable;
 
 /* Module function declarations. */
 static void tap_cmd_listener_thread(const struct zbus_channel *chan);
 static void process_tap_command(const tap_cmd_msg_t *msg);
 
 /* Function Definitions. */
 static int parseTapArgs(char* strData, char *argv[MAX_TAP_ARGV])
 {
     int arg_count = 0;
 
     char *token = strtok(strData, ",");
 
     while (token != NULL && arg_count < MAX_TAP_ARGV)
     {
         argv[arg_count++] = token;
         token = strtok(NULL, ",");
     }
 
     return arg_count;
 }
 
 #if 0
 static void process_tap_command(tap_cmd_msg_t *msg)
 {
     if (!msg)
     {
         LOG_ERR("NULL msg...!!!\n");
         return;
     }
 
     if (strcmp(msg->cmd, "test_led_on") == 0)
     {
         LOG_INF("LED RGB ON\n");
         led_on(LED_APP_RGB, LED_COLOR_YELLOW);
     } 
     else if (strcmp(msg->cmd, "test_led_off") == 0)
     {
         LOG_INF("LED RGB OFF\n");
         led_off(LED_APP_RGB);
     }
     else
     {
         LOG_WRN("TAP: Unknown command: %s\n", msg->cmd);
     }
 }
 #else
 
 static void process_tap_command(const tap_cmd_msg_t *msg)
 {
     if (!msg)
     {
         LOG_ERR("NULL msg...!!!\n");
         return;
     }
 
     // Parse TAP Args.
     char *argv[MAX_TAP_ARGV];
 
     // Allocating some space for copied msg on stack. Update it to allocate dynamically, if required. 
     char copiedMsg[TAP_CMD_MAX_LEN];
 
     // Make a copy of original msg.
     memcpy(copiedMsg, msg->cmd, sizeof(msg->cmd));
 
     int argc = parseTapArgs(copiedMsg, argv);
     LOG_INF("DEBUG_LOG: Parsed args_count (%d).", argc);
 
     // Invoke tap callback.
     if (argc > 0)
     {
         // Print all the registered commands (help).
         if ((argc == 1) && (strcmp(argv[0], "help") == 0))
         {
             if (tapCmdTable.registerCount > 0)
             {
                 printk("\n-----------\n");
                 for (uint8_t idx = 0; idx < tapCmdTable.registerCount && idx < MAX_REGISTRY_COUNT; idx++)
                 {
                     printk("%s\n", tapCmdTable.theRegistry[idx].message.cmd);
                 }
                 printk("\n-----------\n");
             }
             else
             {
                 printk("No TAP cmd registered.\n");
             }
             return;
         }
 
         // Look for empty slot in tap command table.
         for (uint8_t idx = 0; idx < MAX_REGISTRY_COUNT; idx++)
         {
             // Command matched !!
             if (strcmp(argv[0], tapCmdTable.theRegistry[idx].message.cmd) == 0)
             {
                 if (tapCmdTable.theRegistry[idx].handler)
                 {
                     LOG_INF("Notifying tap handler.");
                     tapCmdTable.theRegistry[idx].handler(argc-1, &argv[1]);
                     break;
                 }
             }
             // Continue checking for other commands.
         }
     }
     else
     {
         LOG_WRN("NOT ENOUGH ARGS...\n");
         return;
     }
 }
 #endif
 
 // Zbus channel listener thread for any tap command.
 static void tap_cmd_listener_thread(const struct zbus_channel *chan)
 {
     const tap_cmd_msg_t *msg;
 
     LOG_WRN("DEBUG_LOG: HIT TAP Listner\n");
     msg = zbus_chan_const_msg(chan);
     process_tap_command(msg);
 }
 
 ZBUS_LISTENER_DEFINE(tap_cmd_listener, tap_cmd_listener_thread);
 
 
 /* API to register list of TAP command and its handler. */
 void register_tap_command(tap_cmd_registry_t *entry)
 {
     // Check for null entry.
     if (!entry)
     {
         LOG_ERR("Entry is NULL.");
         return;
     }
 
     // Check for invalid entry.
     if (!entry->handler || (strcmp(entry->message.cmd, "") == 0))
     {
         LOG_ERR("Entry is INVALID, Rejected.");
         return;
     }
 
     // Check if all slots are full.
     if (tapCmdTable.registerCount >= MAX_REGISTRY_COUNT)
     {
         LOG_ERR("No space left to register new TAP cmds.");
         return;
     }
 
     // Look for empty slot in tap command table.
     for (uint8_t idx = 0; idx < MAX_REGISTRY_COUNT; idx++)
     {
         // Check for empty handler.
         if (!tapCmdTable.theRegistry[idx].handler)
         {
             memcpy(&(tapCmdTable.theRegistry[idx]), entry, sizeof(tap_cmd_registry_t));
             tapCmdTable.registerCount++;
             LOG_INF("TAP Registered.");
             return;
         }
 
         // Continue checking for empty slots.
     }
 
     // We should never hit this line.
     LOG_WRN("NO EMPTY SLOTS FOUND. regCount (%d).", tapCmdTable.registerCount);
 }
 
 /* Initialize TAP command handlers, add tap command zbus channel observes/listener. */
 void tap_cmd_handlers_init(void)
 {
     LOG_INF("ENTER tap_cmd_handlers_init()");
 
     int ret = zbus_chan_add_obs(&tap_cmd_chan, &tap_cmd_listener, K_MSEC(200));
     if (ret) {
         LOG_ERR("Failed to add tap_cmd chan listener");
         return;
     }
     memset(tapCmdTable.theRegistry, 0, sizeof(tapCmdTable));
     tapCmdTable.registerCount = 0;
 
     LOG_WRN("tap_cmd chan listener added !!!");
 }
 