/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// model_common.c -- model loading and caching
// 2001-12-28 Merged model functions by Maddes

// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"

model_t	*loadmodel;
char	loadname[32];	// for hunk tags

byte	mod_novis[MAX_MAP_LEAFS/8];

#ifdef GLQUAKE
cvar_t	*gl_subdivide_size;
#endif

cvar_t	*external_lit;	// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes
cvar_t	*external_ent;	// 2001-09-12 .ENT support by Maddes
cvar_t	*external_vis;	// 2001-12-28 .VIS support by Maddes

// 2001-09-18 New cvar system by Maddes (Init)  start
/*
===============
Mod_Init_Cvars
===============
*/
void Mod_Init_Cvars (void)
{
#ifdef GLQUAKE
	gl_subdivide_size = Cvar_Get ("gl_subdivide_size", "128", CVAR_ARCHIVE|CVAR_ORIGINAL);
#endif

// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
	external_lit = Cvar_Get ("external_lit", "1", CVAR_NONE);
	Cvar_SetRangecheck (external_lit, Cvar_RangecheckBool, 0, 1);
	Cvar_Set(external_lit, external_lit->string);	// do rangecheck
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end

// 2001-09-12 .ENT support by Maddes  start
	external_ent = Cvar_Get ("external_ent", "1", CVAR_NONE);
	Cvar_SetRangecheck (external_ent, Cvar_RangecheckBool, 0, 1);
	Cvar_Set(external_ent, external_ent->string);	// do rangecheck
// 2001-09-12 .ENT support by Maddes  end

// 2001-12-28 .VIS support by Maddes  start
	external_vis = Cvar_Get ("external_vis", "1", CVAR_NONE);
	Cvar_SetRangecheck (external_vis, Cvar_RangecheckBool, 0, 1);
	Cvar_Set(external_vis, external_vis->string);	// do rangecheck
// 2001-12-28 .VIS support by Maddes  end
}
// 2001-09-18 New cvar system by Maddes (Init)  end

/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
// 2001-09-18 New cvar system by Maddes (Init)  start
/*
#ifdef GLQUAKE
	gl_subdivide_size = Cvar_Get ("gl_subdivide_size", "128", CVAR_ARCHIVE|CVAR_ORIGINAL);
#endif

// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
	external_lit = Cvar_Get ("external_lit", "1", CVAR_NONE);
	Cvar_SetRangecheck (external_lit, Cvar_RangecheckBool, 0, 1);
	Cvar_Set(external_lit, external_lit->string);	// do rangecheck
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end

// 2001-09-12 .ENT support by Maddes  start
	external_ent = Cvar_Get ("external_ent", "1", CVAR_NONE);
	Cvar_SetRangecheck (external_ent, Cvar_RangecheckBool, 0, 1);
	Cvar_Set(external_ent, external_ent->string);	// do rangecheck
// 2001-09-12 .ENT support by Maddes  end
*/
// 2001-09-18 New cvar system by Maddes (Init)  end

	memset (mod_novis, 0xff, sizeof(mod_novis));
}

/*
===============
Mod_Extradata

Caches the data if needed
===============
*/
void *Mod_Extradata (model_t *mod)
{
	void	*r;

	r = Cache_Check (&mod->cache);
	if (r)
		return r;

	Mod_LoadModel (mod, true);

	if (!mod->cache.data)
		Sys_Error ("Mod_Extradata: caching failed");
	return mod->cache.data;
}

/*
===============
Mod_PointInLeaf
===============
*/
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t		*node;
	float		d;
	mplane_t	*plane;

	if (!model || !model->nodes)
		Sys_Error ("Mod_PointInLeaf: bad model");

	node = model->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (mleaf_t *)node;
		plane = node->plane;
		d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return NULL;	// never reached
}


/*
===================
Mod_DecompressVis
===================
*/
byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->numleafs+7)>>3;
	out = decompressed;

#if 0
	memcpy (out, in, row);
#else
	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);
#endif

	return decompressed;
}

byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model)
{
	if (leaf == model->leafs)
		return mod_novis;
	return Mod_DecompressVis (leaf->compressed_vis, model);
}

