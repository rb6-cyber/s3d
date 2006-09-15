#ifndef _IMP_3DS_CHUNKS_H
#define _IMP_3DS_CHUNKS_H

#include <glib.h>

#include "imp_3ds_callbacks.h"

typedef struct {
	guint32 id;
	char *desc;
	gboolean container;
	x3ds_callback callback;
} x3ds_chunk_desc;

static x3ds_chunk_desc x3ds_chunks[] = {
	{ 0x0002, "M3D version",                0, x3ds_cb_0x0002 },
	{ 0x0001, "(unknown)",                  0, NULL },
	{ 0x0010, "color (float)",              0, x3ds_cb_0x0010 },
	{ 0x0011, "color (24 bit)",             0, x3ds_cb_0x0011 },
	{ 0x0012, "line color (24 bit)",        0, NULL },
	{ 0x0013, "line color (float)",         0, NULL },
	{ 0x0030, "percentage (short)",         0, x3ds_cb_0x0030 },
	{ 0x0031, "percentage (float)",         0, x3ds_cb_0x0031 },
	{ 0x0100, "global scale",               0, NULL },

	{ 0x1100, "BIT_MAP",                    0, NULL },
	{ 0x1200, "SOLID_BGND",                 1, NULL },
	{ 0x1201, "USE_SOLID_BGND",             0, NULL },
	{ 0x1300, "V_GRADIENT",                 0, NULL },
	{ 0x1301, "USE_V_GRADIENT",             0, NULL },
	{ 0x1400, "LO_SHADOW_BIAS",             0, NULL },
	{ 0x1410, "HI_SHADOW_BIAS",             0, NULL },
	{ 0x1420, "SHADOW_MAP_SIZE",            0, NULL },
	{ 0x1430, "SHADOW_SAMPLES",             0, NULL },
	{ 0x1440, "SHADOW_RANGE",               0, NULL },
	{ 0x1450, "SHADOW_FILTER",              0, NULL },
	{ 0x1460, "RAY_BIAS",                   0, NULL },
	{ 0x1500, "O_CONSTS",                   0, NULL },

	{ 0x2100, "AMBIENT_LIGHT",              0, NULL },
	{ 0x2200, "FOG",                        0, NULL },
	{ 0x2300, "DISTANCE_CUE",               0, NULL },
	{ 0x2301, "USE_DISTANCE_CUE",           0, NULL },
	{ 0x2302, "LAYER_FOG",                  0, NULL },

	{ 0x3000, "default view",               0, NULL },
	{ 0x3D3D, "mesh",                       1, NULL },
	{ 0x3D3E, "mesh version",               0, NULL },

	{ 0x4000, "named object",               1, x3ds_cb_0x4000 },
	{ 0x4100, "triangle object",            1, NULL },
	{ 0x4110, "point array",                0, x3ds_cb_0x4110 },
	{ 0x4111, "point flag array",           0, NULL },
	{ 0x4120, "face array",                 1, x3ds_cb_0x4120 },
	{ 0x4130, "mesh mat group",             0, x3ds_cb_0x4130 },
	{ 0x4140, "texture vertices",           0, x3ds_cb_0x4140 },
	{ 0x4150, "smooth group",               0, x3ds_cb_0x4150 },
	{ 0x4160, "mesh matrix",                0, x3ds_cb_0x4160 },
	{ 0x4165, "mesh color",                 0, NULL },
	{ 0x4170, "texture info",               0, NULL },
	{ 0x4600, "N_DIRECT_LIGHT",             0, NULL },
	{ 0x4700, "N_CAMERA",                   0, NULL },

	{ 0x7001, "VIEWPORT_LAYOUT",            0, NULL },

	{ 0xA000, "material name",              0, x3ds_cb_0xA000 },
	{ 0xA010, "ambient color",              1, NULL },
	{ 0xA020, "diffuse color",              1, NULL },
	{ 0xA030, "specular color",             1, NULL },
	{ 0xA040, "shininess",                  1, NULL },
	{ 0xA041, "shininess (2)",              1, NULL },
	{ 0xA042, "shininess (3)",              1, NULL },
	{ 0xA050, "transparency",               1, NULL },
	{ 0xA052, "fallthrough",                1, NULL },
	{ 0xA053, "blur",                       1, NULL },
	{ 0xA081, "two-sided",                  0, x3ds_cb_0xA081 },
	{ 0xA084, "self illumination",          1, NULL },
	{ 0xA085, "wire",                       0, NULL },
	{ 0xA086, "super-sampling",             0, NULL },
	{ 0xA087, "wire size",                  0, NULL },
	{ 0xA08A, "MAT_XPFALLIN",               0, NULL },
	{ 0xA08C, "MAT_PHONGSOFT",              0, NULL },
	{ 0xA08E, "MAT_WIREABS",                0, NULL },
	{ 0xA100, "shading",                    0, NULL },
	{ 0xA200, "texture map",                1, NULL },
	{ 0xA210, "opacity map",                1, NULL },
	{ 0xA220, "reflection map",             1, NULL },
	{ 0xA230, "bump map",                   1, NULL },
	{ 0xA250, "MAT_USE_REFBLUR",            0, NULL },
	{ 0xA252, "bump percentage",            0, NULL },
	{ 0xA300, "texture map name",           0, x3ds_cb_0xA300 },
	{ 0xA351, "texture map tiling",         0, NULL },
	{ 0xA352, "texture map blurring (old)", 0, NULL },
	{ 0xA353, "texture map blurring",       0, NULL },
	{ 0xA354, "texture map scale u",        0, x3ds_cb_0xA354 },
	{ 0xA356, "texture map scale v",        0, x3ds_cb_0xA356 },
	{ 0xA358, "texture map offset u",       0, NULL },
	{ 0xA35A, "texture map offset v",       0, NULL },
	{ 0xAFFF, "material",                   1, x3ds_cb_0xAFFF },

	{ 0xB000, "keyframe data",              1, NULL },
	{ 0xB001, "ambient data node",          1, NULL },
	{ 0xB002, "object node",                1, NULL },
	{ 0xB003, "camera node",                1, NULL },
	{ 0xB004, "target node",                1, NULL },
	{ 0xB005, "light node",                 1, NULL },
	{ 0xB006, "L_TARGET_NODE_TAG",          1, NULL },
	{ 0xB007, "spotlight node",             1, NULL },
	{ 0xB008, "KFSEG",                      0, NULL },
	{ 0xB009, "KFCURTIME",                  0, NULL },
	{ 0xB00A, "keyframe data header",       0, x3ds_cb_0xB00A },
	{ 0xB010, "node header",                0, x3ds_cb_0xB010 },
	{ 0xB011, "instance name",              0, NULL },
	{ 0xB012, "PRESCALE",                   0, NULL },
	{ 0xB013, "pivot",                      0, x3ds_cb_0xB013 },
	{ 0xB014, "bounding box",               0, NULL },
	{ 0xB015, "MORPH_SMOOTH",               0, NULL },
	{ 0xB020, "position tracking tag",      0, x3ds_cb_0xB020 },
	{ 0xB021, "rotation tracking tag",      0, x3ds_cb_0xB021 },
	{ 0xB022, "scale tracking tag",         0, NULL },
	{ 0xB023, "FOV_TRACK_TAG",              0, NULL },
	{ 0xB024, "ROLL_TRACK_TAG",             0, NULL },
	{ 0xB025, "COL_TRACK_TAG",              0, NULL },
	{ 0xB026, "MORPH_TRACK_TAG",            0, NULL },
	{ 0xB027, "HOT_TRACK_TAG",              0, NULL },
	{ 0xB028, "FALL_TRACK_TAG",             0, NULL },
	{ 0xB029, "HIDE_TRACK_TAG",             0, NULL },
	{ 0xB030, "node identification",        0, x3ds_cb_0xB030 },

	{ 0xD000, "VPDATA",                     0, NULL },

	{ 0x0000, NULL, 0, NULL }
};

#endif
