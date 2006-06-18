/*
 * fs.c 
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dfm, a s3d file manager.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3dfm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3dfm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3dfm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "s3dfm.h"
#include <stdio.h>	/*  printf() */
#include <dirent.h>	/* scandir() */
#include <stdlib.h>	/*  malloc() */

/* we want GNU version of basename */
#define _GNU_SOURCE 
#include <string.h>	/*  strlen(), strncmp(), strrchr() */
#include <libgen.h> /* basename() */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>




/* generates the file list */
filelist *fl_new(char *path)
{
	struct dirent **namelist;
	filelist *fl;
	int n,i,j;
	char *name;

	fl=malloc(sizeof(filelist));
	fl->p=NULL;
	fl->n=0;
    n = scandir(path, &namelist, 0, alphasort);
    if (n <= 2) /* . and .. is always included. */
	{
		if (n<0)
	        perror("scandir");
	} else {
		j=0;
		fl->n=n-2 ; /* ignore . and .. */
		fl->p=malloc(sizeof(char *)*fl->n);
		for (i=0;i<n;i++)
		{
			name=namelist[i]->d_name;
			if (!((strcmp(name,".")==0) || (strcmp(name,"..")==0))) /* ignore */
			{
				fl->p[j]=malloc(strlen(name)+strlen(path)+2);
				strcpy(fl->p[j],path);
				strcat(fl->p[j],"/");
				strcat(fl->p[j],name);
				j++;
			}
			free(namelist[i]);
		}
		if (j!=fl->n)
		{
			printf("assertion failed\n");
			exit(-1);
		}
		free (namelist);
	}
	return(fl);	
}
/* delete the filelist */
void fl_del(filelist *fl)
{
	int i;
	for (i=0;i<fl->n;i++)
	{
		free(fl->p[i]);
	}
	free(fl);
}
/* approximate the heaviness of a single */
void fs_approx(char *source, int *files, int *dirs, int *bytes)
{
	int sfiles,sdirs,sbytes; 	/* for subdirs */
	filelist *fl;
	struct stat s;

	*files=	sfiles=	0;
	*dirs=	sdirs=	0;
	*bytes=	sbytes=	0;

	printf("start: %d files, %d dirs, %d bytes in %s\n",*files,*dirs,*bytes,source);
	if (-1==stat(source,&s))
		return;
/*	printf("%s: %08x (%db)\n",source,s.st_mode,(int)s.st_size);*/
	if ((s.st_mode&S_IFMT) == S_IFDIR)
	{
		
		fl=fl_new(source);
		if (fl->n>0)
			fs_fl_approx(fl,&sfiles,&sdirs,&sbytes);
		fl_del(fl);
		printf("%d files, %d dirs, %d bytes in %s\n",sfiles,sdirs,sbytes,source);

		*files=		sfiles;
		*bytes+=	sbytes;
		*dirs=		sdirs;
		*dirs+=		1;
	} else 
		*files=		1;
	*bytes+=s.st_size;
	printf("end: %d files, %d dirs, %d bytes in %s\n",*files,*dirs,*bytes,source);
}
/* approximate the heaviness of our source ...*/
void fs_fl_approx(filelist *fl, int *files, int *dirs, int *bytes)
{
	int i;
	int sfiles,sdirs,sbytes; 	/* for subdirs */

	*files=0;
	*dirs=0;
	*bytes=0;

	for (i=0;i<fl->n;i++)
	{
		fs_approx(fl->p[i],&sfiles,&sdirs,&sbytes);
		*files+=		sfiles;
		*dirs+=			sdirs;
		*bytes+=		sbytes;
	}
}
/* copy a certain file */
int fs_copy(char *source, char *dest)
{
	FILE *fps, *fpd;
	filelist *fl;
	struct stat s;
	char buf[1024];
	int n;
	if (-1==stat(source,&s))
		return(0);
	switch (s.st_mode&S_IFMT)
	{
		case S_IFDIR:
			fl=fl_new(source);
		
			printf("mkdir %s\n",dest);
			mkdir(dest,0777);
		
			fs_fl_copy(fl, dest);

			fl_del(fl);
			break;
		case S_IFIFO:
			printf("link the fifo\n");

			link(source,dest);
			break;
		default:
			printf("atomic copy ... from %s to %s\n", source, dest);
			if (NULL==(fps=fopen(source,"r"))) return(-1);
			if (NULL==(fpd=fopen(dest,"w"))) return(-1);
			/* TODO: overwrite protection etc */
		
			while (!feof(fps))
			{
				n=fread(buf,1,1024,fps);
				fwrite(buf,1,n,fpd);
			}
			fclose(fps);
			fclose(fpd);

	}
	return(0);
}
/* copy the source to the destination, destination should be a directory. */
int fs_fl_copy(filelist *fl, char *dest)
{
	int i;
	int r;
	char *sdest;
	char *bname;
	r=0;
	for (i=0;i<fl->n;i++)
	{
		bname=basename(fl->p[i]);
		sdest=malloc(strlen(dest)+strlen(bname)+2);

		strcpy(sdest,dest);
		strcat(sdest,"/");
		strcat(sdest,bname);
		r|=fs_copy(fl->p[i],sdest);

		free(sdest);
	}

	return(r);
}
/* recursively unlink a dir or file. */
int fs_unlink(char *dest)
{
	filelist *fl;
	struct stat s;
	
	if (-1==stat(dest,&s))		return(-1);
	if ((s.st_mode&S_IFMT) == S_IFDIR)
	{
		printf("%s is a dir, removing below ...\n",dest);
		fl=fl_new(dest);
		if (fs_fl_unlink(fl)) {
			fl_del(fl);
			return(-1);
		} else {
			fl_del(fl);
			printf("removing %s\n",dest);
			if (rmdir(dest)==-1)
			{
				perror("fs_fl_unlink(): rmdir()");
				return(-1);
			}
		}
	} else {
		if (unlink(dest)==-1)
		{
			perror("fs_fl_unlink(): unlink()");
			return(-1);
		}
	}
	return(0);
}
/* remove a lot of files */
int fs_fl_unlink(filelist *fl)
{
	int i,r;
	r=0;
	for (i=0;i<fl->n;i++)
	{
		printf("-> atomic unlink %s\n",fl->p[i]);
		r|=fs_unlink(fl->p[i]);
	}
	return(r);

}
int fs_move(char *source, char *dest)
{
	if (!rename(source,dest))
	{
		switch (errno)
		{
			case EXDEV:
				fs_copy(source,dest);
				fs_unlink(source);
				break;
			default: 
				perror("fs_move()");
				return(-1); /* can't help it */
			
		}
	}
	return(0);	
}

/* moves the source to the destination */
int fs_fl_move(filelist *fl, char *dest)
{
	int i;
	int r;
	char *sdest;
	char *bname;
	r=0;
	for (i=0;i<fl->n;i++)
	{
		bname=basename(fl->p[i]);
		sdest=malloc(strlen(dest)+strlen(bname)+2);

		strcpy(sdest,dest);
		strcat(sdest,"/");
		strcat(sdest,bname);
		r|=fs_move(fl->p[i],sdest);

		free(sdest);
	}

	return(r);

	return(0);
}