/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t *Mod_ForName (char *name, qboolean crash)
{
	model_t	*mod;

	mod = Mod_FindName (name);

	return Mod_LoadModel (mod, crash);
}


/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

byte	*mod_base;

/*
=================
Mod_LoadVisibility
=================
*/
void Mod_LoadVisibility (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->visdata = NULL;
		return;
	}
// 2001-12-28 .VIS support by Maddes  start
//	loadmodel->visdata = Hunk_AllocName ( l->filelen, loadname);
	loadmodel->visdata = Hunk_AllocName ( l->filelen, "INT_VIS");
// 2001-12-28 .VIS support by Maddes  end
	memcpy (loadmodel->visdata, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadEntities
=================
*/
void Mod_LoadEntities (lump_t *l, loadedfile_t *brush_fileinfo)	// 2001-09-12 .ENT support by Maddes
{
// 2001-09-12 .ENT support by Maddes  start
	char	entfilename[1024];
	loadedfile_t	*fileinfo;
	searchpath_t	*s_check;

	loadmodel->entities = NULL;

	if (external_ent->value)
	{
		// check for a .ENT file
		strcpy(entfilename, loadmodel->name);
		COM_StripExtension(entfilename, entfilename);
		strcat(entfilename, ".ent");

		fileinfo = COM_LoadHunkFile (entfilename);
		if (fileinfo && fileinfo->filelen)
		{
			// .ENT file only allowed from same directory of map file or another directory before in the searchpath
			s_check = COM_GetDirSearchPath(brush_fileinfo->path);	// get last searchpath of map directory
			while ( (s_check = s_check->next) )		// next searchpath
			{
				if (s_check == fileinfo->path)	// found .ENT searchpath = after map directory
				{
					Con_DPrintf("%s not allowed from %s as map is from %s\n", entfilename, fileinfo->path->pack ? fileinfo->path->pack->filename : fileinfo->path->filename, brush_fileinfo->path->pack ? brush_fileinfo->path->pack->filename : brush_fileinfo->path->filename);
					break;
				}
			}

			if (!s_check)	// .ENT searchpath not found = before map directory
			{
				Con_Printf("%s loaded from %s\n", entfilename, fileinfo->path->pack ? fileinfo->path->pack->filename : fileinfo->path->filename);
				loadmodel->entities = fileinfo->data;
				return;
			}
		}

		// no .ENT found, use the original entity list
	}
// 2001-09-12 .ENT support by Maddes  end

	if (!l->filelen)
	{
		loadmodel->entities = NULL;
		return;
	}
// 2001-09-12 .ENT support by Maddes  start
//	loadmodel->entities = Hunk_AllocName ( l->filelen, loadname);
	loadmodel->entities = Hunk_AllocName ( l->filelen, "INT_ENT");
// 2001-09-12 .ENT support by Maddes  end
	memcpy (loadmodel->entities, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadVertexes
=================
*/
void Mod_LoadVertexes (lump_t *l)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->position[0] = LittleFloat (in->point[0]);
		out->position[1] = LittleFloat (in->point[1]);
		out->position[2] = LittleFloat (in->point[2]);
	}
}

/*
=================
Mod_LoadSubmodels
=================
*/
void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	dmodel_t	*out;
	int			i, j, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		for (j=0 ; j<MAX_MAP_HULLS ; j++)
			out->headnode[j] = LittleLong (in->headnode[j]);
		out->visleafs = LittleLong (in->visleafs);
		out->firstface = LittleLong (in->firstface);
		out->numfaces = LittleLong (in->numfaces);
	}
}

/*
=================
Mod_LoadEdges
=================
*/
void Mod_LoadEdges (lump_t *l)
{
	dedge_t *in;
	medge_t *out;
	int 	i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( (count + 1) * sizeof(*out), loadname);

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort(in->v[0]);
		out->v[1] = (unsigned short)LittleShort(in->v[1]);
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t *in;
	mtexinfo_t *out;
	int 	i, j, count;
	int		miptex;
	float	len1, len2;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<8 ; j++)
			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);
		len1 = Length (out->vecs[0]);
		len2 = Length (out->vecs[1]);
		len1 = (len1 + len2)/2;
		if (len1 < 0.32)
			out->mipadjust = 4;
		else if (len1 < 0.49)
			out->mipadjust = 3;
		else if (len1 < 0.99)
			out->mipadjust = 2;
		else
			out->mipadjust = 1;
