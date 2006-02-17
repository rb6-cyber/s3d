#include "s3d.h"
#include "s3dlib.h"
#include "proto.h"
#include <stdio.h>  	 /*  fopen(),fclose(),fileno() */
#include <string.h> 	 /*  strncpy(),strncmp(),memcpy() */
#include <stdlib.h>		 /*  atoi(),malloc(),free() */
#include <sys/stat.h>	 /*  fstat() */
#include <unistd.h>		 /*  getpid(), fstat() */
#include <errno.h>		 /*  errno */
#include <sys/socket.h>  /*  socket() */
#include <getopt.h>		 /*  getopt() */
#include <fcntl.h>		 /*  fcntl() */
#ifdef WITH_SIGNALS
#include <signal.h>		 /*  signal.h, SIG_PIPE */
#endif
#include <netinet/in.h>  /*  htons(),htonl() */
#ifndef WIN32
	#include <netdb.h>		 /*  gethostbyname()  */
#endif

static char				*url=NULL;
extern int 				con_type;
/*  this file is the client-lib-implementation which holds the function to connect and control the server. */
#ifdef WITH_SIGNALS
int _s3d_sigio=0;
void sigpipe_handler(int sig, int code)  /*  ... ? */
{
	errs("sigpip_handler()","there is a broken pipe somewhere");
}
void sigio_handler(int sig, int code)  /*  ... ? */
{
	_s3d_sigio=1;
}
void sigint_handler(int sig, int code)  /*  ... ? */
{
	s3d_quit();
}

#endif
int s3d_usage()
{
	printf("s3d-parameters:\n");
	printf(" --s3d-url <url>: skip S3D enviroment and connect to this url\n");
	printf(" --help, -?, -h, --s3d-help: this helpful text\n");
}

static int parse_args(int *argc, char ***argv)
{
	char				 c;
	int					 lopt_idx;
	struct option long_options[] =
	{
		{"s3d-url",1,0,0},
		{"help",0,0,'h'},
		{"s3d-help",0,0,'h'},
		{0,0,0,0}
	};
	while (-1!=(c=getopt_long(*argc,*argv,"?h",long_options,&lopt_idx)))
	{
		switch (c)
		{
		case 0:
			if (0==strcmp(long_options[lopt_idx].name,"s3d-url"))
			{
				if (optarg)
				{
					url=optarg;
					dprintf(HIGH,"connecting to %s",url);
				}
			}
			break;
		case '?':
		case 'h':
			printf("usage: %s [options]",(*argv)[0]);
			s3d_usage();
			return(-1);
		}
	}
	if (*argc>0)
	{
		*argc-=(optind-1); 				 /*  hide s3d-options */
		(*argv)[optind-1]=(*argv)[0]; 	 /*  restore program path */
		*argv+=(optind-1); 				 /*  set the string pointer at the right position */
	}
	return(0);
}
/*  external functions go here ... */
int s3d_init(int *argc, char ***argv, char *name)
{
	char 				*s;
	char 				 urlc[256];		 /*  this should be enough for an url */
	char 				 buf[258]; 		 /*  server buffer */
	int 				 i;
	 /*  null the callback table */
	for (i=0;i<MAX_CB;i++)
	{
		s3d_cb_list[i]=NULL;
	}
	/* ignore some things ... */
	s3d_ignore_callback(S3D_EVENT_KEY);
	s3d_ignore_callback(S3D_EVENT_OBJ_CLICK);
	s3d_ignore_callback(S3D_EVENT_OBJ_INFO);
	s3d_ignore_callback(S3D_EVENT_NEW_OBJECT);

	if (NULL!=(s=getenv("S3D")))
	{
		dprintf(VLOW,"at least we have the enviroment variable ... %s",s);
		url=s;
	}
	parse_args(argc,argv);
	if (url==NULL) /* no url specified or obtained through arguments */
	{
		/* trying standard ways to connect */
		strncpy(urlc,"s3d:///tmp/.s3d:shm/",256);
		if (s3d_net_init(urlc)==CON_NULL)
		{
			strncpy(urlc,"s3d://127.0.0.1:6066/",256);
			if (s3d_net_init(urlc)==CON_NULL)
				return(-1);
		}
	} else {
		strncpy(urlc,url,256);	 /*  this should keep buffer overflows away, maybe */
		urlc[256]=0;			 /*  just to make sure */
		if (!strncmp(urlc, "s3d:// ",6))
		{
			if (s3d_net_init(urlc)==CON_NULL) return(-1);
		} else {
			errs("s3d_init()","invalid url");
			  return(-1);
		}
	}
	strncpy(buf,name,256);  /*  copy the name ... */
	net_send(S3D_P_C_INIT,buf,strlen(buf));

	 /*  TODO: we should wait for the INIT-event here before proceeding. */
	_queue_init();
#ifdef SIGNAL
    if (signal(SIGINT, (sig_t)sigint_handler) == SIG_ERR)
		errn("s3d_init():signal()",errno);
    if (signal(SIGTERM, (sig_t)sigint_handler) == SIG_ERR)
		errn("s3d_init():signal()",errno);

#endif
	return(0);
}
/*  shuts down the socket, clearing the stack */
int s3d_quit()
{
	struct s3d_evt *ret;
	net_send(S3D_P_C_QUIT,NULL,0);
	switch (con_type)
	{
		case CON_TCP:_tcp_quit();break;
		case CON_SHM:_shm_quit();break;
	}
	con_type=CON_NULL;
	_queue_quit();
	while (NULL!=(ret=s3d_pop_event())) s3d_delete_event(ret);  /*  clear the stack ... */
	ret=malloc(sizeof(struct s3d_evt));
	ret->event=S3D_EVENT_QUIT;
	ret->length=0;
	s3d_push_event(ret);
	return(0);
}
/*  apps should use that as main loop for their programs. */
int s3d_mainloop(void (*f)())
{
	while (con_type!=CON_NULL)
	{
		if (f!=NULL)
			f();
		s3d_net_check();
	}
	return(0);
}
/*  opens a file returning it's filesize  */
/*  and setting *pointer to the buffer. to be freed */
int s3d_open_file(char *fname, char **pointer)
{
	FILE *fp;
	char *buf=NULL;
	int filesize;
	struct stat bf;
	*pointer=NULL;
/*	if ((fp = fopen(fname, "rt")) == NULL)
	{ errn("s3d_open_file():fopen()",errno); return(0);}
	if (fseek(fp, 0, SEEK_END) != 0)
	{ errn("s3d_open_file():fseek()",errno); return(0);}
	if ((filesize = (int)ftell(fp)) == (long)-1)
	{ errn("s3d_open_file():ftell()",errno); return(0);}
	if (fseek(fp, 0, SEEK_SET) != 0)
	{ errn("s3d_open_file():fseek()",errno); return(0);}*/

	if ((fp = fopen(fname, "rt")) == NULL)
	{ errn("s3d_open_file():fopen()",errno); return(-1);}
	if (fstat(fileno(fp),&bf))
	{ errn("s3d_open_file():fopen()",errno); return(-1);}
	filesize=bf.st_size;
	dprintf(LOW, "opening %s, filesize is %d",fname, filesize);
	if ((buf=malloc(filesize))==NULL)
	{
		errn("s3d_open_3ds_file():malloc()",errno);
		exit(-1);
	}
	fread(buf, filesize, 1, fp);
	fclose(fp);
	*pointer=buf;
	return(filesize);
}
