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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>

static const uint8_t hub_service_uuid[16] = {
	0x23, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x16,
	0xde, 0xef, 0x12, 0x12, 0x23, 0x16, 0x00, 0x00
};

static int ucpu_check_advertising_info(le_advertising_info *advertising_info)
{
	uint8_t *data = advertising_info->data;
	uint8_t *data_end = data + advertising_info->length;
	size_t length = 0;
	int technic_hub = 0;
	uint8_t *manufacturer_data = NULL;

	while (data < data_end) {
		length = 1 + (size_t)data[0];

		/* Advertising Data Types (AD types)
		 * See: https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile */
		/* Note: 0x09 (Complete Local Name) is only sent when active scanning is used. */
		switch (data[1]) {
		case 0x07: /* Complete List of 128-bit Service Class UUIDs */
			if (length == 1 + 17 && memcmp(data + 2, hub_service_uuid, 16) == 0) {
				technic_hub = 1;
			}
			break;
		case 0xFF: /* Manufacturer Specific Data */
			/* LEGO Manufacturer ID: 0x0397 */
			if (length == 1 + 9 && data[2] == 0x97 && data[3] == 0x03) {
				manufacturer_data = data + 2;
			}
			break;
		}

		data += length;
	}

	if (technic_hub && manufacturer_data != NULL) {
		return 1;
	}

	return 0;
}

static int ucpu_discover_hub(int hci_fd, struct sockaddr_l2 *dest_addr)
{
	int retry = 3;
	struct hci_filter old_filter, new_filter;
	hci_event_hdr *event_hdr;
	evt_le_meta_event *meta_event;
	le_advertising_info *advertising_info;
	socklen_t old_filter_len;
	uint8_t buf[HCI_MAX_EVENT_SIZE];
	int buf_fill, len;

	/* Enable scanning to discover the 6 byte address of a nearby Hub. Passive
	 * scanning is enough, because the Hub periodically repeats its advertising
	 * report (advertising_info->evt_type == 0x0: connectable undirected advertising)
	 * and the report contains enough data to identify the Hub. As for active scanning,
	 * the Hub sends another report (advertising_info->evt_type == 0x4: Scan response)
	 * with its name (e.g: Technic Hub), but this information is not needed for
	 * controlling the Hub. */

	while (--retry != 0) {
		const uint16_t interval = htobs(0x0010);
		const uint16_t window = htobs(0x0010);
		const uint8_t passive = 0x0;

		if (hci_le_set_scan_parameters(hci_fd, passive, interval, window,
				LE_PUBLIC_ADDRESS, 0x0, 10000) >= 0) {
			break;
		}

		if (errno != EIO) {
			return 1;
		}

		if (hci_le_set_scan_enable(hci_fd, 0x00, 0x00, 10000) < 0) {
			return 1;
		}
	}

	old_filter_len = sizeof(old_filter);
	if (getsockopt(hci_fd, SOL_HCI, HCI_FILTER, &old_filter, &old_filter_len) < 0) {
		return 1;
	}

	hci_filter_clear(&new_filter);
	hci_filter_set_ptype(HCI_EVENT_PKT, &new_filter);
	hci_filter_set_event(EVT_LE_META_EVENT, &new_filter);

	if (setsockopt(hci_fd, SOL_HCI, HCI_FILTER, &new_filter, sizeof(new_filter)) < 0) {
		return 1;
	}

	if (hci_le_set_scan_enable(hci_fd, 0x01, 0x0, 10000) < 0) {
		return 1;
	}

	printf("Start scanning...\n");

	buf_fill = 0;
	while (1) {
		len = read(hci_fd, buf + buf_fill, (int)sizeof(buf) - buf_fill);

		if (len < 0) {
			if (errno == EAGAIN) {
				continue;
			}
			return 1;
		}

		buf_fill += len;

		if (buf_fill < HCI_TYPE_LEN + HCI_EVENT_HDR_SIZE) {
			continue;
		}

		event_hdr = (hci_event_hdr*)(buf + HCI_TYPE_LEN);

		if (buf[0] != HCI_EVENT_PKT && event_hdr->evt != EVT_LE_META_EVENT) {
			printf("Unexpected event\n");
			break;
		}

		len = event_hdr->plen + HCI_TYPE_LEN + HCI_EVENT_HDR_SIZE;

		if (len < buf_fill) {
			continue;
		}

		meta_event = (evt_le_meta_event*)(buf + HCI_TYPE_LEN + HCI_EVENT_HDR_SIZE);

		/* Number of reports (meta_event->data[0] should be 1) */
		if (meta_event->subevent == EVT_LE_ADVERTISING_REPORT
				&& meta_event->data[0] == 1) {
			advertising_info = (le_advertising_info*)(meta_event->data + 1);
			if (ucpu_check_advertising_info(advertising_info) != 0) {
				memcpy(&dest_addr->l2_bdaddr, &advertising_info->bdaddr, sizeof(bdaddr_t));
				dest_addr->l2_bdaddr_type = advertising_info->bdaddr_type;
				break;
			}
		}

		if (len == buf_fill) {
			buf_fill = 0;
			continue;
		}

		buf_fill -= len;
		memmove(buf, buf + len, buf_fill);
	}

	if (hci_le_set_scan_enable(hci_fd, 0x00, 0x00, 10000) < 0) {
		return 1;
	}

	if (setsockopt(hci_fd, SOL_HCI, HCI_FILTER, &old_filter, sizeof(old_filter)) < 0) {
		return 1;
	}

	return 0;
}

