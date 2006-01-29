#include "global.h"
#include "proto.h"
/* #include <string.h>  / *  memcpy() * / */
#include <stdlib.h>  /*  malloc() */
#include <string.h>  /*  strncpy(),memset() */
#ifdef WIN32 
#include <winsock2.h>
#else
#include <netinet/in.h>  /*  htonl(),htons() */
#endif 
/*  this code should do the protocol work .... */
/*  */
extern uint8_t obuf[MAXPLEN];
int focus_oid=-1;  /*  initially focus the pid */
/*  handle an incoming command from the client .. */


int prot_com_in(struct t_process *p, uint8_t *pbuf)
{
	uint8_t 	command;
	int 	i;
	char 	name[NAME_MAX];
	uint8_t	*buf,*cptr=NULL;
	uint16_t length;
	uint16_t num;
	uint16_t w,h,x,y;
	uint32_t oid,toid;
	uint8_t	type;
	uint32_t flags,mcp_oid=-1;
	command=pbuf[0];
	if (p->id!=0)
	{
		mcp_oid=p->mcp_oid;		 /*  get mcp-oid if we need to report something to */
								 /*  the mcp */
		if ((mcp_oid==-1) && (command!=S3D_P_C_INIT))
		{
			dprintf(MED,"prot_com_in(): commands without beeing initialized ?! no way, kicking ...");
			event_quit(p);
		}
	} 
	length=ntohs(*((uint16_t *)((uint8_t *)pbuf+1)));
	cptr=buf=pbuf+3;
/* 	if (mcp_oid==-1) dprintf(HIGH,"couldn't find mcp-oid for pid %d!",p->id); */
		switch (command) {
			case S3D_P_C_INIT:
				memset(name,0,NAME_MAX);
				if (length>NAME_MAX) i=NAME_MAX; else i=length;
				strncpy(name,(char *)buf,i);
				dprintf(LOW,"[%d]\"%s\" logged in", p->id,name);
				if (process_protinit(p,name))
					event_quit(p);  /*  couldn't get process */
				else
					event_init(p);
				break;
			case S3D_P_C_NEW_OBJ:
				oid=htonl(obj_new(p));
/* 				dprintf(LOW,"pid %d registering new object %d",pid,oid); */
				prot_com_out(p,S3D_P_S_NEWOBJ,(uint8_t *)&oid, 4);
				break;
			case S3D_P_C_DEL_OBJ:
				if (length==4)
				{
					oid=ntohl(*((uint32_t *)cptr));		
					obj_del(p,oid);
				}
				break;
			case S3D_P_C_CLONE:
				if (length==8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					toid=ntohl(*((uint32_t *)cptr));
					obj_clone_change(p,oid,toid);
				}
				break;
			case S3D_P_C_LINK:
				if (length==4)
				{
					oid=ntohl(*((uint32_t *)cptr));
					obj_unlink(p,oid);
				}
				if (length==8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					toid=ntohl(*((uint32_t *)cptr));
					obj_link(p,oid,toid);
				}
				break;
			case S3D_P_C_QUIT:
				dprintf(LOW,"QUIT issued");
				event_quit(p);
				break;
			case S3D_P_C_PUSH_VERTEX:
				if (length>4)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-4)/(4*3);
/* 					dprintf(LOW,"received %d new vertices for object oid...%d", num, oid); */
					obj_push_vertex(p,oid, (float  *)cptr, num);
				}
				break;
			case S3D_P_C_PUSH_MAT:
				if (length>4)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-4)/(4*12);
/* 					dprintf(LOW,"received %d new materials for object oid...%d", num, oid); */
					obj_push_mat(p,oid, (float *)cptr, num);
				}
				break;
			case S3D_P_C_PUSH_POLY:
				if (length>4)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-4)/(4*4);
