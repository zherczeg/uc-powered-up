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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int print_tilt_values(uint8_t *buffer)
{
	int x, y, z;

	x = (int16_t)((int)buffer[0] | ((int)buffer[1] << 8));
	y = (int16_t)((int)buffer[2] | ((int)buffer[3] << 8));
	z = (int16_t)((int)buffer[4] | ((int)buffer[5] << 8));

	printf("Tilt: x:%d y:%d z:%d\n", x, y, z);
}

int main(int argc, char **argv)
{
	ucpu_connection_t ucpu_connection;
	int received_bytes;
	/* Technic Hub 88012 has a built-in tilt sensor on port 99. */
	uint8_t port_id = 99;

	if (ucpu_connect_to_hub(&ucpu_connection) != 0) {
		return 1;
	}

	ucpu_port_input_format_setup(&ucpu_connection, port_id, 0, 4, 1);

	while (1) {
		received_bytes = ucpu_att_receive(&ucpu_connection);
		if (received_bytes == -1) {
			return 1;
		}

		if (received_bytes > 0 && ucpu_is_notification(&ucpu_connection, received_bytes)
				&& ucpu_is_port_value_single(&ucpu_connection, received_bytes) == port_id
				&& received_bytes == (int)(sizeof(hub_port_value_single_t) + 3 * 2)) {
			print_tilt_values(ucpu_connection.rsp_buf + sizeof(hub_port_value_single_t));
		}
	}

	close(ucpu_connection.sock);
	return 0;
}
