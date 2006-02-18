#include "global.h"	 
#include <stdio.h>	 /*  for printf() */
#include <stdarg.h>	 /*  va_start, va_end */
#include <string.h>  /*  for strerror() */
#include <stdlib.h>  /*  for exit() */
#define 	DBM_MAX		1024  /*  debug message buffer size */
/*  this function writes an error somewhere */
/*  basicly, this is for upcoming logfiles, or maybe draw error-messages into */
/*  the 3d-space */
/*  this is the generic failure routine ... */
void errn(char *func,int en) {
	fprintf(stderr,"error: %s: (%d) %s\n",func,en, strerror(en));
}
/*  ... and it's fatal pendant */
void errnf(char *func,int en) {
	fprintf(stderr,"FATAL: %s: (%d) %s\n",func,en, strerror(en));
	exit(-1);
}

/*  prints an error with the function and it's error-message */
void errs(char *func, char *msg) {
	fprintf(stderr,"error: %s: %s\n",func,msg);
}

void errsf(char *func, char *msg) {
	fprintf(stderr,"FATAL: %s: %s\n",func,msg);
	exit(-1);
}
#ifdef DEBUG
/*  printing error message */
void errds(int relevance,char *func, const char *fmt, ...)
{
	char dbm[DBM_MAX];
	va_list args;
	if (relevance >= DEBUG )
	{
		va_start(args,fmt);
		vsnprintf((char *)&dbm,DBM_MAX,fmt,args);
		va_end(args);
	
		fprintf(stderr,"error: %s:%s\n",func,(char *)&dbm);
	}
}
/*  printing debug message */

void dprintf(int relevance, const char *fmt, ...) {
	char dbm[DBM_MAX];
	va_list args;
	if (relevance >= DEBUG )
	{
		va_start(args,fmt);
		vsnprintf((char *)&dbm,DBM_MAX,fmt,args);
		va_end(args);
	
/*		fprintf(stderr,"debug: %s\n",(char *)&dbm);*/
		fprintf(stdout,"debug: %s\n",(char *)&dbm);
	}
}

#endif
