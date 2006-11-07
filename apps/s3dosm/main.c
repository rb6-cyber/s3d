#include <stdio.h>		 /*  snprintf(), printf(), NULL */
#include <stdlib.h>		 /*  malloc(),free() */
#include <sys/stat.h>	 /*  fstat() */
#include <s3d.h>
#include "http_fetcher.h"
#include "s3dosm.h"
#include <time.h>	 /*  nanosleep(), struct tm, time_t...  */

void mainloop()
{
	struct timespec t={0,100*1000*1000}; /* 100 mili seconds */
	nanosleep(&t,NULL); 
}
char *read_osm(float minlon, float minlat, float maxlon, float maxlat,int *length)
{
	int ret;
	char *user = "foo@packetmixer.de";
	char *pass = "foobar";
	char url[1024];
	char *fileBuf;						/* Pointer to downloaded data */
	snprintf(url,1024,"www.openstreetmap.org/api/0.3/map?bbox=%f,%f,%f,%f",minlon,minlat,maxlon,maxlat);
	printf("downloading url [ %s ]\n",url);

	http_setAuth(user,pass);
	ret = http_fetch(url, &fileBuf);	/* Downloads page */
	if(ret == -1)
	{	
		http_perror("http_fetch");	
		return(NULL);
	}
	if (length!=NULL) *length=ret;
	return(fileBuf);
	
}
char *read_file(char *fname, int *fsize)
{
	FILE *fp;
	char *buf=NULL;
	int filesize;
	struct stat bf;

	if ((fp = fopen(fname, "rt")) == NULL)	{ 	perror("read_file():fopen()"); 		return(NULL);	}
	if (fstat(fileno(fp),&bf))				{ 	perror("read_file():fstat()");		return(NULL);	}
	filesize=bf.st_size;
	if ((buf=malloc(filesize))==NULL)		{	perror("read_file():malloc()");		return(NULL);	}
	fread(buf, filesize, 1, fp);
	fclose(fp);
	if (fsize!=NULL) *fsize=filesize;
	return(buf);
}

int main(int argc, char **argv)
{
	char *file=NULL;
	layer_t *layer;
	int length;
	file=read_file("kismet_sample.xml",&length);
/*	printf("reading data from server ...\n");
	file=read_osm(11.610952060700235,49.409270464751515,14.453271808922661,52.338403146460365,&length);
	file=read_osm(12.8,50.6,13,51,&length);*/
	printf("okay, parsing data...\n");
	layer=parse_kismet(file,length);
	printf("okay, drawing layer...\n");
	if (!s3d_init(&argc,&argv,"s3dosm"))
	{
		draw_layer(layer);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
