#include "s3d.h"
#include "s3dlib.h"
#include <stdarg.h>		 /*  va_list */
#include <stdio.h> 		 /*  perror(),fprintf() */
#include <string.h> 	 /*  sterror */
/*  dprintf is only for internal use. */
#ifdef DEBUG
void dprintf(int relevance, const char *fmt, ...) {
	char dbm[DBM_MAX];
	va_list args;
	if (relevance >= DEBUG )
	{
		va_start(args,fmt);
		vsnprintf((char *)&dbm,DBM_MAX,fmt,args);
		va_end(args);
	
		fprintf(stderr,"s3dlib: %s\n",(char *)&dbm);
	}
}
void errdn(int relevance, char *func,int en) {
	if (relevance >= DEBUG )
		 fprintf(stderr,"s3dlib error: %s: (%d) %s\n",func,en, strerror(en));
}

void errds(int relevance,char *func, const char *fmt, ...)
{
	char dbm[DBM_MAX];
	va_list args;
	if (relevance >= DEBUG )
	{
		va_start(args,fmt);
		vsnprintf((char *)&dbm,DBM_MAX,fmt,args);
		va_end(args);
	
		fprintf(stderr,"s3dlib error: %s:%s\n",func,(char *)&dbm);
	}
}
#endif
void errn(char *func,int en) {
	fprintf(stderr,"s3dlib error: %s: (%d) %s\n",func,en, strerror(en));
}
void errs(char *func, char *msg) {
	fprintf(stderr,"s3dlib error: %s: %s\n",func,msg);
}


