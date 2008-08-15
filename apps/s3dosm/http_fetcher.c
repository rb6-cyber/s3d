/* http_fetcher.c - HTTP handling functions
 *
 * Copyright (C) 2001 Lyle Hanson (lhanson@cs.nmu.edu)
 *
 * This file is part of s3dosm, a gps card application for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3dosm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3dosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3dosm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Changes:
 *  Simon Wunderlich <dotslash@packetmixer.de>
 *  + added http_setAuth() to support basic http-authentication and some minor fixes
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include "http_fetcher.h"

/* Globals */
static int timeout = DEFAULT_READ_TIMEOUT;
static char *userAgent = NULL;
static char *referer = NULL;
static char *auth = NULL;
static int hideUserAgent = 0;
static int hideReferer = 1;
static int errorSource = 0;
static int http_errno = 0;
static int errorInt = 0;   /* When the error message has a %d in it,
          * this variable is inserted */
static int freeOldAgent = 0; /* Indicates previous malloc's */
static int freeOldReferer = 0; /* Indicated previous malloc's */


/*
 * Actually downloads the page, registering a hit (donation)
 * If the fileBuf passed in is NULL, the url is downloaded and then
 * freed; otherwise the necessary space is allocated for fileBuf.
 * Returns size of download on success, -1 on error is set,
 */
