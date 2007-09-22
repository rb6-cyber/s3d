/* http_fetcher.h - HTTP handling functions
 *
 * HTTP Fetcher
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

#ifndef HTTP_FETCHER_H
#define HTTP_FETCHER_H

#include "http_error_codes.h"

#define PORT_NUMBER    80
#define HTTP_VERSION    "HTTP/1.1"
#define DEFAULT_USER_AGENT  "HTTP Fetcher"
#define HTTP_FETCHER_VERSION "1.0"
#define DEFAULT_READ_TIMEOUT 30  /* Seconds to wait before giving up when no data is arriving */

#define REQUEST_BUF_SIZE   1024
#define HEADER_BUF_SIZE   1024
#define DEFAULT_PAGE_BUF_SIZE  1024 * 200 /* 200K should hold most things */



/******************************************************************************/
/**************** Function declarations and descriptions **********************/
/******************************************************************************/

/*
 * [!!! NOTE !!!]  All HTTP Fetcher functions return -1 on error.  You can
 * then either call http_perror to print the error message or call
 * http_strerror to get a pointer to it
 */


/*
 * Download the page, registering a hit. If you pass it a NULL for fileBuf,
 * 'url' will be requested but will not remain in memory (useful for
 * simply registering a hit).  Otherwise necessary space will be allocated
 * and will be pointed to by fileBuf.
 * Returns:
 * # of bytes downloaded, or
 * -1 on error
 */
int http_fetch(const char *url, char **fileBuf);

/*
 * Changes the User Agent (shown to the web server with each request)
 * Send it NULL to avoid telling the server a User Agent
 * By default, the User Agent is sent (The default one unless changed)
 * Returns:
 * 0 on success, or
 * -1 on error (previous value for agent remains unchanged)
 */
int http_setUserAgent(const char *newAgent);

/*
 * Changes the Referer (shown to the web server with each request)
 * Send it NULL to avoid thelling the server a Referer
 * By default, no Referer is sent
 * Returns:
 * 0 on success, or
 * -1 on error
 */
int http_setReferer(const char *newReferer);

/*
 * Changes the maximum amount of time that HTTP Fetcher will wait on
 * data.  If this many seconds elapses without more data from the
 * server, http_fetch will return with an error.
 * If you pass a value less than 0, reads will not time out, potentially
 * waiting forever (or until data shows up, whichever comes first)
 */
void http_setTimeout(int seconds);

/*
 * Activate authentication for the Request. If user or pass is NULL,
 * http_set_Auth assumes a request to cleanup (disable Authentication).
 * Returns 0 on success, and -1 on error or cleanup.
 */

int http_setAuth(const char *user, const char *pass);

/*
 * Takes a url and puts the filename portion of it into 'filename'.
 * Returns:
 * 0 on success, or
 * 1 when url contains no end filename (i.e., "www.foo.com/")
 *  and **filename should not be assumed to point to anything), or
 * -1 on error
 */
int http_parseFilename(const char *url, char **filename);

/*
 * Works like perror.  If an HTTP Fetcher function ever returns an
 * error (-1), this will print a descriptive message to standard output
 */
void http_perror(const char *string);

/*
 * Returns a pointer to the current error description message.  The
 * message pointed to is only good until the next call to http_strerror(),
 * so if you need to hold on to the message for a while you should make
 * a copy of it.
 */
const char *http_strerror();



/******************************************************************************/
/**** The following functions are used INTERNALLY by http_fetcher *************/
/******************************************************************************/

/*
 * Reads the metadata of an HTTP response.  On success returns the number
 * Returns:
 * # of bytes read on success, or
 * -1 on error
 */
int _http_read_header(int sock, char *headerPtr);

/*
 * Opens a TCP socket and returns the descriptor
 * Returns:
 * socket descriptor, or
 * -1 on error
 */
int makeSocket(const char *host);

#endif
