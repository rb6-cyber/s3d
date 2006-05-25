/* $Id: imp_3dmf.c,v 1.1.2.5 2006/01/23 17:03:05 dahms Exp $ */

/*
    libg3d - 3D object loading library

    Copyright (C) 2005, 2006  Markus Dahms <mad@automagically.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <string.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/object.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>
#include <g3d/material.h>
#include <g3d/read.h>
#include <g3d/iff.h>

#define X3DMF_CHUNK_CHAR(id, shift) \
	((((id) >> (shift)) & 0xFF) == 0) ? \
	' ' : ((id) >> (shift)) & 0xFF

typedef struct {
	guint32 id;
	guint32 offset;
	guint32 type;
}
X3dmfTocEntry;

typedef struct {
	guint32 num_entries;
	X3dmfTocEntry *entries;
}
X3dmfToc;


static gboolean x3dmf_read_container(FILE *f, guint32 length, G3DModel *model,
	G3DObject *object, guint32 level, X3dmfToc *toc, G3DContext *context);
static X3dmfToc *x3dmf_read_toc(FILE *f, X3dmfToc *prev_toc,
	G3DContext *context);

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model)
{
	guint32 id, len, flags, tocloc, pos;
	guint16 ver_min, ver_maj;
	FILE *f;
	gchar txthead[10];
	X3dmfToc *toc = NULL;

	f = fopen(filename, "rb");

	if(f == NULL)
	{
		g_warning("failed to open file %s", filename);
		return FALSE;
	}

	g3d_iff_readchunk(f, &id, &len);
	if((id != G3D_IFF_MKID('3', 'D', 'M', 'F')) ||
		(len != 16))
	{
		fseek(f, 0, SEEK_SET);
		fread(txthead, sizeof(gchar), 10, f);
		if(strncmp(txthead, "3DMetafile", 10) == 0)
		{
			g_warning("file %s is an ASCII 3D Metafile (unhandled)\n",
				filename);
		}
		else
		{
			g_warning("file %s is not a 3D Metafile\n", filename);
		}
		return FALSE;
	}

	ver_maj = g3d_read_int16_be(f);
	ver_min = g3d_read_int16_be(f);

	flags = g3d_read_int32_be(f);

	fseek(f, 4, SEEK_CUR); /* FIXME: 64bit file offsets */
	tocloc = g3d_read_int32_be(f);

	/* read TOC if available */
	if(tocloc > 0)
	{
		pos = ftell(f);
		fseek(f, tocloc, SEEK_SET);
		toc = x3dmf_read_toc(f, NULL, context);
		fseek(f, pos, SEEK_SET);
	}

#if DEBUG > 0
	g_print("3DMF: version %d.%d (0x%08x) TOC @ 0x%08x\n",
		ver_maj, ver_min, flags, tocloc);
#endif

	x3dmf_read_container(f, (guint32) -1, model, NULL, 0, toc, context);

	fclose(f);

	return TRUE;
}

char *plugin_description(void)
{
	return g_strdup("import plugin for 3D Metafiles\n");
}

char **plugin_extensions(void)
{
	return g_strsplit("b3d:3mf:3dmf", ":", 0);
}

/*
 * 3DMF specific
 */

static X3dmfToc *x3dmf_read_toc(FILE *f, X3dmfToc *prev_toc,
	G3DContext *context)
{
	X3dmfToc *toc;
	guint32 off_next_toc, typeseed, refseed, entrytype, entrysize, nentries;
	guint32 noff, i;

	if(prev_toc)
		toc = prev_toc;
	else
		toc = g_new0(X3dmfToc, 1);

	/* skip tag and size (FIXME) */
	fseek(f, 8, SEEK_CUR);

	fseek(f, 4, SEEK_CUR); /* FIXME: 64bit file offsets */
	off_next_toc = g3d_read_int32_be(f);
	typeseed = g3d_read_int32_be(f);
	refseed = g3d_read_int32_be(f);
	entrytype = g3d_read_int32_be(f);
	entrysize = g3d_read_int32_be(f);
	nentries = g3d_read_int32_be(f);

	/* resize entry array */
	noff = toc->num_entries;
	toc->num_entries += nentries;
	toc->entries = (X3dmfTocEntry *)g_realloc(toc->entries,
		toc->num_entries * sizeof(X3dmfTocEntry));

	/* read TOC entries */
	for(i = 0; i < nentries; i ++)
	{
		toc->entries[noff + i].id = g3d_read_int32_be(f);
		fseek(f, 4, SEEK_CUR); /* FIXME: 64bit file offsets */
		toc->entries[noff + i].offset = g3d_read_int32_be(f);

		if((entrytype == 1) && (entrysize == 16))
		{
			toc->entries[noff + i].type = g3d_read_int32_be(f);
		}
#if DEBUG > 0
		g_print("3DMF: TOC: %06d @ 0x%08x\n",
			toc->entries[noff + i].id,
			toc->entries[noff + i].offset);
#endif
	}

	/* read next toc */
	if(off_next_toc > 0)
	{
		fseek(f, off_next_toc, SEEK_SET);
		toc = x3dmf_read_toc(f, toc, context);
	}

	return toc;
}

