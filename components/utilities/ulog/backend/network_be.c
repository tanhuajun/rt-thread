/*
 * Copyright (c) 2006-2022, tanhuajun
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-07     tanhuajun    the first version
 */
 
#include <rthw.h>
#include <ulog.h>
#include <sys/socket.h>
#include <netdb.h>

#ifdef ULOG_BACKEND_USING_NETWORK

static struct ulog_backend network = { 0 };
static int ulog_fd;
static struct sockaddr_in ulog_addr;

void ulog_network_backend_output(struct ulog_backend *backend, rt_uint32_t level, const char *tag, rt_bool_t is_raw,
        const char *log, rt_size_t len)
{
	if(ulog_fd == -1)
		return;
	
	sendto(ulog_fd, log, len, 0,
			(struct sockaddr*)&ulog_addr, sizeof(struct sockaddr));
}

void ulog_network_init(struct ulog_backend *backend) {
	ulog_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(ulog_fd == -1) {
		rt_kprintf("socket error\n");
		return;
	}
	
	rt_memset(&ulog_addr, 0, sizeof(ulog_addr));
	ulog_addr.sin_family = AF_INET;
	ulog_addr.sin_port = htons(1234);
	ulog_addr.sin_addr.s_addr = inet_addr("192.168.100.77");
}

void ulog_network_deinit(struct ulog_backend *backend) {
	if(ulog_fd == -1)
		return;
	closesocket(ulog_fd);
}

int ulog_network_backend_init(void) {
	ulog_init();
	
	network.init = ulog_network_init;
	network.deinit = ulog_network_deinit;
	network.output = ulog_network_backend_output;
	
	ulog_backend_register(&network, "network", RT_FALSE);
	
	return 0;
}

#endif
