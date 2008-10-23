/*
 * proto_in.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d API, the API of s3d (the 3d network display server).
 * See http://s3d.berlios.de/ for more updates.
 *
 * The s3d API is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * The s3d API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d API; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "s3d.h"
#include "s3dlib.h"
#include <proto.h>
#include <netinet/in.h>  /*  htons(),htonl() */
#include <errno.h>   /*  errno */
#include <stdlib.h>   /*  malloc(), free() */
#include <inttypes.h>

/*  this proccesses the commands and pushes s3d-events, or does other things ;) */
int net_prot_in(uint8_t opcode, uint16_t length, char *buf)
{
	uint32_t oid = ~0u;
	struct s3d_evt *s3devt = NULL;
	struct mcp_object *mo;
	struct s3d_obj_info *oi;
	struct s3d_texshm *tshm;
	switch (opcode) {
	case S3D_P_S_INIT:
		s3dprintf(MED, "S3D_P_S_INIT: init!!");
		_s3d_ready = 1;
		break;
	case S3D_P_S_QUIT:
		s3dprintf(MED, "S3D_P_S_QUIT: server wants us to go. well ...");
		s3d_quit();
		break;
	case S3D_P_S_CLICK:
		if (length == 4) {
			oid = ntohl(*((uint32_t *)buf));
			if (NULL != (s3devt = (struct s3d_evt*)malloc(sizeof(struct s3d_evt)))) {
				*((uint32_t *)buf) = oid;  /*  reuse buffer ... */
				s3devt->event = S3D_EVENT_OBJ_CLICK;
				s3devt->length = 4;
				s3devt->buf = buf;
			}
			s3dprintf(MED, "S3D_P_S_CLICK: %d got clicked ....", oid);
		}
		break;
	case S3D_P_S_NEWOBJ:
		if (length == 4) {
			oid = ntohl(*((uint32_t *)buf));
			_queue_new_object(oid);
			/*
			if (NULL!=(s3devt=malloc(sizeof(struct s3d_evt))))
			{
			 *((uint32_t *)buf)=oid;  / *  reuse buffer ... * /
			 s3devt->event=S3D_EVENT_NEW_OBJECT;
			 s3devt->length=4;
			 s3devt->buf=buf;
			 _queue_new_object(*((unsigned int *)newevt->buf));
			}*/
			s3dprintf(VLOW, "S3D_P_S_NEWOBJ: new object %d", oid);
		}
		break;
	case S3D_P_S_KEY:
		if (length == 8) {
			if (NULL != (s3devt = (struct s3d_evt*)malloc(sizeof(struct s3d_evt)))) {
				struct s3d_key_event *keyevent;
				s3devt->length = 2;
				keyevent = (struct s3d_key_event *)buf;
				keyevent->keysym = ntohs(keyevent->keysym);
				keyevent->unicode = ntohs(keyevent->unicode);
				keyevent->modifier = ntohs(keyevent->modifier);
				keyevent->state = ntohs(keyevent->state);
				s3devt->buf = buf;
				s3devt->event = (keyevent->state == 0) ? S3D_EVENT_KEYDOWN : S3D_EVENT_KEYUP;

				s3dprintf(VLOW, "S3D_P_S_KEY: key %d hit!!", *((uint16_t *)s3devt->buf));
			}
		}
		break;
	case S3D_P_S_MBUTTON:
		if (length == 2) {
			if (NULL != (s3devt = (struct s3d_evt*)malloc(sizeof(struct s3d_evt)))) {
				s3devt->event = S3D_EVENT_MBUTTON;
				s3devt->length = 2;
				s3devt->buf = buf;
				s3dprintf(VLOW, "S3D_P_S_MBUTTON: mbutton %d, state %d !!", *((uint8_t *)s3devt->buf), *(1 + (uint8_t *)s3devt->buf));
			}
		}
		break;
	case S3D_P_MCP_OBJECT:
		if (length == sizeof(struct mcp_object)) {
			/*     oid=htonl(*((uint32_t *)buf)); */
			if (NULL != (s3devt = (struct s3d_evt*)malloc(sizeof(struct s3d_evt)))) {
				/*      *((uint32_t *)buf)=oid;  / *  reuse buffer ... * / */
				s3devt->event = S3D_MCP_OBJECT;
				s3devt->length = length;
				mo = (struct mcp_object *)buf;
				*((uint32_t *)buf) = ntohl(*((uint32_t *)buf));  /*  revert oid */
				ntohfb(&mo->trans_x, 4);

				buf[length-1] = '\0';  /*  put a null byte at the end  */
				/*  for the not so careful users */
				s3devt->buf = buf;
				s3dprintf(VLOW, "S3D_P_MCP_OBEJCT: something is happening to object %d, name %s",  mo->object, mo->name);

			}
		} else s3dprintf(MED, "wrong length for S3D_P_MCP_OBJECT length %"PRId16" != %d", length, (int)sizeof(struct mcp_object));
		break;
	case S3D_P_S_OINFO:
		if (length == sizeof(struct s3d_obj_info)) {
			/*     oid=htonl(*((uint32_t *)buf)); */
			if (NULL != (s3devt = (struct s3d_evt*)malloc(sizeof(struct s3d_evt)))) {
				/*      *((uint32_t *)buf)=oid;  / *  reuse buffer ... * / */
				s3devt->event = S3D_EVENT_OBJ_INFO;
				s3devt->length = length;
				oi = (struct s3d_obj_info *)buf;
				oi->object = ntohl(oi->object);
				oi->flags = ntohl(oi->flags);
				ntohfb(&oi->trans_x, 8);

				buf[length-1] = '\0';  /*  put a null byte at the end  */
				/*  for the not so careful users */
				s3devt->buf = buf;
				s3dprintf(VLOW, "S3D_P_S_OINFO: something is happening to object %d, name %s",
				          oi->object,
				          oi->name
				         );

			}
		} else s3dprintf(MED, "wrong length for S3D_P_S_OINFO length %"PRId16" != %d", length, (int)sizeof(struct s3d_obj_info));
		break;
	case S3D_P_S_SHMTEX:
		if (length == sizeof(struct s3d_texshm)) {
			tshm = (struct s3d_texshm *)buf;

			/* this is only handled internally ... */
			tshm->oid = ntohl(tshm->oid);
			tshm->tex = ntohl(tshm->tex);
			tshm->shmid = ntohl(tshm->shmid);
			tshm->tw = ntohs(tshm->tw);
			tshm->th = ntohs(tshm->th);
			tshm->w = ntohs(tshm->w);
			tshm->h = ntohs(tshm->h);

			s3dprintf(MED, "S3D_P_S_SHMTEX: texture %d of object %d is available under shmid %d",
			          tshm->tex, tshm->oid, tshm->shmid);
			_s3d_handle_texshm(tshm);
			free(buf);

		} else
			s3dprintf(MED, "wrong length for S3D_P_S_SHMTEX length %"PRId16" != %d", length, (int)sizeof(struct s3d_texshm));
		break;


	case S3D_P_MCP_DEL_OBJECT:
		if (length == 4) {
			if (NULL != (s3devt = (struct s3d_evt*)malloc(sizeof(struct s3d_evt)))) {
				s3devt->event = S3D_MCP_DEL_OBJECT;
				s3devt->length = length;
				*((uint32_t *)buf) = ntohl(*((uint32_t *)buf));  /*  revert oid */
				s3dprintf(MED, "S3D_P_MCP_DEL_OBEJCT: deleting object %d", *((uint32_t *)buf));
				s3devt->buf = buf;
			}
		}
		break;
	default:
		s3dprintf(MED, "don't know command %d", opcode);
		if (buf != NULL) free(buf);
	}
	if (s3devt != NULL) {

		s3d_push_event(s3devt);
	}
	return(0);
}

