#include "s3d.h"
#include "s3dlib.h"
#include <string.h> 	 /*  memcpy() */
#include <stdlib.h>		 /*  malloc(), free() */
#include <unistd.h>		 /*  read(), write() */
#include <errno.h>		 /*  errno */
#include <netinet/in.h>  /*  htons(),htonl() */
#ifdef WITH_SIGNALS
extern int _s3d_sigio;
#endif
int con_type=CON_NULL;
static int _s3d_net_receive();

int net_send(unsigned char opcode, char *buf, unsigned short length)
{
	char *ptr;
/* 	char *buff; */
	char buff[65536];  /*  unsigned short really shouldn't be bigger ;) */
	*(buff)=opcode; 
	ptr=buff+1;
	*((unsigned short *) ptr)=htons(length);
	memcpy(buff+3,buf,length);
	switch (con_type)
	{
		case CON_SHM:shm_writen(buff,length+3);break;
		case CON_TCP:tcp_writen(buff,length+3);break;
	}
	return(0);
}
/* handler for socket based connection types */
int _s3d_net_receive()
{
	return(_s3d_tcp_net_receive());
}
int s3d_net_check()
{
	switch (con_type)
	{
		case CON_TCP:
#ifdef WITH_SIGNALS
		if (_s3d_sigio)
		{
#endif
			while (_s3d_net_receive());
#ifdef WITH_SIGNALS
			_s3d_sigio=0;
		}	
#endif
			break;
		case CON_SHM:
			while(_shm_net_receive());
			break;
	}
	return(0);
}
int s3d_net_init(char *urlc)
{
	char				*s,*sv,*port=NULL;
	char				*first_slash=NULL;
	int					 pn=0;
	int					 tcp,shm;
	tcp=shm=1; /* everything is possible, yet */
	
	 /*  doing a very bad server/port extraction, but I think it'll work ... */
	s=sv=urlc+6;  /*  getting to the "real" thing */
	 /* while (((*s!='/') && (*s!=0)) && (s<(urlc-6))) */
	while (*s!=0)
	{
		if (*s=='/')
		{
			if (first_slash==NULL)
				first_slash=s;
			if (port!=NULL)
				break;
		}
		if (*s==':')  /*  there is a port in here */
		{
			port=s+1;
			*s=0;	 /*  NULL the port  */
		}
		s++;
	}

	*s=0;
	if (port==NULL)
	{
		shm=0;
		if (first_slash!=NULL)
			*first_slash=0;
	} else {
		if (first_slash<port)
			tcp=0;
		else 
			if (first_slash!=NULL)
				*first_slash=0;
		if (!strncmp(port, "shm",3))
		{
			tcp=0; /* null the others */
		} else {
			shm=0;
		}
	}
	if (shm)
	{
		if (!strncmp(port, "shm",3))
			if (!_shm_init(sv)) return(con_type=CON_SHM);
	}
	if (tcp)
	{
		pn=6066;
		if (port!=NULL)
		{
			if (!(pn=atoi(port)))  /*  I hope atoi is safe enough. */
			{
				errn("s3d_init():atoi()",errno);
				pn=6066;
			} 
		}
		if (!_tcp_init(sv,pn)) return(con_type=CON_TCP);
	}
	return(CON_NULL);
}
