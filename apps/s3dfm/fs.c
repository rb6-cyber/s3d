/*
 * fs.c
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <stdio.h> /*  printf() */
#include <dirent.h> /* scandir() */
#include <stdlib.h> /*  malloc() */

/* we want GNU version of basename */
#define _GNU_SOURCE
#include <string.h> /*  strlen(), strncmp(), strrchr() */
#include <libgen.h> /* basename() */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h> /* errno */
#include <time.h> /* nanosleep() */

struct fs_error fs_err = {
	0, 0, NULL, NULL
};


/* generates the file list */
filelist *fl_new(char *path)
{
	struct dirent **namelist;
	filelist *fl;
	int n, i, j;
	char *name;

	fl = (filelist*)malloc(sizeof(filelist));
	fl->p = NULL;
	fl->n = 0;
	n = scandir(path, &namelist, NULL, alphasort);
	if (n <= 2) { /* . and .. is always included. */
		if (n < 0)
			fs_error("fl_new():scandir()", path);
	} else {
		j = 0;
		fl->n = n - 2 ; /* ignore . and .. */
		fl->p = (struct _t_file*)malloc(sizeof(t_file) * fl->n);
		for (i = 0;i < n;i++) {
			name = namelist[i]->d_name;
			if (!((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0))) { /* ignore */
				fl->p[j].name = (char*)malloc(strlen(name) + strlen(path) + 2);
				strcpy(fl->p[j].name, path);
				strcat(fl->p[j].name, "/");
				strcat(fl->p[j].name, name);
				fl->p[j].anode = NULL;
				fl->p[j].size = 0; /*TODO: later */
				fl->p[j].state = STATE_NONE;

				j++;
			}
			free(namelist[i]);
		}
		if (j != fl->n) { /* TODO: GUH! don't exit(-1) */
			printf("assertion failed\n");
			exit(-1);
		}
		free(namelist);
	}
	return(fl);
}
/* delete the filelist */
void fl_del(filelist *fl)
{
	int i;
	for (i = 0;i < fl->n;i++) {
		free(fl->p[i].name);
		if (fl->p[i].anode != NULL) {
			/* maybe let node_delete do that? */
			/*node_delete(fl->p[i].anode);*/
			icon_undisplay(fl->p[i].anode);
			free(fl->p[i].anode);
		}
	}
	free(fl);
}
/* approximate the heaviness of a single */
void fs_approx(char *source, int *files, int *dirs, int *bytes)
{
	int sfiles, sdirs, sbytes;  /* for subdirs */
	filelist *fl;
	struct stat s;

	*files = sfiles = 0;
	*dirs = sdirs = 0;
	*bytes = sbytes = 0;

	/* printf("start: %d files, %d dirs, %d bytes in %s\n",*files,*dirs,*bytes,source);*/
	if (-1 == stat(source, &s))
		return;
	/* printf("%s: %08x (%db)\n",source,s.st_mode,(int)s.st_size);*/
	if ((s.st_mode&S_IFMT) == S_IFDIR) {

		fl = fl_new(source);
		if (fl->n > 0)
			fs_fl_approx(fl, &sfiles, &sdirs, &sbytes);
		fl_del(fl);
		/*  printf("%d files, %d dirs, %d bytes in %s\n",sfiles,sdirs,sbytes,source);*/

		*files =  sfiles;
		*bytes += sbytes;
		*dirs =  sdirs;
		*dirs +=  1;
	} else
		*files =  1;
	*bytes += s.st_size;
	/* printf("end: %d files, %d dirs, %d bytes in %s\n",*files,*dirs,*bytes,source);*/
}

/* returns 1 if source is a directory */
int fs_isdir(const char *source)
{
	struct stat s;

	if (-1 == stat(source, &s))
		return 0;
	if (S_ISDIR(s.st_mode)) {
		return 1;
	}
	return 0;
}

