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

#ifndef DEVICES_H_
#define DEVICES_H_

#include <stdint.h>

/* LEGO Generic Attribute Protocol (GATT).
 * https://lego.github.io/lego-ble-wireless-protocol-docs/index.html
 * Note: all field types must be uint8_t */

typedef struct {
	uint8_t opcode; /* auto-filled */
	uint8_t handle[2]; /* auto-filled */
	uint8_t length; /* auto-filled */
	uint8_t hub_id; /* auto-filled */
	uint8_t message_type;
} hub_common_message_header_t;

#define HUB_ATTACHED_IO 0x04

#define HUB_ATTACHED_IO_DETACHED 0x00
#define HUB_ATTACHED_IO_ATTACHED 0x01
#define HUB_ATTACHED_IO_ATTACHED_VIRTUAL 0x02

typedef struct {
	hub_common_message_header_t common_message_header;
	uint8_t port_id;
	uint8_t event;
} hub_attached_io_t;

typedef struct {
	hub_attached_io_t attached_io;
	uint8_t io_type_id[2];
	uint8_t hardware_revision[4];
	uint8_t software_revision[4];
} hub_attached_io_attached_t;

typedef struct {
	hub_attached_io_t attached_io;
	uint8_t io_type_id[2];
	uint8_t port_id_a;
	uint8_t port_id_b;
} hub_attached_io_attached_virtual_t;

#define HUB_PORT_INFORMATION_REQUEST 0x21

#define HUB_PORT_INFORMATION_PORT_VALUE 0x00
#define HUB_PORT_INFORMATION_MODE_INFO 0x01
#define HUB_PORT_INFORMATION_POSSIBLE_MODE_COMBINATIONS 0x02

typedef struct {
	hub_common_message_header_t common_message_header;
	uint8_t port_id;
	uint8_t information_type;
} hub_port_information_request_t;

#define HUB_PORT_INPUT_FORMAT_SETUP 0x41

typedef struct {
	hub_common_message_header_t common_message_header;
	uint8_t port_id;
	uint8_t mode;
	uint8_t delta_interval[4];
	uint8_t notification_enabled;
} hub_port_input_format_setup_t;

#define HUB_PORT_VALUE_SINGLE 0x45

typedef struct {
	hub_common_message_header_t common_message_header;
	uint8_t port_id;
} hub_port_value_single_t;

#define HUB_VIRTUAL_PORT_SETUP 0x61

typedef struct {
	hub_common_message_header_t common_message_header;
	uint8_t sub_command;
	uint8_t port_id_a;
	uint8_t port_id_b;
} hub_virtual_port_connect_t;

typedef struct {
	hub_common_message_header_t common_message_header;
	uint8_t sub_command;
	uint8_t port_id;
} hub_virtual_port_disconnect_t;

/* Port output command. */

#define PORT_OUTPUT_COMMAND 0x81
#define WRITE_DIRECT_MODE_DATA 0x51

typedef struct {
	hub_common_message_header_t common_message_header;
	uint8_t port_id;
	uint8_t startup_and_complete;
	uint8_t sub_command;
} hub_port_output_command_t;

/* The built-in led uses the first internal port ID */
#define HUB_BUILT_IN_LED_PORT_ID 50

typedef struct {
	hub_port_output_command_t port_output_command;
	uint8_t mode;
	uint8_t color_id;
} hub_led_color_t;

typedef struct {
	hub_port_output_command_t port_output_command;
	uint8_t mode;
	uint8_t rgb[3];
} hub_led_rgb_t;

#define HUB_MOTOR_START_SPEED 0x07

typedef struct {
	hub_port_output_command_t port_output_command;
	int8_t speed;
	int8_t max_power;
	uint8_t use_profile;
} hub_motor_start_speed_t;

#define HUB_MOTOR_GOTO_ABSOLUTE_POSITION 0x0d

typedef struct {
	hub_port_output_command_t port_output_command;
	uint8_t absolute_pos[4];
	int8_t speed;
	int8_t max_power;
	int8_t end_state;
	uint8_t use_profile;
} hub_motor_goto_absolute_position_t;

#endif /* DEVICES_H_ */
