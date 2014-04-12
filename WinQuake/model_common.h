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
// 2001-12-28 Merged model functions by Maddes

#ifndef __MODEL_COMMON__
#define __MODEL_COMMON__

extern	model_t	*loadmodel;
extern	char	loadname[];	// for hunk tags

extern	byte	*mod_base;

extern	cvar_t	*external_lit;	// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes
extern	cvar_t	*external_ent;	// 2001-09-12 .ENT support by Maddes
extern	cvar_t	*external_vis;	// 2001-12-28 .VIS support by Maddes

void	Mod_Init (void);
//void	Mod_ClearAll (void);
model_t *Mod_ForName (char *name, qboolean crash);
void	*Mod_Extradata (model_t *mod);	// handles caching
//void	Mod_TouchModel (char *name);

mleaf_t *Mod_PointInLeaf (float *p, model_t *model);
byte	*Mod_LeafPVS (mleaf_t *leaf, model_t *model);

void Mod_LoadBrushModel (model_t *mod, void *buffer, loadedfile_t *fileinfo);	// 2001-09-12 .ENT support by Maddes

// 2001-12-28 .VIS support by Maddes  start
#define VISPATCH_MAPNAME_LENGTH	32

typedef struct vispatch_s {
	char	mapname[VISPATCH_MAPNAME_LENGTH];	// map for which these data are for, always use strncpy and strncmp for this field
	int		filelen;		// length of data after VisPatch header (VIS+Leafs)
} vispatch_t;
// 2001-12-28 .VIS support by Maddes  end


#endif	// __MODEL_COMMON__