#define HUB_SERVICE_CHARACTERISTIC_FOUND 0x1
#define CLIENT_CHARACTERISTIC_CONFIG_FOUND 0x2
#define ALL_CHARACTERISTICS_FOUND (HUB_SERVICE_CHARACTERISTIC_FOUND | CLIENT_CHARACTERISTIC_CONFIG_FOUND)

static int ucpu_get_characteristic_handle(ucpu_connection_t *ucpu_connection, uint8_t *client_config_handle)
{
	att_op_find_info_req_t find_info_req;
	uint32_t characteristics_found = 0;
	uint16_t starting_handle = 0x0001;
	uint16_t prev_starting_handle = 0;
	int received_len;
	uint8_t *src, *src_end;

	/* Instead of the 128 bit UUIDs, Blutooth uses 16 bit handles to
	 * access attributes. The following code obtains the handle of the
	 * single attribute defined by LEGO GATT service. */

	/* The handles are stored in a service definition list called GATT
	 * profile. The following code finds the service which has the LEGO
	 * Hub Characteristic. To enable notifications, the Client Characteristic
	 * Configuration Descriptor (CCCD) is searched as well. */

	while (1) {
		if (starting_handle <= prev_starting_handle) {
			break;
		}
		prev_starting_handle = starting_handle;

		find_info_req.opcode = ATT_OP_FIND_INFO_REQ;
		/* Little endian format. */
		find_info_req.starting_handle[0] = (uint8_t)(starting_handle & 0xff);
		find_info_req.starting_handle[1] = (uint8_t)(starting_handle >> 8);
		find_info_req.ending_handle[0] = 0xff; /* 0xffff */
		find_info_req.ending_handle[1] = 0xff;

		if (ucpu_att_send(ucpu_connection, &find_info_req, sizeof(att_op_find_info_req_t)) != 0) {
			return 1;
		}

		received_len = ucpu_att_receive(ucpu_connection);

		if (received_len < 2 + 4 || ucpu_connection->rsp_buf[0] != ATT_OP_FIND_INFO_RESP) {
			break;
		}

		src_end = ucpu_connection->rsp_buf + received_len;

		if (ucpu_connection->rsp_buf[1] == 1) {
			/* Length of 16 bit handle and 16 bit UUID pairs should be divisible by 4. */
			if (((received_len - 2) % 4) != 0) {
				break;
			}

			src = ucpu_connection->rsp_buf + 2;
			do {
				if (src[3] == 0x28 && (src[2] | 0x1) == 0x01) {
					/* Clear status when a new primary or secondary service starts. */
					characteristics_found = 0;
				}
				else if (src[3] == 0x29 && src[2] == 0x02) {
					/* Found a Client Characteristic Configuration Descriptor */
					client_config_handle[0] = src[0];
					client_config_handle[1] = src[1];

					characteristics_found |= CLIENT_CHARACTERISTIC_CONFIG_FOUND;
					if (characteristics_found == ALL_CHARACTERISTICS_FOUND) {
						return 0;
					}
				}
				src += 2 + 2;
			} while (src < src_end);

			/* Since handles should be returned in ascending order, the highest handle should be the last value. */
			starting_handle = (uint16_t)(((uint32_t)src_end[-4] | ((uint32_t)src_end[-3] << 8)) + 1);
			continue;
		}

		/* Length of 16 bit handle and 128 bit UUID pairs should be divisible by 18. */
		if (ucpu_connection->rsp_buf[1] != 2 || ((received_len - 2) % 18) != 0) {
			break;
		}

		if (!(characteristics_found & HUB_SERVICE_CHARACTERISTIC_FOUND)) {
			src = ucpu_connection->rsp_buf + 2;
			do {
				/* The characteristic and service UUIDs are nearly the same except one byte. */
				if (src[2 + 12] == 0x24) {
					src[2 + 12] = 0x23;

					if (memcmp(src + 2, hub_service_uuid, sizeof(hub_service_uuid)) == 0) {
						ucpu_connection->handle[0] = src[0];
						ucpu_connection->handle[1] = src[1];

						characteristics_found |= HUB_SERVICE_CHARACTERISTIC_FOUND;
						if (characteristics_found == ALL_CHARACTERISTICS_FOUND) {
							return 0;
						}
					}
				}
				src += 2 + 8;
			} while (src < src_end);
		}

		starting_handle = (uint16_t)(((uint32_t)src_end[-18] | ((uint32_t)src_end[-17] << 8)) + 1);
	}

	/* Close socket when service is not found. */
	close(ucpu_connection->sock);
	ucpu_connection->sock = -1;
	return 1;
}