int http_fetch(const char *url_tmp, char **fileBuf)
{
	fd_set rfds;
	struct timeval tv;
	char requestBuf[REQUEST_BUF_SIZE];
	char headerBuf[HEADER_BUF_SIZE];
	char *url, *pageBuf, *host, *charIndex;
	int sock, bytesRead = 0, contentLength = -1;
	int ret = -1, i, selectRet;


	if (url_tmp == NULL) {
		errorSource = FETCHER_ERROR;
		http_errno = HF_NULLURL;
		return -1;
	}

	/* Copy the url passed in into a buffer we can work with, change, etc. */
	url = (char*)malloc(strlen(url_tmp) + 1);
	if (url == NULL) {
		errorSource = ERRNO;
		return -1;
	}
	strncpy(url, url_tmp, strlen(url_tmp) + 1);

	/* Seek to the file path portion of the url */
	charIndex = strstr(url, "://");
	if (charIndex != NULL) {
		/* url contains a protocol field */
		charIndex += strlen("://");
		host = charIndex;
		charIndex = strchr(charIndex, '/');
	} else {
		host = (char *)url;
		charIndex = strchr(url, '/');
	}

	/* Compose a request string */
	if (charIndex == NULL)
		/* The url has no '/' in it, assume the user is making a root-level
		 * request */
		sprintf(requestBuf, "GET / %s\r\n", HTTP_VERSION);
	else
		sprintf(requestBuf, "GET %s %s\r\n", charIndex, HTTP_VERSION);
	/* Null out the end of the hostname if need be */
	if (charIndex != NULL)
		*charIndex = 0;

	strcat(requestBuf, "Host: ");
	strcat(requestBuf, host);
	strcat(requestBuf, "\r\n");


	if (!hideReferer && referer != NULL) { /* NO default referer */
		strcat(requestBuf, "Referer: ");
		strcat(requestBuf, referer);
		strcat(requestBuf, "\r\n");
	}

	if (!hideUserAgent && userAgent == NULL) {
		strcat(requestBuf, "User-Agent: ");
		strcat(requestBuf, DEFAULT_USER_AGENT);
		strcat(requestBuf, "/");
		strcat(requestBuf, HTTP_FETCHER_VERSION);
		strcat(requestBuf, "\r\n");
	} else if (!hideUserAgent) {
		strcat(requestBuf, "User-Agent: ");
		strcat(requestBuf, userAgent);
		strcat(requestBuf, "\r\n");
	}
	if (auth != NULL) {
		strcat(requestBuf, "Authorization: Basic ");
		strcat(requestBuf, auth);
		strcat(requestBuf, "\r\n");
	}
	strcat(requestBuf, "\r\n");

	printf("[HTTP] creating connection ...\n");
	sock = makeSocket(host);   /* errorSource set within makeSocket */
	if (sock == -1) {
		free(url);
		return -1;
	}
	printf("[HTTP] sending request \n");
	if (write(sock, requestBuf, strlen(requestBuf)) == -1) {
		close(sock);
		free(url);
		errorSource = ERRNO;
		return -1;
	}

	printf("[HTTP] receiving header\n");
	/* Grab enough of the response to get the metadata */
	ret = _http_read_header(sock, headerBuf); /* errorSource set within */
	if (ret < 0) {
		close(sock);
		free(url);
		return -1;
	}
	printf("[HTTP] receiving content\n");

	/* Get the return code */
	charIndex = strstr(headerBuf, "HTTP/");
	if (charIndex == NULL) {
		close(sock);
		free(url);
		errorSource = FETCHER_ERROR;
		http_errno = HF_FRETURNCODE;
		return -1;
	}
	while (*charIndex != ' ')
		charIndex++;
	charIndex++;

	ret = sscanf(charIndex, "%i", &i);
	if (ret != 1) {
		close(sock);
		free(url);
		errorSource = FETCHER_ERROR;
		http_errno = HF_CRETURNCODE;
		return -1;
	}
	if (i < 200 || i > 299) {
		close(sock);
		free(url);
		errorInt = i; /* Status code, to be inserted in error string */
		errorSource = FETCHER_ERROR;
		http_errno = HF_STATUSCODE;
		return -1;
	}

	/*
	 * Parse out about how big the data segment is.
	 * Note that under current HTTP standards (1.1 and prior), the
	 * Content-Length field is not guaranteed to be accurate or even present.
	 * I just use it here so I can allocate a ballpark amount of memory.
	 *
	 * Note that some servers use different capitalization
	 */
	charIndex = strstr(headerBuf, "Content-Length:");
	if (charIndex == NULL)
		charIndex = strstr(headerBuf, "Content-length:");

	if (charIndex != NULL) {
		ret = sscanf(charIndex + strlen("content-length: "), "%i",
		             &contentLength);
		if (ret < 1) {
			close(sock);
			free(url);
			errorSource = FETCHER_ERROR;
			http_errno = HF_CONTENTLEN;
			return -1;
		}
	}

	/* Allocate enough memory to hold the page */
	if (contentLength == -1)
		contentLength = DEFAULT_PAGE_BUF_SIZE;

	pageBuf = (char *)malloc(contentLength + 1);
	if (pageBuf == NULL) {
		close(sock);
		free(url);
		errorSource = ERRNO;
		return -1;
	}

	/* Begin reading the body of the file */
	while (ret > 0) {
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		if (timeout >= 0)
			selectRet = select(sock + 1, &rfds, NULL, NULL, &tv);
		else  /* No timeout, can block indefinately */
			selectRet = select(sock + 1, &rfds, NULL, NULL, NULL);

		if (selectRet == 0 && timeout < 0) {
			errorSource = FETCHER_ERROR;
			http_errno = HF_DATATIMEOUT;
			errorInt = timeout;
			close(sock);
			free(url);
			free(pageBuf);
			return -1;
		} else if (selectRet == -1) {
			close(sock);
			free(url);
			free(pageBuf);
			errorSource = ERRNO;
			return -1;
		}

		ret = read(sock, pageBuf + bytesRead, contentLength);
		if (ret == -1) {
			close(sock);
			free(url);
			free(pageBuf);
			errorSource = ERRNO;
			return -1;
		}

		bytesRead += ret;

		if (ret > 0) {
			/* To be tolerant of inaccurate Content-Length fields, we'll
			 * allocate another read-sized chunk to make sure we have
			 * enough room.
			 */
			pageBuf = (char *)realloc(pageBuf, bytesRead + contentLength);
			if (pageBuf == NULL) {
				close(sock);
				free(url);
				free(pageBuf);
				errorSource = ERRNO;
				return -1;
			}
		}
		printf("[HTTP] read %d bytes\n", ret);
	}

	/*
	 * The download buffer is too large.  Trim off the safety padding.
	 */
	pageBuf = (char *)realloc(pageBuf, bytesRead);
	/* pageBuf shouldn't be null, since we're _shrinking_ the buffer,
	 * and if it DID fail, we could go on with the too-large buffer,
	 * but something would DEFINATELY be wrong, so we'll just give
	 * an error message */
	if (pageBuf == NULL) {
		close(sock);
		free(url);
		free(pageBuf);
		errorSource = ERRNO;
		return -1;
	}

	if (fileBuf == NULL) /* They just wanted us to "hit" the url */
		free(pageBuf);
	else
		*fileBuf = pageBuf;

	close(sock);
	free(url);
	return bytesRead;
}



/*
 * Changes the User Agent.  Returns 0 on success, -1 on error.
 */
