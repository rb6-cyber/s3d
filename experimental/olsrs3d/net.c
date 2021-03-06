// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 2004-2015  Marek Lindner <mareklindner@neomailbox.ch>
 * SPDX-FileCopyrightText: 2004-2015  Andreas Langer <an.langer@gmx.de>
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* close() */
#include <errno.h>
#include <string.h>  /* strlen(), memmove(), strncpy(), strncat() */
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>  /* fnctl() */
#include "olsrs3d.h"

#define PORT 2004   /* the port client will be connecting to  */
static char buf[MAXDATASIZE];


static int sockfd, numbytes;
int net_init(char *host)
{
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information  */

	if ((he = gethostbyname(host)) == NULL) {  /* get the host info  */
		herror("gethostbyname");
		return 1;
	}

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 1;
	}

	their_addr.sin_family = AF_INET;    /* host byte order  */
	their_addr.sin_port = htons(PORT);  /* short, network byte order  */
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  /* zero the rest of the struct */

	if (connect(sockfd, (struct sockaddr *)&their_addr,
	                sizeof(struct sockaddr)) == -1) {
		close(sockfd);
		sockfd = -1;
		perror("connect");
		return 1;
	}
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	return 0;
}

int net_main(void)
{
	int len_lbuf;

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
		if (errno == EAGAIN)
			return 0; /* well, that's okay ... */
		perror("recv");
		return -1;
	}

	if (numbytes == 0) {
		printf("connection reset\n");
		return -1;
	}

	buf[numbytes] = '\0';

	/* check for potential buffer overflow */
	len_lbuf = strlen(lbuf);
	if ((len_lbuf + strlen(buf)) < MAXLINESIZE) {

		strncat(lbuf, buf, MAXLINESIZE - len_lbuf - 1);

	} else {

		/* hope that carriage return is now in buf */
		if (len_lbuf < MAXLINESIZE) {

			if (Debug) printf("WARNING: lbuf almost filled without *any* carriage return within that data !\nAppending truncated buf to lbuf to prevent buffer overflow.\n");
			strncat(lbuf, buf, MAXLINESIZE - len_lbuf - 1);

		} else {

			if (Debug) printf("ERROR: lbuf filled without *any* carriage return within that data !\nClearing lbuf to prevent buffer overflow.\n");
			strncpy(lbuf, buf, MAXLINESIZE);
			if (MAXLINESIZE > 0)
				lbuf[MAXLINESIZE - 1] = '\0';

		}

	}

	process_main();

	if (++Net_read_count > 5) {
		return 0;   /* continue mainloop */
	} else {
		return 1;   /* continue reading data from socket */
	}

}

int net_quit(void)
{
	close(sockfd);

	return 0;
}


