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
#include <sys/socket.h>

void print_attached_io_update(ucpu_connection_t *ucpu_connection, int update_type)
{
	hub_attached_io_t *attached_io = (hub_attached_io_t*)ucpu_connection->rsp_buf;
	uint8_t io_type_id[2];

	if (update_type == HUB_ATTACHED_IO_DETACHED) {
		printf("Port %d detached\n", attached_io->port_id);
		return;
	}

	if (update_type == HUB_ATTACHED_IO_ATTACHED) {
		printf("Port %d attached\n", attached_io->port_id);
		io_type_id[0] = ((hub_attached_io_attached_t*)attached_io)->io_type_id[0];
		io_type_id[1] = ((hub_attached_io_attached_t*)attached_io)->io_type_id[1];
	} else {
		/* Should not happen since this port type can only be created from software. */
		printf("Virtual port %d attached\n", attached_io->port_id);
		io_type_id[0] = ((hub_attached_io_attached_t*)attached_io)->io_type_id[0];
		io_type_id[1] = ((hub_attached_io_attached_t*)attached_io)->io_type_id[1];
	}

	if (io_type_id[1] != 0) {
		printf("    Unknown device: 0x%2x%2x\n", io_type_id[0], io_type_id[1]);
		return;
	}

	switch (io_type_id[0]) {
	case 0x01:
		printf("    Basic motor\n");
		break;
	case 0x02:
		printf("    System Train Motor\n");
		break;
	case 0x05:
		printf("    Button\n");
		break;
	case 0x08:
		printf("    LED light\n");
		break;
	case 0x14:
		printf("    Voltage sensor\n");
		break;
	case 0x15:
		printf("    Current sensor\n");
		break;
	case 0x16:
		printf("    Piezo tone (sound)\n");
		break;
	case 0x17:
		printf("    Hub LED light\n");
		break;
	case 0x22:
		printf("    External tilt sensor\n");
		break;
	case 0x23:
		printf("    Motion sensor\n");
		break;
	case 0x25:
		printf("    Color distance sensor\n");
		break;
	case 0x26:
		printf("    External motor with tacho\n");
		break;
	case 0x27:
		printf("    Internal motor with tacho\n");
		break;
	case 0x28:
		printf("    Internal tilt sensor\n");
		break;
	case 0x29:
		printf("    Duplo train motor\n");
		break;
	case 0x2a:
		printf("    Duplo train speaker\n");
		break;
	case 0x2b:
		printf("    Duplo train color sensor\n");
		break;
	case 0x2c:
		printf("    Duplo train speedometer\n");
		break;
	case 0x2e:
		printf("    Technic large motor\n");
		break;
	case 0x2f:
		printf("    Technic extra large motor\n");
		break;
	case 0x30:
		printf("    Technic medium angular motor\n");
		break;
	case 0x31:
		printf("    Technic large angular motor\n");
		break;
	case 0x36:
		printf("    Hub gest sensor\n");
		break;
	case 0x37:
		printf("    Remote control button\n");
		break;
	case 0x38:
		printf("    Remote control RSSI\n");
		break;
	case 0x39:
		printf("    Hub accelerometer\n");
		break;
	case 0x3a:
		printf("    Hub gyro sensor\n");
		break;
	case 0x3b:
		printf("    Hub tilt sensor\n");
		break;
	case 0x3c:
		printf("    Hub temperature sensor\n");
		break;
	case 0x3d:
		printf("    Color sensor\n");
		break;
	case 0x3e:
		printf("    Distance sensor\n");
		break;
	case 0x3f:
		printf("    Force sensor\n");
		break;
	case 0x4b:
		printf("    Technic medium angular motor (grey)\n");
		break;
	case 0x4e:
		printf("    Technic large angular motor (grey)\n");
		break;
	default:
		printf("    Unknown device: 0x%2x%2x\n", io_type_id[0], io_type_id[1]);
		break;
	}
}

int main(int argc, char **argv)
{
	ucpu_connection_t ucpu_connection;
	int received_bytes;

	if (ucpu_connect_to_hub(&ucpu_connection) != 0) {
		return 1;
	}

	while (1) {
		received_bytes = ucpu_att_receive(&ucpu_connection);
		if (received_bytes == -1) {
			return 1;
		}

		if (received_bytes > 0 && ucpu_is_notification(&ucpu_connection, received_bytes)) {
			int update_type = ucpu_is_attached_io_update(&ucpu_connection, received_bytes);

			if (update_type >= 0) {
				print_attached_io_update(&ucpu_connection, update_type);
			}
		}
	}

	close(ucpu_connection.sock);
	return 0;
}
