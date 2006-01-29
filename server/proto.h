/*  this defines the protocol */

/*  simply put, a command can be issued from either the client or  */
/*  the server. of course, the client's command-set should be kept  */
/*  as low as possible to keep the library added to the client's code */
/*  small. */


/*  S3D_P_(C=Client,S=Server)_(command) */
/*  C/S defines if who can invoke the command  */
/*  */
/*  every command is 1 byte long, arguments differ. */
#define S3D_P_C_INIT			1
			/*  max 256b: 	name */
#define S3D_P_C_QUIT			2  /*  no argument */

#define S3D_P_C_NEW_OBJ			3  /*  add a new object */
			 /*  return: 4b:	object id  */
#define S3D_P_C_DEL_OBJ			4
			 /*  4b:			object id */
#define S3D_P_C_CLONE			5
			 /*  4b:			object id */
			 /*  return: 4b: object id */
			 /*  if 8b: 		 */
			 /*  4b:			object id */
			 /*  4b:			target oid */
#define S3D_P_C_LINK			6
			 /*  if 4b: */
			 /*  4b:		oid (for unlink) */
			 /*  if 8b: */
			 /*  4b:		oid from */
			 /*  4b:		oid to */
#define S3D_P_C_PUSH_VERTEX		8
			 /*  4b:		object id */
			 /*  n*3f:	vertexes, each with x,y,z in float */
#define S3D_P_C_PUSH_MAT		9
			 /*  4b:		object id */
			 /*  3*4f:	material elements [amb,spec,diff with r,g,b,a] */
#define S3D_P_C_PUSH_POLY		10
			 /*  4b:		object id */
#define S3D_P_C_PUSH_TEX		11
			 /*  4b: 			object id */
			 /*  nx(2x2b):	width,height */
#define S3D_P_C_DEL_VERTEX		12
			 /*  4b:		object id */
			 /*  4b:		number */
#define S3D_P_C_DEL_POLY		13
			 /*  4b:		object id */
			 /*  4b:		number */
#define S3D_P_C_DEL_MAT			14
			 /*  4b:		object id */
			 /*  4b:		number */
#define S3D_P_C_DEL_TEX			15
			 /*  4b:		object id */
			 /*  4b:		number */
#define S3D_P_C_PEP_POLY_NORMAL	16
			 /*  4b:		object id */
			 /*  n*9f:	normals (3* x/y/z for each vertex of the poly) */
#define S3D_P_C_PEP_POLY_TEXC	17
			 /*  4b:		object id */
			 /*  n*6f		poly texture coordinates (3* u/v for each vertex of the poly) */
#define S3D_P_C_PEP_MAT			18
			 /*  4b:		object id */
			 /*  3*4f:	material elements [amb,spec,diff with r,g,b,a] */
#define S3D_P_C_PEP_MAT_TEX		19
			 /*  4b:		object id */
			 /*  4b:		texture index references */

#define S3D_P_C_LOAD_POLY_NORMAL	24
			 /*  4b:		object id */
			 /*  4b:		position */
			 /*  n*9f:	normals (3* x/y/z for each vertex of the poly) */
#define S3D_P_C_LOAD_POLY_TEXC	25
			 /*  4b:		object id */
			 /*  4b:		position */
			 /*  n*6f		poly texture coordinates (3* u/v for each vertex of the poly) */
#define S3D_P_C_LOAD_MAT		26
			 /*  4b:		object id */
			 /*  4b:		position */
			 /*  3*4f:	material elements [amb,spec,diff with r,g,b,a] */
#define S3D_P_C_LOAD_TEX		28
			 /*  4b:		object id */
			 /*  4b:		texture number */
			 /*  4*2b:	xpos,ypos,width,height */
			 /*  n*2b:	pixbuf (16bit) */
#define S3D_P_C_LOAD_MAT_TEX	29
			 /*  4b:		object id */
			 /*  4b:		texture index references */

#define S3D_P_C_TOGGLE_FLAGS 	32
			 /*  4b:		object id */
			 /*  1b:		type  */
			 /*  4b:		flags */
#define S3D_P_C_TRANSLATE 		33
			 /*  4b:		object id */
			 /*  3f:		position */
#define S3D_P_C_ROTATE			34
			 /*  4b:		object id */
			 /*  3f:		rotation angle over x,y,z axis */
#define S3D_P_C_SCALE			35
			 /*  4b: 		object id */
			 /*  1f:		scale */
#define S3D_P_C_GET_SIZE		36
			   /*  4b: object id */
#define S3D_P_MCP_FOCUS			66		 /*  set the app which should get the keystrokes etc */
			 /*  4b:		object id/pid */
			 /*  */
			 /*  */
			 /*   */
#define S3D_P_MCP_OBJECT	67
			 /*  4b: 		oid */
			 /*  3*f:		translate */
			 /*  ... ? */
			 /*  max 256b:name */
#define S3D_P_MCP_DEL_OBJECT 68

#define S3D_P_S_INIT	1
			 /*  1b: acknowledged */
			 /*  3b: version,major,minor */
			 /*  description string */
#define S3D_P_S_QUIT	2
#define S3D_P_S_CLICK	3
			 /*  4b: oid */
#define	S3D_P_S_KEY		4
			 /*  2b: button */
#define S3D_P_S_NEWOBJ	16
			 /*  4b: oid */

#define S3D_P_S_OINFO	32
			 /*  4b: 		oid */
			 /*  3*f:		translate */
			 /*  3*f:		rotate */
			 /*  1*f:		scale */
			 /*  1*f:		radius */
			 /*  max 256b:name */