#if 0
		if (len1 + len2 < 0.001)
			out->mipadjust = 1;		// don't crash
		else
			out->mipadjust = 1 / floor( (len1+len2)/2 + 0.1 );
#endif

		miptex = LittleLong (in->miptex);
		out->flags = LittleLong (in->flags);

		if (!loadmodel->textures)
		{
			out->texture = r_notexture_mip;	// checkerboard texture
			out->flags = 0;
		}
		else
		{
			if (miptex >= loadmodel->numtextures)
				Sys_Error ("miptex >= loadmodel->numtextures");
			out->texture = loadmodel->textures[miptex];
			if (!out->texture)
			{
				out->texture = r_notexture_mip; // texture not found
				out->flags = 0;
			}
		}
	}
}

/*
=================
Mod_SetParent
=================
*/
void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents < 0)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_LoadNodes
=================
*/
void Mod_LoadNodes (lump_t *l)
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}

		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);

		for (j=0 ; j<2 ; j++)
		{
			p = LittleShort (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}

	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

void Mod_ProcessLeafs (dleaf_t *in, int filelen);	// 2001-12-28 .VIS support by Maddes

/*
=================
Mod_LoadLeafs
=================
*/
void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t 	*in;
// 2001-12-28 .VIS support by Maddes  start
/*
	mleaf_t 	*out;
	int			i, j, count, p;
*/
// 2001-12-28 .VIS support by Maddes  end

	in = (void *)(mod_base + l->fileofs);
// 2001-12-28 .VIS support by Maddes  start
/*
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);
*/

	Mod_ProcessLeafs (in, l->filelen);
}

void Mod_ProcessLeafs (dleaf_t *in, int filelen)
{
	mleaf_t 	*out;
	int			i, j, count, p;

	if (filelen % sizeof(*in))
		Sys_Error ("Mod_ProcessLeafs: funny lump size in %s", loadmodel->name);
	count = filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), "USE_LEAF");
// 2001-12-28 .VIS support by Maddes  end

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}

		p = LittleLong(in->contents);
		out->contents = p;

		out->firstmarksurface = loadmodel->marksurfaces +
			LittleShort(in->firstmarksurface);
		out->nummarksurfaces = LittleShort(in->nummarksurfaces);

		p = LittleLong(in->visofs);
		if (p == -1)
			out->compressed_vis = NULL;
		else
			out->compressed_vis = loadmodel->visdata + p;
		out->efrags = NULL;

		for (j=0 ; j<4 ; j++)
			out->ambient_sound_level[j] = in->ambient_level[j];

#ifdef GLQUAKE
		// gl underwater warp
		if (out->contents != CONTENTS_EMPTY)
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
				out->firstmarksurface[j]->flags |= SURF_UNDERWATER;
		}
#endif
	}
}

/*
=================
Mod_LoadClipnodes
=================
*/
void Mod_LoadClipnodes (lump_t *l)
{
	dclipnode_t *in, *out;
	int			i, count;
	hull_t		*hull;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->clipnodes = out;
	loadmodel->numclipnodes = count;

	hull = &loadmodel->hulls[1];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 32;

	hull = &loadmodel->hulls[2];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -32;
	hull->clip_mins[1] = -32;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 32;
	hull->clip_maxs[1] = 32;
	hull->clip_maxs[2] = 64;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);
		out->children[0] = LittleShort(in->children[0]);
		out->children[1] = LittleShort(in->children[1]);
	}
}

/*
=================
Mod_MakeHull0

Deplicate the drawing hull structure as a clipping hull
=================
*/
void Mod_MakeHull0 (void)
{
	mnode_t		*in, *child;
	dclipnode_t *out;
	int			i, j, count;
	hull_t		*hull;

	hull = &loadmodel->hulls[0];

	in = loadmodel->nodes;
	count = loadmodel->numnodes;
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = in->plane - loadmodel->planes;
		for (j=0 ; j<2 ; j++)
		{
			child = in->children[j];
			if (child->contents < 0)
				out->children[j] = child->contents;
			else
				out->children[j] = child - loadmodel->nodes;
		}
	}
}

