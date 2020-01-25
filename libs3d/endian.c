// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  S3D contributors
 */

#include <arpa/inet.h>               /* for htonl, ntohl, htons, ntohs */
#include <stdint.h>                  /* for uint32_t, uint16_t */

/* convert buffer with floats from host to network endianess */
void htonfb(float* netfloat, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		*(uint32_t*)&netfloat[i] = htonl(*(uint32_t*)&netfloat[i]);
	}
}

/* convert buffer with floats from network to host endianess */
void ntohfb(float* netfloat, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		*(uint32_t*)&netfloat[i] = ntohl(*(uint32_t*)&netfloat[i]);
	}
}

/* convert buffer with uint32_ts from host to network endianess */
void htonlb(uint32_t* netint32, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		netint32[i] = htonl(netint32[i]);
	}
}

/* convert buffer with uint32_ts from network to host endianess */
void ntohlb(uint32_t* netint32, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		netint32[i] = ntohl(netint32[i]);
	}
}

/* convert buffer with uint16_ts from host to network endianess */
void htonsb(uint16_t* netint16, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		netint16[i] = htons(netint16[i]);
	}
}

/* convert buffer with uint16_ts from network to host endianess */
void ntohsb(uint16_t* netint16, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		netint16[i] = ntohs(netint16[i]);
	}
}