static gboolean x3dmf_read_mesh(FILE *f, G3DObject *object, G3DContext *context)
{
	guint32 i, j, nconts, nfaces, nbytes = 0;
	G3DFace *face;

	object->vertex_count = g3d_read_int32_be(f);
	object->vertex_data = g_new0(gfloat, object->vertex_count * 3);
	nbytes += 4;

	for(i = 0; i < object->vertex_count; i ++)
	{
		object->vertex_data[i * 3 + 0] = g3d_read_float_be(f);
		object->vertex_data[i * 3 + 1] = g3d_read_float_be(f);
		object->vertex_data[i * 3 + 2] = g3d_read_float_be(f);
		nbytes += 12;

		g3d_context_update_interface(context);
	}

	nfaces = g3d_read_int32_be(f);
	nconts = g3d_read_int32_be(f);
	nbytes += 8;

	for(i = 0; i < nfaces; i ++)
	{
		face = g_malloc0(sizeof(G3DFace));

		face->vertex_count = g3d_read_int32_be(f);
		nbytes += 4;
		face->vertex_indices = g_new0(guint32, face->vertex_count);

		for(j = 0; j < face->vertex_count; j ++)
		{
			face->vertex_indices[j] = g3d_read_int32_be(f);
			nbytes += 4;
		}

		face->material = g_slist_nth_data(object->materials, 0);
		object->faces = g_slist_prepend(object->faces, face);

		g3d_context_update_interface(context);
	}

	return nbytes;
}

static G3DObject *x3dmf_object_new(FILE *f, G3DModel *model)
{
	G3DObject *object;
	G3DMaterial *material;

	object = g_new0(G3DObject, 1);
	material = g3d_material_new();

	object->name = g_strdup_printf("container @ 0x%08lx", ftell(f) - 8);
	model->objects = g_slist_append(model->objects, object);
	object->materials =	g_slist_append(object->materials, material);

	return object;
}

static guint32 x3dmf_read_packed(FILE *f, guint32 maxx, guint32 *nread)
{
	if(maxx > 0xFFFE)
	{
		if(nread) (*nread) += 4;
		return g3d_read_int32_be(f);
	}
	else if(maxx > 0xFE)
	{
		if(nread) (*nread) += 2;
		return g3d_read_int16_be(f);
	}
	else
	{
		if(nread) (*nread) += 1;
		return g3d_read_int8(f);
	}
}

/*
 [tmsh] - TriMesh
 http://developer.apple.com/documentation/QuickTime/QD3D/qd3dmetafile.33.htm

 Uns32               numTriangles
 Uns32               numTriangleAttributeTypes
 Uns32               numEdges
 Uns32               numEdgeAttributeTypes
 Uns32               numPoints
 Uns32               numVertexAttributeTypes
 TriMeshTriangleData         triangles[numTriangles]
 TriMeshEdgeData             edges[numEdges]
 Point3D                     points[numPoints]
 BoundingBox                 bBox
*/