/*
=================
Mod_LoadMarksurfaces
=================
*/
void Mod_LoadMarksurfaces (lump_t *l)
{
	int		i, j, count;
	short		*in;
	msurface_t **out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleShort(in[i]);
		if (j >= loadmodel->numsurfaces)
			Sys_Error ("Mod_ParseMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
void Mod_LoadSurfedges (lump_t *l)
{
	int		i, count;
	int		*in, *out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for ( i=0 ; i<count ; i++)
		out[i] = LittleLong (in[i]);
}

/*
=================
Mod_LoadPlanes
=================
*/
void Mod_LoadPlanes (lump_t *l)
{
	int			i, j;
	mplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*2*sizeof(*out), loadname);

	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
	}
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (vec3_t mins, vec3_t maxs)
{
	int		i;
	vec3_t	corner;

	for (i=0 ; i<3 ; i++)
	{
		corner[i] = fabs(mins[i]) > fabs(maxs[i]) ? fabs(mins[i]) : fabs(maxs[i]);
	}

	return Length (corner);
}

// 2001-12-28 .VIS support by Maddes  start
/*
=================
Mod_LoadExternalVisibility
=================
*/
void Mod_LoadExternalVisibility (int fhandle)
{
	long	filelen;

	// get visibility data length
	filelen = 0;
	Sys_FileRead (fhandle, &filelen, 4);
	filelen = LittleLong(filelen);

	Con_Printf("...%i bytes visibility data\n", filelen);

	// load visibility data
	if (!filelen)
	{
		loadmodel->visdata = NULL;
		return;
	}
	loadmodel->visdata = Hunk_AllocName ( filelen, "EXT_VIS");
	Sys_FileRead (fhandle, loadmodel->visdata, filelen);
}

/*
=================
Mod_LoadExternalLeafs
=================
*/
void Mod_LoadExternalLeafs (int fhandle)
{
	dleaf_t 	*in;
	long	filelen;

	// get leaf data length
	filelen = 0;
	Sys_FileRead (fhandle, &filelen, 4);
	filelen = LittleLong(filelen);

	Con_Printf("...%i bytes leaf data\n", filelen);

	// load leaf data
	if (!filelen)
	{
		loadmodel->leafs = NULL;
		loadmodel->numleafs = 0;
		return;
	}
	in = Hunk_AllocName (filelen, "EXT_LEAF");
	Sys_FileRead (fhandle, in, filelen);

	Mod_ProcessLeafs (in, filelen);
}

int Mod_FindExternalVIS (loadedfile_t *brush_fileinfo)
{
	char	visfilename[1024];
	int		fhandle;
	int		len, i, pos;
	searchpath_t	*s_vis;
	vispatch_t	header;
	char	mapname[VISPATCH_MAPNAME_LENGTH+5];	// + ".vis" + EoS

	fhandle = -1;

	if (external_vis->value)
	{
		// check for a .VIS file
		strcpy(visfilename, loadmodel->name);
		COM_StripExtension(visfilename, visfilename);
		strcat(visfilename, ".vis");

		len = COM_OpenFile (visfilename, &fhandle, &s_vis);
		if (fhandle == -1)	// check for a .VIS file with map's directory name (e.g. ID1.VIS)
		{
			strcpy(visfilename, "maps/");
			strcat(visfilename, COM_SkipPath(brush_fileinfo->path->filename));
			strcat(visfilename, ".vis");
			len = COM_OpenFile (visfilename, &fhandle, &s_vis);
		}

		if (fhandle >= 0)
		{
			// check file for size
			if (len <= 0)
			{
				COM_CloseFile(fhandle);
				fhandle = -1;
			}
		}

		if (fhandle >= 0)
		{
			// search map in visfile
			strncpy(mapname, loadname, VISPATCH_MAPNAME_LENGTH);
			mapname[VISPATCH_MAPNAME_LENGTH] = 0;
			strcat(mapname, ".bsp");

			pos = 0;
			while ((i = Sys_FileRead (fhandle, &header, sizeof(struct vispatch_s))) == sizeof(struct vispatch_s))
			{
				header.filelen = LittleLong(header.filelen);

				pos += i;

				if (!Q_strncasecmp (header.mapname, mapname, VISPATCH_MAPNAME_LENGTH))	// found
				{
					break;
				}

				pos += header.filelen;

				Sys_FileSeek(fhandle, pos);
			}

			if (i != sizeof(struct vispatch_s))
			{
				COM_CloseFile(fhandle);
				fhandle = -1;
			}
		}

		if (fhandle >= 0)
		{
			Con_Printf("%s for %s loaded from %s\n", visfilename, mapname, s_vis->pack ? s_vis->pack->filename : s_vis->filename);
		}
	}

	return fhandle;
}
// 2001-12-28 .VIS support by Maddes  end

/*
=================
Mod_LoadBrushModel
=================
*/
void Mod_LoadBrushModel (model_t *mod, void *buffer, loadedfile_t *brush_fileinfo)	// 2001-09-12 .ENT support by Maddes
{
	int			i, j;
	dheader_t	*header;
	dmodel_t 	*bm;
	int			fhandle;	// 2001-12-28 .VIS support by Maddes

	loadmodel->type = mod_brush;

	header = (dheader_t *)buffer;

	i = LittleLong (header->version);
	if (i != BSPVERSION)
// 2001-12-16 No crash on wrong BSP version by MrG  start
//		Sys_Error ("Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, i, BSPVERSION);
	{
		Con_Printf("Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, i, BSPVERSION);
		mod->numvertexes=-1;	// HACK - incorrect BSP version is no longer fatal
		return;
	}
// 2001-12-16 No crash on wrong BSP version by MrG  end

// swap all the lumps
	mod_base = (byte *)header;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

// load into heap
	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadTextures (&header->lumps[LUMP_TEXTURES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces (&header->lumps[LUMP_MARKSURFACES]);
// 2001-12-28 .VIS support by Maddes  start
	loadmodel->visdata = NULL;
	loadmodel->leafs = NULL;
	loadmodel->numleafs = 0;

	fhandle = Mod_FindExternalVIS (brush_fileinfo);
	if (fhandle >= 0)
	{
		Mod_LoadExternalVisibility (fhandle);
		Mod_LoadExternalLeafs (fhandle);
	}

	if ((loadmodel->visdata == NULL)
	    || (loadmodel->leafs == NULL)
	    || (loadmodel->numleafs == 0))
	{
		if (fhandle >= 0)
		{
			Con_Printf("External VIS data are invalid!!!\n");
		}
// 2001-12-28 .VIS support by Maddes  end
		Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
		Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
// 2001-12-28 .VIS support by Maddes  start
	}

	if (fhandle >= 0)
	{
		COM_CloseFile(fhandle);
	}
// 2001-12-28 .VIS support by Maddes  end
	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadClipnodes (&header->lumps[LUMP_CLIPNODES]);
	Mod_LoadEntities (&header->lumps[LUMP_ENTITIES], brush_fileinfo);	// 2001-09-12 .ENT support by Maddes
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);

	Mod_MakeHull0 ();

	mod->numframes = 2;		// regular and alternate animation
#ifndef GLQUAKE
	mod->flags = 0;
#endif

//
// set up the submodels (FIXME: this is confusing)
//
	for (i=0 ; i<mod->numsubmodels ; i++)
	{
		bm = &mod->submodels[i];

		mod->hulls[0].firstclipnode = bm->headnode[0];
		for (j=1 ; j<MAX_MAP_HULLS ; j++)
		{
			mod->hulls[j].firstclipnode = bm->headnode[j];
			mod->hulls[j].lastclipnode = mod->numclipnodes-1;
		}

		mod->firstmodelsurface = bm->firstface;
		mod->nummodelsurfaces = bm->numfaces;

		VectorCopy (bm->maxs, mod->maxs);
		VectorCopy (bm->mins, mod->mins);
		mod->radius = RadiusFromBounds (mod->mins, mod->maxs);

		mod->numleafs = bm->visleafs;

		if (i < mod->numsubmodels-1)
		{	// duplicate the basic information
			char	name[10];

			sprintf (name, "*%i", i+1);
			loadmodel = Mod_FindName (name);
			*loadmodel = *mod;
			strcpy (loadmodel->name, name);
			mod = loadmodel;
		}
	}
}
