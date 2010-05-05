/*
 * testvis.c
 *
 * Copyright (C) 2007  Andreas Langer <andreas_lbg@gmx.de>
 *
 * This file is part of testvis, a simple dot drawing application.
 * See http://s3d.berlios.de/ for more updates.
 *
 * testvis is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * testvis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with testvis; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <config-s3d.h>

#define BUFFER_SIZE 1000
#define LINE_SIZE 100

#ifndef VISUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define VISUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define VISUNUSED(x) /* x */
#else
#define VISUNUSED(x) x
#endif
#endif

struct data {
	int index;
	int active;
	char line[LINE_SIZE];
	struct data *next;
};

struct t_data {
	struct data *head, *end;
};


static void list_data(struct data *head, struct data *end)
{
	struct data *tmp;

	for (tmp = head->next; tmp != end; tmp = tmp->next) {
		printf("%d: %s | %d\n", tmp->index, tmp->line, tmp->active);
	}
}

static void rem_data(int index, struct data *head, struct data *end)
{
	struct data *tmp, *prev = head;

	for (tmp = head->next; tmp != end; prev = tmp, tmp = tmp->next) {
		if (tmp->index == index)
			break;
	}

	if (tmp != end) {

		prev->next = tmp->next;

		printf("remove index %d\n", tmp->index);
		free(tmp);
	} else {
		printf("index not found\n");
	}
	return;
}

static void dea_data(int index, struct data *head, struct data *end)
{
	struct data *tmp;

	for (tmp = head->next; tmp != end; tmp = tmp->next) {
		if (tmp->index == index)
			break;
	}

	if (tmp != end && tmp != head) {
		tmp->active = 0;
	}
	return;
}

static void act_data(int index, struct data *head, struct data *end)
{
	struct data *tmp;

	for (tmp = head->next; tmp != end; tmp = tmp->next) {
		if (tmp->index == index)
			break;
	}

	if (tmp != end && tmp != head) {
		tmp->active = 1;
	}
	return;
}

static void sig(int VISUNUSED(signr))
{
	return;
}

static void* server(void *args)
{
	struct t_data *t = (struct t_data*)args;
	int listen_fd, yes = 1;
	struct sockaddr_in sock, client;
	struct data *tmp;

	int sock2;
	socklen_t len;

	char buffer[2000];
	char start[] = "digraph topology\n{\n";
	char end[] = "}\n";
	int index;

	signal(SIGPIPE, sig);

	listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	memset((char *) &sock, 0, sizeof(sock));
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = htonl(INADDR_ANY);
	sock.sin_port = htons(2004);

	bind(listen_fd, (struct sockaddr *) &sock, sizeof(sock));
	listen(listen_fd, 1);

	len = sizeof(client);

	while (1) {

		sock2 = accept(listen_fd, (struct sockaddr*) & client, &len);

		while (1) {

			memset(buffer, 0, 2000);
			strcat(buffer, start);
			index = strlen(start);

			for (tmp = t->head->next; tmp != t->end; tmp = tmp->next) {

				if (!tmp->active)
					continue;
				strcat(&buffer[index], tmp->line);
				index += strlen(tmp->line);
				strcat(&buffer[index], "\n");
				index++;

			}

			strcat(&buffer[index], end);
			buffer[index+2] = 0;
			if (send(sock2, buffer, strlen(buffer), 0) < 1)
				break;

			sleep(3);

		}
	}

	return NULL;
}

int main(void)
{
	char buffer[BUFFER_SIZE];
	char *tmp_buffer;
	struct data *head, *z, *t;
	static int index = 0;
	struct t_data t_dat;
	pthread_t thread;

	head = (struct data *)malloc(sizeof(*head));
	z = (struct data *)malloc(sizeof(*z));

	head->next = z;

	t_dat.head = head;
	t_dat.end = z;
	pthread_create(&thread, NULL, server, &t_dat);
	pthread_detach(thread);
	printf("\ntestivs: ");

	while (fgets(buffer, BUFFER_SIZE, stdin)) {

		if ((tmp_buffer = strstr(buffer, "add")) != NULL) {
			tmp_buffer[strlen(tmp_buffer) - 1] = 0;
			tmp_buffer += 4;
			t = (struct data *)malloc(sizeof(*t));
			strncpy(t->line, tmp_buffer, LINE_SIZE);
			t->line[LINE_SIZE - 1] = 0; /* make sure it's terminated */
			t->index = ++index;
			t->active = 1;
			t->next = head->next;
			head->next = t;
		} else if (strstr(buffer, "list") != NULL) {

			list_data(head, z);

		} else if ((tmp_buffer = strstr(buffer, "rem")) != NULL) {

			tmp_buffer[strlen(tmp_buffer) - 1] = 0;
			tmp_buffer += 4;
			rem_data(atoi(tmp_buffer), head, z);

		} else if ((tmp_buffer = strstr(buffer, "dea")) != NULL) {

			tmp_buffer[strlen(tmp_buffer) - 1] = 0;
			tmp_buffer += 4;
			dea_data(atoi(tmp_buffer), head, z);

		} else if ((tmp_buffer = strstr(buffer, "act")) != NULL) {

			tmp_buffer[strlen(tmp_buffer) - 1] = 0;
			tmp_buffer += 4;
			act_data(atoi(tmp_buffer), head, z);

		} else if (strstr(buffer, "quit") != NULL) {

			break;

		} else

			printf("command not found\n");

		printf("testivs: ");

	}

	return 0;
}
