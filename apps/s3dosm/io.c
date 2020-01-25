// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include "s3dosm.h"
#include <stdio.h>   /* fopen(), fread(), fclose() */
#include <stdlib.h>   /* malloc(), free() */
#include <string.h>   /* strstr() */
#include <sys/stat.h>  /* fstat() */
#include <getopt.h>   /* getopt() */
#include <errno.h>   /* errno */
#include <s3d.h>   /* s3d_usage() */

char *read_file(const char *fname, int *fsize)
{
	FILE *fp;
	char *buf = NULL;
	int filesize;
	struct stat bf;

	if ((fp = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "read_file( %s ):fopen(): %s", fname, strerror(errno));
		return NULL;
	}
	if (fstat(fileno(fp), &bf))    {
		fprintf(stderr, "read_file( %s ):fopen(): %s", fname, strerror(errno));
		fclose(fp);
		return NULL;
	}
	filesize = bf.st_size;
	if ((buf = (char*)malloc(filesize)) == NULL)  {
		fprintf(stderr, "read_file( %s ):malloc(): %s", fname, strerror(errno));
		fclose(fp);
		return NULL;
	}
	fread(buf, filesize, 1, fp);
	fclose(fp);
	if (fsize != NULL) *fsize = filesize;
	return buf;
}

int process_args(int argc, char **argv)
{
	int      lopt_idx = 0, i;
	int      c;
	float    minlat, minlon, maxlat, maxlon;
	char     info[1024];
	struct option long_options[] = {
		{
			"help", 0, NULL, 'h'
		}, {"osm", 1, NULL, 'o'}, {NULL, 0, NULL, 0}
	};
	optind = 0;
	opterr = 0;
	while (-1 != (c = getopt_long(argc, argv, "dH:?ho:", long_options, &lopt_idx))) {
		switch (c) {
		case 0:
			break;
		case 'o':
			if (4 == sscanf(optarg, "%f,%f,%f,%f", &minlat, &minlon, &maxlat, &maxlon))
				layerset_add(load_osm_web(minlat, minlon, maxlat, maxlon));
			else {
				printf("%s: bad map bounding box", optarg);
				return -1;
			}
			break;
		case 'h':
		case '?':
			printf("\nUSAGE: %s [options] [files]\n\n", argv[0]);
			printf("options:\n");
			printf("\t--osm, -o MINLAT,MINLON,MAXLAT,MAXLON:\n");
			printf("\t\tload a map with the given bounding box\n\t\tfrom the openstreetmap server\n");
			printf("\t--help, -?, -h: this helpful text\n\n");
			printf("supported file types:\n");
			printf("\t+ Kismet .xml Logs\n");
			printf("\t+ Opemstreetmap .osm files\n\n");
			s3d_usage(); /* add s3d usage */
			return -1;
		default:
			break;
		}
	}
	for (i = 1; i < argc; i++) {
		if (strstr(argv[i], ".osm") - argv[i] == (signed)(strlen(argv[i]) - 4)) {
			snprintf(info, 1024, "loading OSM-File: %s", argv[i]);
			load_window(info);
			layerset_add(load_osm_file(argv[i]));
		} else if (strstr(argv[i], ".xml") - argv[i] == (signed)(strlen(argv[i]) - 4)) { /* might be osm or kismet xml */
			char *file;
			int fsize;
			if (NULL == (file = read_file(argv[i], &fsize)))
				break;
			if (NULL != strstr(file, "<!DOCTYPE detection-run SYSTEM \"http://kismetwireless.net")) {
				snprintf(info, 1024, "loading Kismet-File: %s", argv[i]);
				load_window(info);
				layerset_add(parse_kismet(file, fsize));
			} else if (NULL != strstr(file, "<osm ")) {
				snprintf(info, 1024, "loading OSM-File: %s", argv[i]);
				load_window(info);
				layerset_add(parse_osm(file, fsize));
			}
			free(file);
		}
	}
	load_window_remove();
	return 0;
}