int http_setUserAgent(const char *newAgent)
{
	char *tmp;

	if (newAgent == NULL) {
		if (freeOldAgent) free(userAgent);
		userAgent = NULL;
		hideUserAgent = 1;
	} else {
		tmp = (char *)malloc(strlen(newAgent));
		if (tmp == NULL) {
			errorSource = ERRNO;
			return -1;
		}
		if (freeOldAgent) free(userAgent);
		userAgent = tmp;
		strcpy(userAgent, newAgent);
		freeOldAgent = 1;
		hideUserAgent = 0;
	}

	return 0;
}



/*
 * Changes the Referer.  Returns 0 on success, -1 on error
 */
int http_setReferer(const char *newReferer)
{
	char *tmp;

	if (newReferer == NULL) {
		if (freeOldReferer) free(referer);
		referer = NULL;
		hideReferer = 1;
	} else {
		tmp = (char *)malloc(strlen(newReferer));
		if (tmp == NULL) {
			errorSource = ERRNO;
			return -1;
		}
		if (freeOldReferer) free(referer);
		referer = tmp;
		strcpy(referer, newReferer);
		freeOldReferer = 1;
		hideReferer = 0;
	}

	return 0;
}
int http_setAuth(const char *user, const char *pass)
{
	unsigned char plain[1024];
	char ec64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char *b64;
	int i, j, c, len;
	char o = 0;
	/* base64 encode user and pass */
	if ((user == NULL) || (pass == NULL)) { /* bad input or request to clean up */
		if (auth != NULL) free(auth); /* free old auth */
		auth = NULL;
		return(-1);
	}

	snprintf((char *)plain, 1024, "%s:%s", user, pass);
	len = strlen((char *)plain);
	b64 = (char*)malloc(len * 4 + 1);
	i = j = c = 0;
	while (i < len || c != 0) {
		switch (c) {
		case 0:
			o = ec64[ plain[i] >> 2 ];
			i++;
			break;
		case 1:
			o = ec64[((plain[i-1] & 0x3) << 4) | (plain[i] >> 4)];
			i++;
			break;
		case 2:
			o = (i >= len) ? '=' : ec64[((plain[i-1] & 0xf) << 2) | (plain[i] >> 6)];
			break;
		case 3:
			o = (i >= len) ? '=' : ec64[ plain[i] & 0x3f];
			i++;
			break;
		}
		b64[j] = o;
		c = (c + 1) % 4;
		j++;
	}
	b64[j] = 0;
	if (auth != NULL) free(auth); /* free old auth */
	auth = b64;
	return(0);


}




/*
 * Changes the amount of time that HTTP Fetcher will wait for data
 * before timing out on reads
 */
void http_setTimeout(int seconds)
{
	timeout = seconds;
}



/*
 * Puts the filename portion of the url into 'filename'.
 * Returns:
 * 0 on success
 * 1 when url contains no end filename (i.e., 'www.foo.com/'),
 *  and **filename should not be assumed to be valid
 * -1 on error
 */
int http_parseFilename(const char *url, char **filename)
{
	char *ptr;

	if (url == NULL) {
		errorSource = FETCHER_ERROR;
		http_errno = HF_NULLURL;
		return -1;
	}

	ptr = (char *)rindex(url, '/');
	if (ptr == NULL)
		/* Root level request, apparently */
		return 1;

	ptr++;
	if (*ptr == '\0') return 1;

	*filename = (char *)malloc(strlen(ptr));
	if (*filename == NULL) {
		errorSource = ERRNO;
		return -1;
	}
	strcpy(*filename, ptr);

	return 0;
}



/* Depending on the source of error, calls either perror() or prints
 * an HTTP Fetcher error message to stdout */
void http_perror(const char *string)
{
	if (errorSource == ERRNO)
		perror(string);
	else if (errorSource == H_ERRNO)
		herror(string);
	else if (errorSource == FETCHER_ERROR) {
		char *stringIndex;

		if (strstr(http_errlist[http_errno], "%d") == NULL) {
			fputs(string, stderr);
			fputs(": ", stderr);
			fputs(http_errlist[http_errno], stderr);
			fputs("\n", stderr);
		} else {
			/* The error string has a %d in it, we need to insert errorInt */
			stringIndex = (char *)http_errlist[http_errno];
			while (*stringIndex != '%') {  /* Print up to the %d */
				fputc(*stringIndex, stderr);
				stringIndex++;
			}
			fprintf(stderr, "%d", errorInt); /* Print the number */
			stringIndex += 2;     /* Skip past the %d */
			while (*stringIndex != 0) {  /* Print up to the end NULL */
				fputc(*stringIndex, stderr);
				stringIndex++;
			}
			fputs("\n", stderr);
		}
	}
}