int ucpu_connect_to_hub(ucpu_connection_t *ucpu_connection)
{
	int sock, dev_id, hci_fd, flags;
	struct sockaddr_l2 src_addr;
	struct sockaddr_l2 dest_addr;
	gatt_client_characteristic_configuration_t client_configuration;

	dev_id = hci_get_route(NULL);
	if (dev_id < 0) {
		printf("Cannot open device\n");
		return 1;
	}

	hci_fd = hci_open_dev(dev_id);
	if (hci_fd < 0) {
		printf("Cannot open HCI device\n");
		return 1;
	}

	if (ucpu_discover_hub(hci_fd, &dest_addr) != 0) {
		/* Note: scanning requires administrator rights (sudo) because
		 * it can be used to gather all information about the network. */
		printf("Scanning failed (sudo might be needed)\n");
		return 1;
	}

	close(hci_fd);

	printf("Connecting to LEGO Hub\n");

	/* Bluetooth provides a reliable transfer protocol called L2CAP (logical link
	 * control and adaptation protocol). Raw sockets are not recommended to use. */
	sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET /*| SOCK_NONBLOCK*/, BTPROTO_L2CAP);

	if (sock == -1) {
		printf("Cannot create socket\n");
		return 1;
	}

	/* Bluetooth uses logical channels for communication. Each channel represents
	 * a service which processes a given set of commands. The communication protocol
	 * of some channels are defined by the standard including the attribute protocol
	 * channel (0x4) which can be used to communicate with the Hub. */

	/* Noth: both the source and destination must use the same cid (channel identifier). */
	src_addr.l2_family = AF_BLUETOOTH;
	src_addr.l2_psm = 0;
	memset(&src_addr.l2_bdaddr, 0, sizeof(bdaddr_t));
	src_addr.l2_cid = htobs(0x4);
	src_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;

	if (bind(sock, (struct sockaddr*)&src_addr, sizeof(struct sockaddr_l2)) != 0) {
		close(sock);
		printf("Cannot bind socket\n");
		return 1;
	}

	dest_addr.l2_family = AF_BLUETOOTH;
	dest_addr.l2_psm = 0;
	dest_addr.l2_cid = htobs(0x4);
	dest_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;

	if (connect(sock, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_l2)) != 0) {
		close(sock);
		printf("Cannot connect to the device\n");
		return 1;
	}

	ucpu_connection->sock = sock;

	if (ucpu_get_characteristic_handle(ucpu_connection, client_configuration.handle) != 0) {
		printf("Cannot communicate with the device\n");
		return 1;
	}

	client_configuration.opcode = ATT_WRITE_CMD;
	/* Set bit 0 to 1: enable server notifications */
	client_configuration.configuration[0] = 0x1;
	client_configuration.configuration[1] = 0x0;

	if (ucpu_att_send(ucpu_connection, &client_configuration,
			sizeof(gatt_client_characteristic_configuration_t)) != 0) {
		printf("Cannot set client configuration\n");
		return 1;
	}

	flags = fcntl(sock, F_GETFL, 0);

	if (flags != -1) {
		flags = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
	}

	if (flags == -1) {
		close(sock);
		ucpu_connection->sock = -1;
		printf("Cannot set socket non-blocking\n");
		return 1;
	}

	printf("Connection completed\n");
	return 0;
}