static guint32 x3dmf_read_tmsh(FILE *f, G3DObject *object,
	G3DContext *context)
{
	G3DFace *face;
	guint32 nread = 0, nfaces, nverts, nedges, i;

	nfaces = g3d_read_int32_be(f); /* numTriangles */
	nread += 4;

	g3d_read_int32_be(f); /* numTriangleAttributeTypes */
	nread += 4;

	nedges = g3d_read_int32_be(f); /* numEdges */
	nread += 4;

	g3d_read_int32_be(f); /* numEdgeAttributeTypes */
	nread += 4;

	nverts = g3d_read_int32_be(f); /* numPoints */
	nread += 4;

	g3d_read_int32_be(f); /* numVertexAttributeTypes */
	nread += 4;

#if DEBUG > 0
	g_print("3DMF: [tmsh] %d faces, %d edges, %d vertices\n",
		nfaces, nedges, nverts);
#endif

	/* triangles */
	for(i = 0; i < nfaces; i ++)
	{
		face = g_new0(G3DFace, 1);

		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		face->vertex_indices[0] = x3dmf_read_packed(f, nfaces, &nread);
		face->vertex_indices[1] = x3dmf_read_packed(f, nfaces, &nread);
		face->vertex_indices[2] = x3dmf_read_packed(f, nfaces, &nread);

		face->material = g_slist_nth_data(object->materials, 0);
		object->faces = g_slist_prepend(object->faces, face);
	}

	/* edges */
	for(i = 0; i < nedges; i ++)
	{
		/* pointIndices */
		x3dmf_read_packed(f, nedges, &nread);
		x3dmf_read_packed(f, nedges, &nread);
		/* triangleIndices */
		x3dmf_read_packed(f, nedges, &nread);
		x3dmf_read_packed(f, nedges, &nread);
	}

	/* points */
	object->vertex_count = nverts;
	object->vertex_data = g_new0(gfloat, 3 * nverts);
	for(i = 0; i < nverts; i ++)
	{
		object->vertex_data[i * 3 + 0] = g3d_read_float_be(f);
		object->vertex_data[i * 3 + 1] = g3d_read_float_be(f);
		object->vertex_data[i * 3 + 2] = g3d_read_float_be(f);
		nread += 12;
	}

	/* bBox */
	/* Point3D min */
	g3d_read_float_be(f);
	g3d_read_float_be(f);
	g3d_read_float_be(f);
	nread += 12;
	/* Point3D max */
	g3d_read_float_be(f);
	g3d_read_float_be(f);
	g3d_read_float_be(f);
	nread += 12;
	/* boolean isEmpty */
	g3d_read_int32_be(f);
	nread += 4;

	return nread;
}

static gboolean x3dmf_read_rfrn(FILE *f, G3DModel *model,
	X3dmfToc *toc, G3DContext *context)
{
	G3DObject *object;
	guint32 id, len, i, refid, savedoffset;
	X3dmfTocEntry *tocentry = NULL;

	refid = g3d_read_int32_be(f);
	if(refid == 0)
	{
		/* FIXME */
		return FALSE;
	}

	if(toc == NULL)
	{
		return FALSE;
	}

	/* find reference object */
	for(i = 0; i < toc->num_entries; i ++)
	{
		if(toc->entries[i].id == refid)
		{
			tocentry = &(toc->entries[i]);
		}
	}

	if(tocentry == NULL)
	{
		/* FIXME: error handling */
		return FALSE;
	}

	savedoffset = ftell(f);
	fseek(f, tocentry->offset, SEEK_SET);

	object = x3dmf_object_new(f, model);

	g3d_iff_readchunk(f, &id, &len);
	switch(id)
	{
		case G3D_IFF_MKID('c', 't', 'n', 'r'):
			x3dmf_read_container(f, len, model, NULL, 0xFF, toc, context);
			break;

		default:
			break;
	}

	fseek(f, savedoffset, SEEK_SET);

	return TRUE;
}

