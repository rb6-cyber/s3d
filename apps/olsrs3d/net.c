#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>		/* fnctl() */
#include "olsrs3d.h"

#define PORT 2004 		/* the port client will be connecting to  */
char buf[MAXDATASIZE];


int sockfd, numbytes;
int net_init(char *host)
{
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information  */

    if ((he=gethostbyname(host)) == NULL) {  /* get the host info  */
        herror("gethostbyname");
        return(1);
    }

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return(1);
    }

    their_addr.sin_family = AF_INET;    /* host byte order  */
    their_addr.sin_port = htons(PORT);  /* short, network byte order  */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&their_addr,
                                          sizeof(struct sockaddr)) == -1) {
        perror("connect");
        return(1);
    }
	fcntl(sockfd,F_SETFL, O_NONBLOCK);
	return(0);
}
int net_main()
{
	if ((numbytes=recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		if (errno==EAGAIN)
			return(0); /* well, that's okay ... */
		perror("recv");
		return(-1);
	}
	if (numbytes==0)
	{
		printf("connection reset\n");
		return(-1);
	}
	buf[numbytes] = '\0';
 	/*strncat(lbuf,buf,MAXLINESIZE);*/
	strncpy(lbuf,buf,MAXLINESIZE);
	process_main();
	return(1);
}
int net_quit()
{
    close(sockfd);

    return 0;
}


