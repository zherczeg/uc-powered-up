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

int main(int argc, char **argv)
{
	ucpu_connection_t ucpu_connection;
	hub_attached_io_attached_virtual_t *attached_io_attached_virtual;
	int received_bytes;
	uint8_t port_id;

	if (ucpu_connect_to_hub(&ucpu_connection) != 0) {
		return 1;
	}

	/* This test joins two motors and controls them in a "synchronized" way. */

	/* Drain notifications. */
	do {
		received_bytes = ucpu_att_receive(&ucpu_connection);
		if (received_bytes == -1) {
			return 1;
		}
	} while (received_bytes > 0);

	/* Connect port 0 and port 1 and wait for its virtual port id. */
	ucpu_virtual_port_connect(&ucpu_connection, 0, 1);

	while (1) {
		received_bytes = ucpu_att_receive(&ucpu_connection);
		if (received_bytes == -1) {
			return 1;
		}

		if (received_bytes > 0) {
			if (ucpu_is_attached_io_update(&ucpu_connection, received_bytes) == HUB_ATTACHED_IO_ATTACHED_VIRTUAL) {
				attached_io_attached_virtual = (hub_attached_io_attached_virtual_t*)ucpu_connection.rsp_buf;

				/* Only this port connection should happen. */
				if (attached_io_attached_virtual->port_id_a != 0 || attached_io_attached_virtual->port_id_b != 1) {
					printf("Unexpected virtual port creation\n");
					close(ucpu_connection.sock);
					return 1;
				}

				port_id = attached_io_attached_virtual->attached_io.port_id;
				printf("Virtual port (%d) is successfully created\n", port_id);
				break;
			}
		}
	}

	/* Move forward for 3 sec, than backwards for 3 sec. */
	ucpu_motor_start_speed(&ucpu_connection, port_id, -70, 70, 0);
	sleep(3);
	ucpu_motor_start_speed(&ucpu_connection, port_id, 70, 70, 0);
	sleep(3);
	ucpu_motor_start_speed(&ucpu_connection, port_id, 0, 0, 0);

	close(ucpu_connection.sock);
	return 0;
}
