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

#include "globals.h"

/* Supports both signed and unsigned values. */
#define UCPU_SET_32BIT_VALUE(target, value) \
	do { \
		(target)[0] = (uint8_t)(value); \
		(target)[1] = (uint8_t)((value) >> 8); \
		(target)[2] = (uint8_t)((value) >> 16); \
		(target)[3] = (uint8_t)((value) >> 24); \
	} while (0)

#define UCPU_INIT_PORT_OUTPUT(message, port_id_, startup_and_complete_, sub_command_) \
	do { \
		(message).port_output_command.common_message_header.message_type = PORT_OUTPUT_COMMAND; \
		(message).port_output_command.port_id = (port_id_); \
		(message).port_output_command.startup_and_complete = (startup_and_complete_); \
		(message).port_output_command.sub_command = (sub_command_); \
	} while (0)

int ucpu_is_notification(ucpu_connection_t *ucpu_connection, int message_length)
{
	hub_common_message_header_t *common_message_header;

	if (message_length < sizeof(hub_common_message_header_t)) {
		return 0;
	}

	common_message_header = (hub_common_message_header_t*)ucpu_connection->rsp_buf;

	return (common_message_header->opcode == ATT_HANDLE_VALUE_NTF
		&& common_message_header->handle[0] == ucpu_connection->handle[0]
		&& common_message_header->handle[1] == ucpu_connection->handle[1]
		&& common_message_header->length == message_length - 3
		&& common_message_header->hub_id == 0);
}

int ucpu_is_attached_io_update(ucpu_connection_t *ucpu_connection, int message_length)
{
	hub_attached_io_t *attached_io;
	int expected_length;

	if (message_length < sizeof(hub_attached_io_t)) {
		return -1;
	}

	attached_io = (hub_attached_io_t*)ucpu_connection->rsp_buf;

	if (attached_io->common_message_header.message_type != HUB_ATTACHED_IO
			|| attached_io->event > HUB_ATTACHED_IO_ATTACHED_VIRTUAL) {
		return -1;
	}

	switch (attached_io->event) {
	case HUB_ATTACHED_IO_DETACHED:
		expected_length = sizeof(hub_attached_io_t);
		break;
	case HUB_ATTACHED_IO_ATTACHED:
		expected_length = sizeof(hub_attached_io_attached_t);
		break;
	default:
		expected_length = sizeof(hub_attached_io_attached_virtual_t);
		break;
	}

	if (message_length == expected_length) {
		return attached_io->event;
	}
	return -1;
}

int ucpu_is_port_value_single(ucpu_connection_t *ucpu_connection, int message_length)
{
	hub_port_value_single_t *port_value_single;

	if (message_length <= sizeof(hub_port_value_single_t)) {
		return -1;
	}

	port_value_single = (hub_port_value_single_t*)ucpu_connection->rsp_buf;

	if (port_value_single->common_message_header.message_type == HUB_PORT_VALUE_SINGLE) {
		return port_value_single->port_id;
	}

	return -1;
}

int ucpu_port_information_request(ucpu_connection_t *ucpu_connection, uint8_t port_id, uint8_t information_type)
{
	hub_port_information_request_t port_information_request;

	port_information_request.common_message_header.message_type = HUB_PORT_INFORMATION_REQUEST;
	port_information_request.port_id = port_id;
	port_information_request.information_type = information_type;

	return ucpu_send_command(ucpu_connection,
		&port_information_request.common_message_header, sizeof(hub_port_information_request_t));
}

int ucpu_port_input_format_setup(ucpu_connection_t *ucpu_connection, uint8_t port_id, uint8_t mode,
	uint32_t delta_interval, uint8_t notification_enabled)
{
	hub_port_input_format_setup_t port_input_format_setup;

	port_input_format_setup.common_message_header.message_type = HUB_PORT_INPUT_FORMAT_SETUP;
	port_input_format_setup.port_id = port_id;
	port_input_format_setup.mode = mode;
	UCPU_SET_32BIT_VALUE(port_input_format_setup.delta_interval, delta_interval);
	port_input_format_setup.notification_enabled = notification_enabled;

	return ucpu_send_command(ucpu_connection,
		&port_input_format_setup.common_message_header, sizeof(hub_port_input_format_setup_t));
}

