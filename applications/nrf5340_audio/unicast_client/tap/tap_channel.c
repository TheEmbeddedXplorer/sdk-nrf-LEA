/*
 * File: tap_channel.c
 * Description: Definition of user tap command ZBus channel.
 */

 #include "tap_channel.h"

 // ZBus channel definition.
 ZBUS_CHAN_DEFINE(tap_cmd_chan,
     tap_cmd_msg_t,
     NULL,   // validator
     NULL,   // user data
     ZBUS_OBSERVERS_EMPTY,
     ZBUS_MSG_INIT(.cmd = "")
 );
 