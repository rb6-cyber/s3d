/*
 * allocate.c
 *
 * Copyright (C) 2006 	Marek Lindner <lindner_marek@yahoo.de>
 * 						Simon Wunderlich <dotslash@packetmixer.de>
 * 						(probably more people from the batman project)
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* this file is taken from the batman project (www.open-mesh.net/batman)
 * to find heap corruptions... */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocate.h"

#define DEBUG_MALLOC

#define MAGIC_NUMBER 0x12345678

#if defined DEBUG_MALLOC

struct chunkHeader *chunkList = NULL;

struct chunkHeader
{
	struct chunkHeader *next;
	unsigned int length;
	int tag;
	unsigned int magicNumber;
};

struct chunkTrailer
{
	unsigned int magicNumber;
};

void checkIntegrity(void)
{
	struct chunkHeader *walker;
	struct chunkTrailer *chunkTrailer;
	unsigned char *memory;

	for (walker = chunkList; walker != NULL; walker = walker->next)
	{
		if (walker->magicNumber != MAGIC_NUMBER)
		{
			fprintf(stderr, "Invalid magic number in header: %08x, tag = %d\n", walker->magicNumber, walker->tag);
			exit(1);
		}

		memory = (unsigned char *)walker;

		chunkTrailer = (struct chunkTrailer *)(memory + sizeof(struct chunkHeader) + walker->length);

		if (chunkTrailer->magicNumber != MAGIC_NUMBER)
		{
			fprintf(stderr, "Invalid magic number in header: %08x, tag = %d\n", chunkTrailer->magicNumber, walker->tag);
			exit(1);
		}
	}
}

void checkLeak(void)
{
	struct chunkHeader *walker;

	for (walker = chunkList; walker != NULL; walker = walker->next)
		fprintf(stderr, "Memory leak detected, tag = %d\n", walker->tag);
}

void *debugMalloc(unsigned int length, int tag)
{
	unsigned char *memory;
	struct chunkHeader *chunkHeader;
	struct chunkTrailer *chunkTrailer;
	unsigned char *chunk;

// 	printf("sizeof(struct chunkHeader) = %u, sizeof (struct chunkTrailer) = %u\n", sizeof (struct chunkHeader), sizeof (struct chunkTrailer));

	memory = malloc(length + sizeof(struct chunkHeader) + sizeof(struct chunkTrailer));

	if (memory == NULL)
	{
		fprintf(stderr, "Cannot allocate %u bytes, tag = %d\n", (unsigned int)(length + sizeof(struct chunkHeader) + sizeof(struct chunkTrailer)), tag);
		exit(1);
	}

	chunkHeader = (struct chunkHeader *)memory;
	chunk = memory + sizeof(struct chunkHeader);
	chunkTrailer = (struct chunkTrailer *)(memory + sizeof(struct chunkHeader) + length);

	chunkHeader->length = length;
	chunkHeader->tag = tag;
	chunkHeader->magicNumber = MAGIC_NUMBER;

	chunkTrailer->magicNumber = MAGIC_NUMBER;

	chunkHeader->next = chunkList;
	chunkList = chunkHeader;

	return chunk;
}

void *debugRealloc(void *memoryParameter, unsigned int length, int tag)
{
	unsigned char *memory;
	struct chunkHeader *chunkHeader;
	struct chunkTrailer *chunkTrailer;
	unsigned char *result;
	unsigned int copyLength;

	if (memoryParameter) { /* if memoryParameter==NULL, realloc() should work like malloc() !! */
		memory = memoryParameter;
		chunkHeader = (struct chunkHeader *)(memory - sizeof(struct chunkHeader));
	
		if (chunkHeader->magicNumber != MAGIC_NUMBER)
		{
			fprintf(stderr, "Invalid magic number in header: %08x, tag = %d\n", chunkHeader->magicNumber, chunkHeader->tag);
			exit(1);
		}
	
		chunkTrailer = (struct chunkTrailer *)(memory + chunkHeader->length);
	
		if (chunkTrailer->magicNumber != MAGIC_NUMBER)
		{
			fprintf(stderr, "Invalid magic number in header: %08x, tag = %d\n", chunkTrailer->magicNumber, chunkHeader->tag);
			exit(1);
		}
	}
	

	result = debugMalloc(length, tag);
	if (memoryParameter) {
		copyLength = length;

		if (copyLength > chunkHeader->length)
			copyLength = chunkHeader->length;
	
		memcpy(result, memoryParameter, copyLength);
		debugFree(memoryParameter);
	}


	return result;
}

void debugFree(void *memoryParameter)
{
	unsigned char *memory;
	struct chunkHeader *chunkHeader;
	struct chunkTrailer *chunkTrailer;
	struct chunkHeader *walker;
	struct chunkHeader *previous;

	memory = memoryParameter;
	chunkHeader = (struct chunkHeader *)(memory - sizeof(struct chunkHeader));

	if (chunkHeader->magicNumber != MAGIC_NUMBER)
	{
		fprintf(stderr, "Invalid magic number in header: %08x, tag = %d\n", chunkHeader->magicNumber, chunkHeader->tag);
		exit(1);
	}

	previous = NULL;

	for (walker = chunkList; walker != NULL; walker = walker->next)
	{
		if (walker == chunkHeader)
			break;

		previous = walker;
	}

	if (walker == NULL)
	{
		fprintf(stderr, "Double free detected, tag = %d\n", chunkHeader->tag);
		exit(1);
	}

	if (previous == NULL)
		chunkList = walker->next;

	else
		previous->next = walker->next;

	chunkTrailer = (struct chunkTrailer *)(memory + chunkHeader->length);

	if (chunkTrailer->magicNumber != MAGIC_NUMBER)
	{
		fprintf(stderr, "Invalid magic number in header: %08x, tag = %d\n", chunkTrailer->magicNumber, chunkHeader->tag);
		exit(1);
	}

	free(chunkHeader);
}

#else

void checkIntegrity(void)
{
}

void checkLeak(void)
{
}

void *debugMalloc(unsigned int length, int tag)
{
	void *result;

	result = malloc(length);

	if (result == NULL)
	{
		fprintf(stderr, "Cannot allocate %u bytes, tag = %d\n", length, tag);
		exit(1);
	}

	return result;
}

void *debugRealloc(void *memory, unsigned int length, int tag)
{
	void *result;

	result = realloc(memory, length);

	if (result == NULL)
	{
		fprintf(stderr, "Cannot re-allocate %u bytes, tag = %d\n", length, tag);
		exit(1);
	}

	return result;
}

void debugFree(void *memory)
{
	free(memory);
}

#endif