/* 					dprintf(LOW,"received %d new polygons for object oid...%d", num, oid); */
					for (i=0;i<(num*4);i++)
						*((uint32_t *)cptr+i)=
								ntohl(*((uint32_t *)cptr+i));
					 /*  convert index names */
					obj_push_poly(p,oid, (uint32_t *)cptr, num);
				}
				break;
			case S3D_P_C_PUSH_TEX:
				if (length>4)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-4)/(2*2);
					dprintf(LOW,"received %d new textures for object oid...%d", num, oid);
					for (i=0;i<(num*2);i++)
						*((uint16_t *)cptr+i)=
								ntohs(*((uint16_t *)cptr+i));
					 /*  convert index names */
					obj_push_tex(p,oid, (uint16_t *)cptr, num);
				}
				break;
			case S3D_P_C_PEP_POLY_NORMAL:
				if (length>4)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-4)/(9*4);
					dprintf(MED,"PEP_POLY_NORMAL[%d]: oid %d, %f polys",length,oid, (length-4)/(9.0*4.0));
					obj_pep_poly_normal(p,oid, (float *)cptr, num);
				}
				break;
			case S3D_P_C_PEP_POLY_TEXC:
				if (length>4)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-4)/(6*4);
					dprintf(MED,"PEP_POLY_TEXC[%d]: oid %d, %f polys",length,oid, (length-4)/(6.0*4.0));
					obj_pep_poly_texc(p,oid, (float *)cptr, num);
				}
				break;
			case S3D_P_C_PEP_MAT:
				if (length>4)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-4)/(4*12);
					dprintf(LOW,"PEP_MAT[%d]: %d materials for object oid...%d", length, num, oid);
					obj_pep_mat(p,oid, (float *)cptr, num);
				}
				break;
			case S3D_P_C_PEP_MAT_TEX:
				if (length>4)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-4)/(4);
					dprintf(LOW,"PEP_MAT_TEX[%d]: %d materials for object oid...%d", length, num, oid);
					obj_pep_mat_tex(p,oid, (uint32_t *)cptr, num);
				}
				break;
			case S3D_P_C_LOAD_POLY_NORMAL:
				if (length>8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					toid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-8)/(9*4);
					dprintf(MED,"LOAD_POLY_NORMAL[%d]: oid %d, %f polys",length,oid, (length-8)/(9.0*4.0));
					obj_load_poly_normal(p,oid, (float *)cptr, toid, num);
				}
				break;
			case S3D_P_C_LOAD_POLY_TEXC:
				if (length>8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					toid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-8)/(6*4);
					dprintf(MED,"LOAD_POLY_TEXC[%d]: oid %d, %f polys",length,oid, (length-8)/(6.0*4.0));
					obj_load_poly_texc(p,oid, (float *)cptr, toid, num);
				}
				break;
			case S3D_P_C_LOAD_MAT:
				if (length>8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					toid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=(length-8)/(4*12);
					dprintf(LOW,"LOAD_MAT[%d]: %d materials for object oid...%d", length, num, oid);
					obj_load_mat(p,oid, (float *)cptr, toid, num);
				}
				break;
			case S3D_P_C_LOAD_TEX:
				if (length>8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					toid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					x=ntohs(*((uint16_t *)cptr));			cptr+=2;
					y=ntohs(*((uint16_t *)cptr));			cptr+=2;
					w=ntohs(*((uint16_t *)cptr));			cptr+=2;
					h=ntohs(*((uint16_t *)cptr));			cptr+=2;
					num=length-16;
	 /* 				dprintf(MED,"LOAD_TEX[%d]: oid %d, texture %d, [%d x %d] data at [%d x %d] (%d = %d)",length, oid,toid,w,h,x,y,num,w*h*4,num); */
					if ((w*h*4)==num)  /*  check correct size */
						obj_load_tex(p,oid, toid, x, y, w, h, cptr);
				}
				break;
			case S3D_P_C_DEL_VERTEX:
				if (length==8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=ntohl(*((uint32_t *)cptr));		cptr+=4;
/* 					dprintf(LOW,"deleting %d vertices for object oid...%d", num, oid); */
					obj_del_vertex(p,oid,num);
				}
				break;
			case S3D_P_C_DEL_POLY:
				if (length==8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=ntohl(*((uint32_t *)cptr));		cptr+=4;
/* 					dprintf(LOW,"deleting %d vertices for object oid...%d", num, oid); */
					obj_del_poly(p,oid,num);
				}	
				break;
			case S3D_P_C_DEL_MAT:
				if (length==8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=ntohl(*((uint32_t *)cptr));		cptr+=4;
/* 					dprintf(LOW,"deleting %d materials for object oid...%d", num, oid); */
					obj_del_mat(p,oid,num);
				}
				break;
			case S3D_P_C_DEL_TEX:
				if (length==8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					num=ntohl(*((uint32_t *)cptr));		cptr+=4;
/* 					dprintf(LOW,"deleting %d textures for object oid...%d", num, oid); */
					obj_del_tex(p,oid,num);
				}
				break;
			case S3D_P_C_TOGGLE_FLAGS:
				if (length==9)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					type=*cptr;								cptr+=1;
					flags=ntohl(*((uint32_t *)cptr));		cptr+=4;
					obj_toggle_flags(p,oid, type, flags);
				}
				break;
			case S3D_P_C_TRANSLATE:
				if (length>=16)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					obj_translate(p,oid,(float *)cptr);
				}
				break;
			case S3D_P_C_ROTATE:
				if (length>=16)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					obj_rotate(p,oid,(float *)cptr);
				}
				break;
			case S3D_P_C_SCALE:
				if (length>=8)
				{
					oid=ntohl(*((uint32_t *)cptr));		cptr+=4;
					obj_scale(p,oid,*((float *)cptr));
				}
				break;
			case S3D_P_MCP_FOCUS:
				if ((p->id==MCP) && (length==4))
				{
					oid=ntohl(*((uint32_t *)cptr));
					mcp_focus(oid);	
				}
				break;
			default:
				dprintf(LOW,"don't know this command (%d)",command);
		}
	return(0);
}
/*  this pushes some buffer out on the wire... */
int prot_com_out(struct t_process *p, uint8_t opcode, uint8_t *buf, uint16_t length)
{
	uint8_t *ptr;
	if (p->con_type!=CON_NULL)
	{
		*(obuf)=opcode; 
		ptr=obuf+1;
		*((uint16_t *) ptr)=htons(length);
		if (length)
			memcpy(obuf+3,buf,length);
		if (n_writen(p,obuf,length+3)<0)
		{
			dprintf(LOW,"prot_com_out():n_writen(): connection seems to be dead (pid %d)", p->id);
			process_del(p->id);
		}
		return(0);
	}
	return(-1);
}

