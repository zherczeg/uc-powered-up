/*
 *    uc-powered-up (micro/universal c implementation of powered up, you see powered up, ...)
 *
 *    Copyright Zoltan Herczeg (hzmester@freemail.hu). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this list of
 *      conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this list
 *      of conditions and the following disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER(S) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdint.h>
#include "commands.h"

/* Attribute protocol (ATT) is not part of the bluetooth library.
 * Instead of defining packed structures, uint8_t arrays are used. */

#define ATT_OP_FIND_INFO_REQ 0x04
#define ATT_OP_FIND_INFO_RESP 0x05

typedef struct {
	uint8_t opcode;
	uint8_t starting_handle[2];
	uint8_t ending_handle[2];
} att_op_find_info_req_t;

#define ATT_WRITE_REQ 0x12
#define ATT_WRITE_CMD 0x52
#define ATT_HANDLE_VALUE_NTF 0x1b

typedef struct {
	uint8_t opcode;
	uint8_t handle[2];
	uint8_t configuration[2];
} gatt_client_characteristic_configuration_t;

/* General context. */

typedef struct {
	int sock;
	uint8_t handle[2];
	uint8_t rsp_buf[64];
} ucpu_connection_t;

int ucpu_connect_to_hub(ucpu_connection_t *ucpu_connection);
int ucpu_att_send(ucpu_connection_t *ucpu_connection, void *req_buf, uint16_t req_buf_len);
int ucpu_att_receive(ucpu_connection_t *ucpu_connection);
int ucpu_send_command(ucpu_connection_t *ucpu_connection, hub_common_message_header_t *message, uint16_t message_len);

int ucpu_is_notification(ucpu_connection_t *ucpu_connection, int message_length);
/* The following ucpu_is_* functions can only be invoked if ucpu_is_notification returned non-zero */
int ucpu_is_attached_io_update(ucpu_connection_t *ucpu_connection, int message_length);
int ucpu_is_port_value_single(ucpu_connection_t *ucpu_connection, int message_length);

int ucpu_port_information_request(ucpu_connection_t *ucpu_connection, uint8_t port_id, uint8_t information_type);
int ucpu_port_input_format_setup(ucpu_connection_t *ucpu_connection, uint8_t port_id, uint8_t mode,
	uint32_t delta_interval, uint8_t notification_enabled);
int ucpu_virtual_port_connect(ucpu_connection_t *ucpu_connection, uint8_t port_id_a, uint8_t port_id_b);
int ucpu_virtual_port_disconnect(ucpu_connection_t *ucpu_connection, uint8_t port_id);

int ucpu_set_led_color(ucpu_connection_t *ucpu_connection, uint8_t port_id, uint8_t color_id);
int ucpu_set_led_rgb(ucpu_connection_t *ucpu_connection, uint8_t port_id, uint8_t r, uint8_t g, uint8_t b);
int ucpu_motor_start_speed(ucpu_connection_t *ucpu_connection, uint8_t port_id,
	int8_t speed, int8_t max_power, uint8_t use_profile);
int ucpu_motor_goto_absolute_position(ucpu_connection_t *ucpu_connection, uint8_t port_id,
	int32_t absolute_pos, int8_t speed, int8_t max_power, int8_t end_state, uint8_t use_profile);

#endif /* GLOBALS_H_ */