int ucpu_virtual_port_connect(ucpu_connection_t *ucpu_connection, uint8_t port_id_a, uint8_t port_id_b)
{
	hub_virtual_port_connect_t virtual_port_connect;

	virtual_port_connect.common_message_header.message_type = HUB_VIRTUAL_PORT_SETUP;
	virtual_port_connect.sub_command = 1;
	virtual_port_connect.port_id_a = port_id_a;
	virtual_port_connect.port_id_b = port_id_b;

	return ucpu_send_command(ucpu_connection,
		&virtual_port_connect.common_message_header, sizeof(hub_virtual_port_connect_t));
}

int ucpu_virtual_port_disconnect(ucpu_connection_t *ucpu_connection, uint8_t port_id)
{
	hub_virtual_port_disconnect_t virtual_port_disconnect;

	virtual_port_disconnect.common_message_header.message_type = HUB_VIRTUAL_PORT_SETUP;
	virtual_port_disconnect.sub_command = 0;
	virtual_port_disconnect.port_id = port_id;

	return ucpu_send_command(ucpu_connection,
		&virtual_port_disconnect.common_message_header, sizeof(hub_virtual_port_disconnect_t));
}

int ucpu_set_led_color(ucpu_connection_t *ucpu_connection, uint8_t port_id, uint8_t color_id)
{
	hub_led_color_t led_color;

	UCPU_INIT_PORT_OUTPUT(led_color, port_id, 0x00, WRITE_DIRECT_MODE_DATA);

	led_color.mode = 0x0; /* Indexed mode. */
	led_color.color_id = color_id;

	return ucpu_send_command(ucpu_connection,
		&led_color.port_output_command.common_message_header, sizeof(hub_led_color_t));
}

int ucpu_set_led_rgb(ucpu_connection_t *ucpu_connection, uint8_t port_id, uint8_t r, uint8_t g, uint8_t b)
{
	hub_led_rgb_t led_rgb;

	UCPU_INIT_PORT_OUTPUT(led_rgb, port_id, 0x00, WRITE_DIRECT_MODE_DATA);

	led_rgb.mode = 0x1;
	led_rgb.rgb[0] = r;
	led_rgb.rgb[1] = g;
	led_rgb.rgb[2] = b;

	return ucpu_send_command(ucpu_connection,
		&led_rgb.port_output_command.common_message_header, sizeof(hub_led_rgb_t));
}

int ucpu_motor_start_speed(ucpu_connection_t *ucpu_connection, uint8_t port_id,
	int8_t speed, int8_t max_power, uint8_t use_profile)
{
	hub_motor_start_speed_t motor_start_speed;

	UCPU_INIT_PORT_OUTPUT(motor_start_speed, port_id, 0x10, HUB_MOTOR_START_SPEED);
	motor_start_speed.speed = speed;
	motor_start_speed.max_power = max_power;
	motor_start_speed.use_profile = use_profile;

	return ucpu_send_command(ucpu_connection,
		&motor_start_speed.port_output_command.common_message_header, sizeof(hub_motor_start_speed_t));
}

int ucpu_motor_goto_absolute_position(ucpu_connection_t *ucpu_connection, uint8_t port_id,
	int32_t absolute_pos, int8_t speed, int8_t max_power, int8_t end_state, uint8_t use_profile)
{
	hub_motor_goto_absolute_position_t motor_goto_absolute_position;

	UCPU_INIT_PORT_OUTPUT(motor_goto_absolute_position, port_id, 0x10, HUB_MOTOR_GOTO_ABSOLUTE_POSITION);
	UCPU_SET_32BIT_VALUE(motor_goto_absolute_position.absolute_pos, absolute_pos);
	motor_goto_absolute_position.speed = speed;
	motor_goto_absolute_position.max_power = max_power;
	motor_goto_absolute_position.end_state = end_state;
	motor_goto_absolute_position.use_profile = use_profile;

	return ucpu_send_command(ucpu_connection,
		&motor_goto_absolute_position.port_output_command.common_message_header, sizeof(hub_motor_goto_absolute_position_t));
}