/* approximate the heaviness of our source ...*/
void fs_fl_approx(filelist *fl, int *files, int *dirs, int *bytes)
{
	int i;
	int sfiles, sdirs, sbytes;  /* for subdirs */

	*files = 0;
	*dirs = 0;
	*bytes = 0;

	for (i = 0;i < fl->n;i++) {
		fs_approx(fl->p[i].name, &sfiles, &sdirs, &sbytes);
		*files +=  sfiles;
		*dirs +=   sdirs;
		*bytes +=  sbytes;
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
	if (-1 == stat(source, &s))
		return(0);
	switch (s.st_mode&S_IFMT) {
	case S_IFDIR:
		fl = fl_new(source);

		printf("mkdir %s\n", dest);
		mkdir(dest, 0777);

		fs_fl_copy(fl, dest);

		fl_del(fl);
		break;
	case S_IFIFO:
		printf("link the fifo\n");

		link(source, dest);
		break;
	default:
		printf("fs_copy -> atomic copy\n");
		printf("open source...");
		if (NULL == (fps = fopen(source, "rb"))) {
			fs_error("fs_copy():fopen(source)", source);
			return(-1);
		}
		printf("ok\n");
		printf("open dest...");
		if (NULL == (fpd = fopen(dest, "wb"))) {
			fs_error("fs_copy():fopen(source)", source);
			fclose(fps);
			return(-1);
		}
		printf("ok\n");
		/* TODO: overwrite protection etc */
		printf("copy ...");

		while (!feof(fps)) {
			printf(".");
			errno = 0;
			n = fread(buf, 1, 1024, fps);
			if (errno) fs_error("fs_copy():fread(source)", source);
			fwrite(buf, 1, n, fpd);
			if (errno) fs_error("fs_copy():fwrite(source)", source);
		}
		printf("ok\n");
		fclose(fps);
		fclose(fpd);

	}
	return(0);
}
/* copy the source to the destination, destination should be a directory. */
int fs_fl_copy(filelist *fl, const char *dest)
{
	int i;
	int r;
	char *sdest;
	char *bname;
	r = 0;
	for (i = 0;i < fl->n;i++) {
		fl->p[i].state = STATE_INUSE;
		bname = basename(fl->p[i].name);
		sdest = (char*)malloc(strlen(dest) + strlen(bname) + 2);

		strcpy(sdest, dest);
		strcat(sdest, "/");
		strcat(sdest, bname);
		r |= fs_copy(fl->p[i].name, sdest);

		free(sdest);
		fl->p[i].state = STATE_FINISHED;
	}

	return(r);
}
/* recursively unlink a dir or file. */
int fs_unlink(char *dest)
{
	filelist *fl;
	struct stat s;

	if (-1 == stat(dest, &s))  return(-1);
	if ((s.st_mode&S_IFMT) == S_IFDIR) {
		printf("%s is a dir, removing below ...\n", dest);
		fl = fl_new(dest);
		if (fs_fl_unlink(fl)) {
			fl_del(fl);
			return(-1);
		} else {
			fl_del(fl);
			printf("removing %s\n", dest);
			if (rmdir(dest) == -1) {
				fs_error("fs_fl_unlink(): rmdir()", dest);
				return(-1);
			}
		}
	} else {
		if (unlink(dest) == -1) {
			fs_error("fs_fl_unlink(): unlink()", dest);
			return(-1);
		}
	}
	return(0);
}
/* remove a lot of files */
int fs_fl_unlink(filelist *fl)
{
	int i, r;
	r = 0;
	for (i = 0;i < fl->n;i++) {
		fl->p[i].state = STATE_INUSE;
		printf("-> atomic unlink %s\n", fl->p[i].name);
		r |= fs_unlink(fl->p[i].name);
		fl->p[i].state = STATE_FINISHED;
	}
	return(r);

}
int fs_move(char *source, char *dest)
{
	if (!rename(source, dest)) {
		switch (errno) {
		case EXDEV:
			fs_copy(source, dest);
			fs_unlink(source);
			break;
		default:
			fs_error("fs_move()", dest);
			return(-1); /* can't help it */

		}
	}
	return(0);
}

/* moves the source to the destination */
int fs_fl_move(filelist *fl, const char *dest)
{
	int i;
	int r;
	char *sdest;
	char *bname;
	r = 0;
	for (i = 0;i < fl->n;i++) {
		fl->p[i].state = STATE_INUSE;
		bname = basename(fl->p[i].name);
		sdest = (char*)malloc(strlen(dest) + strlen(bname) + 2);

		strcpy(sdest, dest);
		strcat(sdest, "/");
		strcat(sdest, bname);
		r |= fs_move(fl->p[i].name, sdest);

		free(sdest);
		fl->p[i].state = STATE_FINISHED;
	}

	return(r);
}

/* write an error and wait for a reaction */
int fs_error(const char *message, char *file)
{
	static struct timespec t = {
		0, 100*1000*1000
	};
	fs_err.err = errno;
	fs_err.message = (char*)message;
	fs_err.file = file;
	fs_err.state = ESTATE_RISE;
	printf("[FS ERROR]: %s %s %s", message, file, strerror(errno));
	while (fs_err.state != ESTATE_NONE)
		nanosleep(&t, NULL); /* until situation clear, wait (and don't waste cpu-time) */
	return(0);
}