static gboolean x3dmf_read_container(FILE *f, guint32 length, G3DModel *model,
	G3DObject *object, guint32 level, X3dmfToc *toc, G3DContext *context)
{
	G3DMaterial *material = NULL;
	guint32 len, id, chk, i;
	gfloat matrix[16];

	g3d_matrix_identity(matrix);

	while(length > 0)
	{
		if(feof(f)) break;

		g3d_iff_readchunk(f, &id, &len);
		length -= 8;

		if(id == 0)
			return FALSE;

#if DEBUG > 1
		g_print("%.*s[%c%c%c%c]: %d bytes\n",
			level * 2, " ",
			X3DMF_CHUNK_CHAR(id, 24), X3DMF_CHUNK_CHAR(id, 16),
			X3DMF_CHUNK_CHAR(id, 8), X3DMF_CHUNK_CHAR(id, 0),
			len);
#endif
		length -= len;

		switch(id)
		{
			case G3D_IFF_MKID('a', 'm', 'b', 'n'):
				/* ambient light */
				break;

			case G3D_IFF_MKID('a', 't', 't', 'r'):
				/* attribute set */
				break;

			case G3D_IFF_MKID('b', 'g', 'n', 'g'):
				/* begin group */
				fseek(f, len, SEEK_CUR);
				break;

			case G3D_IFF_MKID('c', 'n', 't', 'r'):
				/* container */
#if DEBUG > 0
				g_print("3DMF: new container (level %d) @ 0x%lx (%d bytes)\n",
					level + 1, ftell(f) - 8, len);
#endif
				x3dmf_read_container(f, len, model, object, level + 1,
					toc, context);
				break;

			case G3D_IFF_MKID('c', 's', 'p', 'c'):
				/* specular control */
				g3d_read_float_be(f);
				break;

			case G3D_IFF_MKID('c', 't', 'w', 'n'):
				/* interactive renderer */
				break;

			case G3D_IFF_MKID('e', 'n', 'd', 'g'):
				/* end group */
				break;

			case G3D_IFF_MKID('k', 'd', 'i', 'f'):
				/* diffuse color */
				if(object)
				{
#if DEBUG > 2
					g_print("3DMF: kdif: got object\n");
#endif
					material = g_slist_nth_data(object->materials, 0);
					material->r = g3d_read_float_be(f);
					material->g = g3d_read_float_be(f);
					material->b = g3d_read_float_be(f);
				}
				else
				{
					fseek(f, len, SEEK_CUR);
				}
				break;

			case G3D_IFF_MKID('k', 's', 'p', 'c'):
				/* specular color */
				if(object)
				{
#if DEBUG > 2
					g_print("3DMF: kspc: got object\n");
#endif
					material = g_slist_nth_data(object->materials, 0);
					material->specular[0] = g3d_read_float_be(f);
					material->specular[1] = g3d_read_float_be(f);
					material->specular[2] = g3d_read_float_be(f);
				}
				else
				{
					fseek(f, len, SEEK_CUR);
				}
				break;

			case G3D_IFF_MKID('k', 'x', 'p', 'r'):
				/* transparency color */
				if(object)
				{
					/* use average as alpha */
					material = g_slist_nth_data(object->materials, 0);
					material->a = 1.0 - (g3d_read_float_be(f) +
						g3d_read_float_be(f) + g3d_read_float_be(f)) / 3.0;

					if(material->a < 0.1)
						material->a = 0.1;
				}
				else
				{
					fseek(f, len, SEEK_CUR);
				}
				break;

			case G3D_IFF_MKID('l', 'g', 'h', 't'):
				/* light data */

				/* isOn */
				g3d_read_int32_be(f);
				/* intensity */
				g3d_read_int32_be(f);
				/* color */
				g3d_read_float_be(f);
				g3d_read_float_be(f);
				g3d_read_float_be(f);
				break;

			case G3D_IFF_MKID('m', 'e', 's', 'h'):
				/* mesh */
				if(object == NULL)
					object = x3dmf_object_new(f, model);
				material = g_slist_nth_data(object->materials, 0);

				chk = x3dmf_read_mesh(f, object, context);
				g3d_object_transform(object, matrix);
				if(chk != len)
				{
					g_warning("3DMF: mesh: wrong length (%u != %u)\n",
						chk, len);
					return FALSE;
				}
				break;

			case G3D_IFF_MKID('m', 't', 'r', 'x'):
				/* matrix */
				for(i = 0; i < 16; i ++)
					matrix[i] = g3d_read_float_be(f);
				if(object)
				{
#if DEBUG > 2
					g_print("3DMF: mtrx: object is set\n");
#endif
					g3d_object_transform(object, matrix);
				}
#if DEBUG > 3
				for(i = 0; i < 4; i ++)
					g_print("3DMF: mtrx: %+1.2f %+1.2f %+1.2f %+1.2f\n",
						matrix[i * 4 + 0], matrix[i * 4 + 1],
						matrix[i * 4 + 2], matrix[i * 4 + 3]);
#endif
				break;

			case G3D_IFF_MKID('n', 'r', 'm', 'l'):
				/* normal */
				fseek(f, 12, SEEK_CUR);
				break;

			case G3D_IFF_MKID('r', 'f', 'r', 'n'):
				/* reference */
				x3dmf_read_rfrn(f, model, toc, context);
				break;

			case G3D_IFF_MKID('s', 'e', 't', ' '):
				/* ??: skip this cntr chunk */
				fseek(f, length, SEEK_CUR);
				length = 0;
				break;

			case G3D_IFF_MKID('t', 'm', 's', 'h'):
				/* triangle mesh */
				if(object == NULL)
					object = x3dmf_object_new(f, model);
				material = g_slist_nth_data(object->materials, 0);

				chk = x3dmf_read_tmsh(f, object, context);
				g3d_object_transform(object, matrix);
				if(chk != len)
				{
#if DEBUG > 0
					g_print("3DMF: tmsh: offset %d bytes\n", len - chk);
#endif
					fseek(f, len - chk, SEEK_CUR);
				}
				break;

			case G3D_IFF_MKID('t', 'o', 'c', ' '):
				/* TOC, should be already handled */
				fseek(f, len, SEEK_CUR);
				break;

			case G3D_IFF_MKID('t', 'r', 'n', 's'):
				/* translate */
				if(object)
				{
					gfloat x,y,z;
					gfloat matrix[16];

					x = g3d_read_float_be(f);
					y = g3d_read_float_be(f);
					z = g3d_read_float_be(f);

					g3d_matrix_identity(matrix);
					g3d_matrix_translate(x, y, z, matrix);

					g3d_object_transform(object, matrix);
				}
				else
				{
#if DEBUG > 0
					g_print("3DMF: [trns] no object\n");
#endif
					fseek(f, 12, SEEK_CUR);
				}
				break;

			case G3D_IFF_MKID('v', 'a', 'n', 'a'):
				/* view angle aspect camera */

				/* fieldOfView */
				g3d_read_float_be(f);
				/* aspectRatioXtoY */
				g3d_read_float_be(f);
				break;

			case G3D_IFF_MKID('v', 'a', 's', 'l'):
				/* vertex attribute set list */
				fseek(f, len, SEEK_CUR);
				break;

			case G3D_IFF_MKID(0xFF, 0xFF, 0xFF, 0xE7):
				/* unknown */
				fseek(f, len, SEEK_CUR);
#if DEBUG > 0
				g_print("3DMF: 0xFFFFFFE7 (unknown)\n");
#endif
				break;

			case G3D_IFF_MKID(0xFF, 0xFF, 0xFF, 0xE9):
				/* unknown */
				fseek(f, len, SEEK_CUR);
#if DEBUG > 0
				g_print("3DMF: 0xFFFFFFE9 (unknown)\n");
#endif
				break;

			case G3D_IFF_MKID(0xFF, 0xFF, 0xFF, 0xEA):
				/* unknown */
				fseek(f, len, SEEK_CUR);
#if DEBUG > 0
				g_print("3DMF: 0xFFFFFFEA (unknown)\n");
#endif
				break;

			case G3D_IFF_MKID(0xFF, 0xFF, 0xFF, 0xEB):
				/* unknown */
				fseek(f, len, SEEK_CUR);
#if DEBUG > 0
				g_print("3DMF: 0xFFFFFFEB (unknown)\n");
#endif
				break;

			case G3D_IFF_MKID(0xFF, 0xFF, 0xFF, 0xEC):
				/* unknown */
				fseek(f, len, SEEK_CUR);
#if DEBUG > 0
				g_print("3DMF: 0xFFFFFFEC (unknown)\n");
#endif
				break;

			case G3D_IFF_MKID(0xFF, 0xFF, 0xFF, 0xEF):
				/* end of container? */
				fseek(f, len, SEEK_CUR);
#if DEBUG > 0
				g_print("3DMF: 0xFFFFFFEF (end of container?)\n");
#endif
				return TRUE;
				break;

			case G3D_IFF_MKID(0xFF, 0xFF, 0xFF, 0xF4):
				/* unknown */
				fseek(f, len, SEEK_CUR);
#if DEBUG > 0
				g_print("3DMF: 0xFFFFFFF4 (unknown)\n");
#endif
				break;

			case G3D_IFF_MKID(0xFF, 0xFF, 0xFF, 0xFD):
				/* unknown */
				fseek(f, len, SEEK_CUR);
#if DEBUG > 0
				g_print("3DMF: 0xFFFFFFFD (unknown)\n");
#endif
				break;

			default:
#if DEBUG > 0
				g_print("3DMF: Container: unknown chunk '%c%c%c%c' @ 0x%08lx "
					"(%d bytes)\n",
					X3DMF_CHUNK_CHAR(id, 24), X3DMF_CHUNK_CHAR(id, 16),
					X3DMF_CHUNK_CHAR(id, 8), X3DMF_CHUNK_CHAR(id, 0),
					ftell(f) - 8, len);
#endif
				fseek(f, len, SEEK_CUR);
				break;
		}
	}

	return TRUE;
}

