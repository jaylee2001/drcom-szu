/*
	libdrcom - Library for communicating with DrCOM 2133 Broadband Access Server
	Copyright (C) 2005 William Poetra Yoga Hadisoeseno <williampoetra@yahoo.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	02111-1307	USA
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include "drcomd.h"
#include "daemon_server.h"
#include "log.h"
////////////////////////Drcom5.2.0/////////////////////////////////
extern u_int32_t g_challenge;
struct keepalive_msg
{
	u_int8_t h;
	u_int8_t checksum[16];
	u_int8_t unkown_m1[11];
	u_int8_t unkown_m2[2];
	u_int8_t unkown_m3[5];
	u_int8_t unkown_m4[3];
};
///////////////////////////////////////////////////////////////////////
static int drcom_keepalive(struct drcom_handle *h)
{
	struct drcom_socks *socks = (struct drcom_socks *) h->socks;
	struct drcom_host_msg *host_msg = (struct drcom_host_msg *) h->keepalive;
	struct sockaddr_in servaddr_in;
	int r;

////////////////////////Drcom5.2.0///////////////////////////////////
	struct keepalive_msg msg;
	struct drcom_info *info = (struct drcom_info *) h->info;

	u_int8_t t[22];
	u_int8_t d[16];
	u_int16_t pkt_type = PKT_LOGIN;
	memset(t, 0, 22);
	memcpy(t, &pkt_type, 2);
	memcpy(t + 2, &g_challenge, 4);
	u_int32_t passwd_len = strlen(info->password);

		strncpy((char *) (t + 6), info->password, 16);
		MD5((unsigned char *) t, passwd_len + 6, d);
		memcpy(&(msg.checksum), d, 16);
	msg.h = 0xff;
	msg.unkown_m1[0] = 0x00;
	msg.unkown_m1[1] = 0x00;
	msg.unkown_m1[2] = 0x00;
	msg.unkown_m1[3] = 0x44;
	msg.unkown_m1[4] = 0x72;
	msg.unkown_m1[5] = 0x63;
	msg.unkown_m1[6] = 0x6f;
	msg.unkown_m1[7] = 0xac;
	msg.unkown_m1[8] = 0x10;
	msg.unkown_m1[9] = 0x03;
	msg.unkown_m1[10] = 0x64;

	msg.unkown_m3[0] = 0xc0;
	msg.unkown_m3[1] = 0xa8;
	msg.unkown_m3[2] = 0x1f;
	msg.unkown_m3[3] = 0x3b;
	msg.unkown_m3[4] = 0x01;

///////////////////////////////////////////////////////////////////////
	memcpy(&servaddr_in, &socks->servaddr_in, sizeof(servaddr_in));

	while (1)
	{
		r = sendto(socks->sockfd, host_msg, sizeof(struct drcom_host_msg), 0,
			(struct sockaddr *) &servaddr_in,
			sizeof(struct sockaddr));
		if (r != sizeof(struct drcom_host_msg))
			goto err;

//		r = sendto(socks->sockfd, &msg, sizeof(struct keepalive_msg), 0,(struct sockaddr *) &servaddr_in,sizeof(struct sockaddr));
//		if (r != sizeof(struct keepalive_msg))
//			goto err;

		sleep(10);

	}

err:
	logerr("keepalive: %s", strerror(errno));
	return -1;
}

void *daemon_keepalive(void *arg)
{
	block_sigusr1();
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	drcom_keepalive((struct drcom_handle *) arg);
	loginfo("keepalive returns\n");
	return NULL;
}