/*
 * Returns a pointer to the current error description message. The
 * message pointed to is only good until the next call to http_strerror(),
 * so if you need to hold on to the message for a while you should make
 * a copy of it
 */
const char *http_strerror(void)
{
	if (errorSource == ERRNO)
		return strerror(errno);
	else if (errorSource == H_ERRNO)
		return hstrerror(h_errno);
	else if (errorSource == FETCHER_ERROR) {
		if (strstr(http_errlist[http_errno], "%d") == NULL)
			return http_errlist[http_errno];
		else {
			/* The error string has a %d in it, we need to insert errorInt.
			 * convertedError[128] has been declared for that purpose */
			char *stringIndex, *originalError;

			originalError = (char *)http_errlist[http_errno];
			convertedError[0] = 0;  /* Start off with NULL */
			stringIndex = strstr(originalError, "%d");
			strncat(convertedError, originalError,  /* Copy up to %d */
			        abs(stringIndex - originalError));
			sprintf(&convertedError[strlen(convertedError)], "%d", errorInt);
			stringIndex += 2;  /* Skip past the %d */
			strcat(convertedError, stringIndex);

			return convertedError;
		}
	}

	return http_errlist[HF_METAERROR]; /* Should NEVER happen */
}


/*
 * Reads the metadata of an HTTP response.
 * Perhaps a little inefficient, as it reads 1 byte at a time, but
 * I don't think it's that much of a loss (most headers aren't HUGE).
 * Returns:
 * # of bytes read on success, or
 * -1 on error
 */
int _http_read_header(int sock, char *headerPtr)
{
	fd_set rfds;
	struct timeval tv;
	int bytesRead = 0, newlines = 0, ret, selectRet;

	while (newlines != 2 && bytesRead != HEADER_BUF_SIZE) {
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		if (timeout >= 0)
			selectRet = select(sock + 1, &rfds, NULL, NULL, &tv);
		else  /* No timeout, can block indefinately */
			selectRet = select(sock + 1, &rfds, NULL, NULL, NULL);

		if (selectRet == 0 && timeout < 0) {
			errorSource = FETCHER_ERROR;
			http_errno = HF_HEADTIMEOUT;
			errorInt = timeout;
			return -1;
		} else if (selectRet == -1) {
			errorSource = ERRNO;
			return -1;
		}

		ret = read(sock, headerPtr, 1);
		if (ret == -1) {
			errorSource = ERRNO;
			return -1;
		}
		bytesRead++;

		if (*headerPtr == '\r') {  /* Ignore CR */
			/* Basically do nothing special, just don't set newlines
			 * to 0 */
			headerPtr++;
			continue;
		} else if (*headerPtr == '\n')  /* LF is the separator */
			newlines++;
		else
			newlines = 0;

		headerPtr++;
	}

	headerPtr -= 3;  /* Snip the trailing LF's */
	*headerPtr = '\0';
	return bytesRead;
}



/*
 * Opens a TCP socket and returns the descriptor
 * Returns:
 * socket descriptor, or
 * -1 on error
 */
int makeSocket(const char *host)
{
	int sock;          /* Socket descriptor */
	struct sockaddr_in sa;       /* Socket address */
	struct hostent *hp;        /* Host entity */
	int ret;

	hp = gethostbyname(host);
	if (hp == NULL) {
		errorSource = H_ERRNO;
		return -1;
	}

	/* Copy host address from hostent to (server) socket address */
	memcpy((char *)&sa.sin_addr, (char *)hp->h_addr, hp->h_length);
	sa.sin_family = hp->h_addrtype;  /* Set service sin_family to PF_INET */
	sa.sin_port = htons(PORT_NUMBER); /* Put portnum into sockaddr */

	sock = socket(hp->h_addrtype, SOCK_STREAM, 0);
	if (sock == -1) {
		errorSource = ERRNO;
		return -1;
	}

	ret = connect(sock, (struct sockaddr *) & sa, sizeof(sa));
	if (ret == -1) {
		errorSource = ERRNO;
		return -1;
	}

	return sock;
}
