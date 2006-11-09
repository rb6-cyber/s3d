#include <stdio.h>		 /*  snprintf(), printf(), NULL */
#include <stdlib.h>		 /*  malloc(),free() */
#include <sys/stat.h>	 /*  fstat() */
#include <s3d.h>
#include "s3dosm.h"
#include <time.h>	 /*  nanosleep(), struct tm, time_t...  */

void mainloop()
{
	struct timespec t={0,100*1000*1000}; /* 100 mili seconds */
	nanosleep(&t,NULL); 
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
	layer_t *layer1,*layer2;
	printf("loading data");
	layer1=load_kismet_file("kismet_sample.xml");
/*	layer2=load_osm_web(11.610952060700235,49.409270464751515,14.453271808922661,52.338403146460365);	*/	/* sachsen */
	layer2=load_osm_file("sachsen.osm");
	if (layer1==NULL || layer2==NULL)
	{
		printf("could not read data :(");
		return(-1);
	}
	printf("okay, drawing layer...\n");
	if (!s3d_init(&argc,&argv,"s3dosm"))
	{
		nav_init();
		draw_layer(layer1);
		draw_layer(layer2);
		nav_center(layer2->center_la,layer2->center_lo);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
