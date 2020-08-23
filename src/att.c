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

/* Implement Bluetooth ATT (Attribute Protocol) */

#include "globals.h"

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

int ucpu_att_send(ucpu_connection_t *ucpu_connection, void *req_buf, uint16_t req_buf_len)
{
	ssize_t ret;

	while (1) {
		ret = send(ucpu_connection->sock, req_buf, req_buf_len, 0);

		if (ret != (int)req_buf_len) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				continue;
			}

			close(ucpu_connection->sock);
			ucpu_connection->sock = -1;
			return 1;
		}
		return 0;
	}
}

int ucpu_att_receive(ucpu_connection_t *ucpu_connection)
{
	ssize_t ret = recv(ucpu_connection->sock, ucpu_connection->rsp_buf, sizeof(ucpu_connection->rsp_buf), 0);

	if (ret <= 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			return 0;
		}

		close(ucpu_connection->sock);
		ucpu_connection->sock = -1;
		return -1;
	}

	return (int)ret;
}

int ucpu_send_command(ucpu_connection_t *ucpu_connection, hub_common_message_header_t *message, uint16_t message_len)
{
	message->opcode = ATT_WRITE_CMD;
	message->handle[0] = ucpu_connection->handle[0];
	message->handle[1] = ucpu_connection->handle[1];
	message->length = (uint8_t)(message_len - 3);
	message->hub_id = 0;

	return ucpu_att_send(ucpu_connection, message, message_len);
}
