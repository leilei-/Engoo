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
// r_main.c

#include "quakedef.h"
#include "r_local.h"
#include "matrixlib.h"

//define	PASSAGES
oldrefdef_t	r_oldrefdef;
void		*colormap;
vec3_t		viewlightvec;
alight_t	r_viewlighting = {128, 192, (int)viewlightvec};
float		r_time1;
int			r_numallocatededges;
qboolean	r_drawpolys;
qboolean	r_drawculledpolys;
qboolean	r_worldpolysbacktofront;
qboolean	r_recursiveaffinetriangles = true;
int			r_pixbytes = 1;
float		r_aliasuvscale = 1.0;
int			r_outofsurfaces;
int			r_outofedges;

qboolean	r_dowarp, r_dowarpold, r_viewchanged, r_docrapold, r_doshrooms;
qboolean	inwat;	// leilei - for underwater sounds
int	r_docrap;

mplane_t	*mirror_plane;
int			numbtofpolys;
btofpoly_t	*pbtofpolys;
mvertex_t	*r_pcurrentvertbase;

int			c_surf;
int			r_maxsurfsseen, r_maxedgesseen, r_cnumsurfs;
qboolean	r_surfsonstack;
int			r_clipflags;

byte		*r_warpbuffer;
byte		*r_lowbuffer;
byte		*r_fogbuffer;
#ifdef WATERREFLECTIONS
byte		*r_reflectbuffer;
byte		*r_lowreflectbuffer;
byte		*r_warpreflectbuffer;
#endif
byte		*r_shadowbuffer;
/*
byte		*r_res2buffer;
byte		*r_res3buffer;
byte		*r_res4buffer;
byte		*r_res5buffer;
*/
extern cvar_t	*temp2;
byte		*r_stack_start;
cvar_t  *r_shadowhack;
qboolean	r_fov_greater_than_90;

//
// view origin
//
vec3_t	vup, base_vup;
vec3_t	vpn, base_vpn;
vec3_t	vright, base_vright;
vec3_t	r_origin;

//
// screen size info
//
refdef_t	r_refdef;
float		xcenter, ycenter;
float		xscale, yscale;
float		xscaleinv, yscaleinv;
float		xscaleshrink, yscaleshrink;
float		aliasxscale, aliasyscale, aliasxcenter, aliasycenter;

int		screenwidth;

float	pixelAspect;
float	screenAspect;
float	verticalFieldOfView;
float	xOrigin, yOrigin;

mplane_t	screenedge[4];

//
// refresh flags
//
int		r_framecount = 1;	// so frame counts initialized to 0 don't match
int		r_visframecount;
int		d_spanpixcount;
int		r_polycount;
int		r_drawnpolycount;
int		r_wholepolycount;
void R_ApplyFog(void);
void R_ApplyDof(void);
#define		VIEWMODNAME_LENGTH	256
char		viewmodname[VIEWMODNAME_LENGTH+1];
int			modcount;

int			*pfrustum_indexes[4];
int			r_frustum_indexes[4*6];

int		reinit_surfcache = 1;	// if 1, surface cache is currently empty and
								// must be reinitialized for current cache size

mleaf_t		*r_viewleaf, *r_oldviewleaf;

texture_t	*r_notexture_mip;

float		r_aliastransition, r_resfudge;

int		d_lightstylevalue[256];	// 8.8 fraction of base light value

float	dp_time1, dp_time2, db_time1, db_time2, rw_time1, rw_time2;
float	se_time1, se_time2, de_time1, de_time2
#ifndef VMTOC
 , dv_time1, dv_time2
#endif
 ;

void R_MarkLeaves (void);

cvar_t	*r_draworder;
cvar_t	*r_speeds;
cvar_t	*r_timegraph;
cvar_t	*r_graphheight;
cvar_t	*r_clearcolor;
cvar_t	*r_waterwarp;
cvar_t	*r_fullbright;
cvar_t	*r_drawentities;
cvar_t	*r_drawviewmodel;
cvar_t	*r_aliasstats;
cvar_t	*r_dspeeds;
cvar_t	*r_drawflat;
cvar_t	*r_ambient;
cvar_t	*r_reportsurfout;
cvar_t	*r_maxsurfs;
cvar_t	*r_numsurfs;
cvar_t	*r_reportedgeout;
cvar_t	*r_maxedges;
cvar_t	*r_numedges;
cvar_t	*r_aliastransbase;
cvar_t	*r_aliastransadj;
cvar_t  *r_lerpmodels;
cvar_t  *r_lerpmove;
cvar_t  *r_lowdetail;
cvar_t  *r_lowworld;
cvar_t  *r_filter;
cvar_t  *r_shading;
cvar_t  *r_shift1;	// Only used for debugging
cvar_t  *r_shift2;	// Too
cvar_t  *r_lowmodels;
cvar_t  *r_coloredlights;
cvar_t  *r_coloredlightmethod;
cvar_t  *r_shiftlights;
cvar_t  *r_particlesort;
cvar_t  *r_depthoffield;
cvar_t  *r_truecolor;
cvar_t  *r_overbrightBits;
cvar_t  *r_fullbrights;
cvar_t  *r_overbrightmdl;
cvar_t  *r_coloreddyns;
cvar_t  *r_menucolor;
cvar_t  *r_tinge;
cvar_t  *r_tingecolor;
cvar_t  *r_dynamic;
cvar_t  *r_waterblend; // 0 - alpha 1 - additive 2 - multiply 3 - gelmap (ewwww!)
cvar_t  *r_tranquality; // 0 - use palmap(32768) 1 - use d_15to8 (65536) 2 - use d_8to24?
cvar_t  *r_lightquality; // 0 - less dynlight calcs 1 - normal dynlight calcs 
cvar_t  *r_shadequality; // 0 - 
cvar_t  *r_shadedither; // 0 - normal	1- noise dither 2- some dither 2 3- some dither 3
cvar_t  *r_shinygrays; 
cvar_t  *r_fogquality; // 0 - quick fog precision 1 - precise fog precision (lol redundancy)
cvar_t  *r_waterquality; // 0 - stabdard 1 - refractions 2. reflections and refractions
cvar_t  *r_fogdither; // 0 - fog is a normal 64 rows 1 - fog is dithered into more rows 2 - fog is dithered even further
cvar_t  *r_dither; // 0 - lookups 1 - ditherups

cvar_t  *r_virtualmode;	// 0 - Off
						// 1 - 160x100 
					    // 2 - 320x200
					    // 3 - 320x400
						// 4 - 360x480
						// 5 - 640x400

cvar_t  *r_alphashift;

cvar_t  *r_particlespray;
cvar_t  *r_particleblood;
cvar_t  *r_particlegunshot;
cvar_t  *r_particlesparks;
cvar_t  *r_particletrails;
cvar_t  *r_particlebloodfade;
cvar_t  *r_particletrans;
cvar_t  *r_particleset;
cvar_t  *r_flares;
cvar_t  *r_flamehack;
cvar_t  *r_particlelit;
cvar_t  *r_particlesprite;
cvar_t  *r_muzzlehack;
extern cvar_t	*scr_fov;

void CreatePassages (void);
void SetVisibilityByPassages (void);

/*
==================
R_InitTextures
==================
*/
void	R_InitTextures (void)
{
	int		x,y, m;
	byte	*dest;

// create a simple checkerboard texture for the default
	r_notexture_mip = Hunk_AllocName (sizeof(texture_t) + 16*16+8*8+4*4+2*2, "notexture");

	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->offsets[0] = sizeof(texture_t);
	r_notexture_mip->offsets[1] = r_notexture_mip->offsets[0] + 16*16;
	r_notexture_mip->offsets[2] = r_notexture_mip->offsets[1] + 8*8;
	r_notexture_mip->offsets[3] = r_notexture_mip->offsets[2] + 4*4;

	for (m=0 ; m<4 ; m++)
	{
		dest = (byte *)r_notexture_mip + r_notexture_mip->offsets[m];
		for (y=0 ; y< (16>>m) ; y++)
			for (x=0 ; x< (16>>m) ; x++)
			{
				if (  (y< (8>>m) ) ^ (x< (8>>m) ) ) // leilei - grey checkerboard
					*dest++ = 4;// was 0
				else
					*dest++ = 8;// was 0xff;
			}
	}
}
extern cvar_t *r_novis;
// 2001-09-18 New cvar system by Maddes (Init)  start
/*
===============
R_Init_Cvars
===============
*/
void R_Init_Cvars (void)
{
	r_draworder = Cvar_Get ("r_draworder", "0", CVAR_ORIGINAL);
	r_speeds = Cvar_Get ("r_speeds", "0", CVAR_ORIGINAL);
	r_timegraph = Cvar_Get ("r_timegraph", "0", CVAR_ORIGINAL);
	r_graphheight = Cvar_Get ("r_graphheight", "10", CVAR_ORIGINAL);
	r_drawflat = Cvar_Get ("r_drawflat", "0", CVAR_ORIGINAL);
	r_ambient = Cvar_Get ("r_ambient", "0", CVAR_ORIGINAL);
	r_clearcolor = Cvar_Get ("r_clearcolor", "2", CVAR_ORIGINAL);
	r_waterwarp = Cvar_Get ("r_waterwarp", "0", CVAR_ORIGINAL);	
	r_fullbright = Cvar_Get ("r_fullbright", "0", CVAR_ORIGINAL);
	r_drawentities = Cvar_Get ("r_drawentities", "1", CVAR_ORIGINAL);
	r_drawviewmodel = Cvar_Get ("r_drawviewmodel", "1", CVAR_ORIGINAL);
	r_aliasstats = Cvar_Get ("r_polymodelstats", "0", CVAR_ORIGINAL);
	r_dspeeds = Cvar_Get ("r_dspeeds", "0", CVAR_ORIGINAL);
	r_reportsurfout = Cvar_Get ("r_reportsurfout", "0", CVAR_ORIGINAL);
	r_maxsurfs = Cvar_Get ("r_maxsurfs", "0", CVAR_ORIGINAL);
	r_numsurfs = Cvar_Get ("r_numsurfs", "0", CVAR_ORIGINAL);
	r_reportedgeout = Cvar_Get ("r_reportedgeout", "0", CVAR_ORIGINAL);
	r_maxedges = Cvar_Get ("r_maxedges", "0", CVAR_ORIGINAL);
	r_numedges = Cvar_Get ("r_numedges", "0", CVAR_ORIGINAL);
	r_aliastransbase = Cvar_Get ("r_aliastransbase", "200", CVAR_ORIGINAL);
	r_aliastransadj = Cvar_Get ("r_aliastransadj", "100", CVAR_ORIGINAL);
	r_novis = Cvar_Get ("r_novis", "0", CVAR_ORIGINAL);
	
	r_virtualmode = Cvar_Get ("r_virtualmode", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_filter = Cvar_Get ("r_filter", "0", CVAR_ARCHIVE|CVAR_ORIGINAL);
	r_shading = Cvar_Get ("r_shading", "1", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_lowworld = Cvar_Get ("r_lowworld", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_lowmodels = Cvar_Get ("r_lowmodels", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_coloredlights = Cvar_Get ("r_coloredlights", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_coloredlightmethod = Cvar_Get ("r_coloredlightmethod", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_shiftlights = Cvar_Get ("r_shiftlights", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particlesort = Cvar_Get ("r_particlesort", "1", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_depthoffield = Cvar_Get ("r_depthoffield", "0", CVAR_ORIGINAL);
	r_fogdither = Cvar_Get ("r_fogdither", "2", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_fogquality = Cvar_Get ("r_fogquality", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_waterquality = Cvar_Get ("r_waterquality", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_truecolor = Cvar_Get ("r_truecolor", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_overbrightBits = Cvar_Get ("r_overbrightBits", "1", CVAR_ORIGINAL);
	r_fullbrights = Cvar_Get ("r_fullbrights", "1", CVAR_ORIGINAL);
	r_coloreddyns = Cvar_Get ("r_coloreddyns", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_shift1 = Cvar_Get ("sh1", "1", CVAR_ORIGINAL); // debugging
	r_shift2 = Cvar_Get ("sh2", "2", CVAR_ORIGINAL);
	r_waterblend = Cvar_Get ("r_waterblend", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_tranquality = Cvar_Get ("r_tranquality", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_lightquality = Cvar_Get ("r_lightquality", "1", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_shadequality = Cvar_Get ("r_shadequality", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_shadedither = Cvar_Get ("r_shadedither", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_shinygrays = Cvar_Get ("r_shinygrays", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_dither = Cvar_Get ("r_dither", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_menucolor = Cvar_Get ("r_menucolor", "1",CVAR_ARCHIVE | CVAR_ORIGINAL);
	r_tingecolor = Cvar_Get ("r_tingecolor", "10",CVAR_ARCHIVE | CVAR_ORIGINAL);
	r_tinge = Cvar_Get ("r_tinge", "0",CVAR_ARCHIVE | CVAR_ORIGINAL);
	r_alphashift = Cvar_Get ("r_alphashift", "0", CVAR_ORIGINAL);
	r_lowworld = Cvar_Get ("r_lowworld", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_shadowhack = Cvar_Get ("r_shadowhack", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_overbrightmdl = Cvar_Get ("r_overbrightmdl", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particlespray = Cvar_Get ("r_particlespray", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particleblood = Cvar_Get ("r_particleblood", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particlegunshot = Cvar_Get ("r_particlegunshot", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particlesparks = Cvar_Get ("r_particlesparks", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particletrails = Cvar_Get ("r_particletrails", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particlebloodfade = Cvar_Get ("r_particlebloodfade", "0.5", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particletrans = Cvar_Get ("r_particletrans", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particleset = Cvar_Get ("r_particleset", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_flares = Cvar_Get ("r_flares", "1", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_flamehack = Cvar_Get ("r_flamehack", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particlesprite = Cvar_Get ("r_particlesprite", "1", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_muzzlehack = Cvar_Get ("r_muzzlehack", "0", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_dynamic = Cvar_Get ("r_dynamic", "1", CVAR_ARCHIVE |CVAR_ORIGINAL);
	r_particlelit = Cvar_Get ("r_particlelit", "0", CVAR_ORIGINAL);
	Cvar_SetValue (r_maxedges, (float)NUMSTACKEDGES);
	Cvar_SetValue (r_maxsurfs, (float)NUMSTACKSURFACES);


}
// 2001-09-18 New cvar system by Maddes (Init)  end
extern int overbrightmdl;

/*
===============
R_Init
===============
*/
void R_Init (void)
{
	int		dummy;
//		FILE	*f;

// get stack position so we can guess if we are going to overflow
	r_stack_start = (byte *)&dummy;

	R_InitTurb ();
	{
	}
	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f);
	Cmd_AddCommand ("pointfile", R_ReadPointFile_f);

	view_clipplanes[0].leftedge = true;
	view_clipplanes[1].rightedge = true;
	view_clipplanes[1].leftedge = view_clipplanes[2].leftedge =
			view_clipplanes[3].leftedge = false;
	view_clipplanes[0].rightedge = view_clipplanes[2].rightedge =
			view_clipplanes[3].rightedge = false;

	r_refdef.xOrigin = XCENTERING;
	r_refdef.yOrigin = YCENTERING;

	R_InitParticles ();
	R_InitFlares ();


if (COM_CheckParm("-old320200"))
	screenfake = 1;

if (COM_CheckParm("-old320400"))
	screenfake = 2;

if (COM_CheckParm("-old360480"))
	screenfake = 3;

if (COM_CheckParm("-old640400"))
	screenfake = 4;


// TODO: collect 386-specific code in one place
#if	id386broken
	Sys_MakeCodeWriteable ((long)R_EdgeCodeStart, (long)R_EdgeCodeEnd - (long)R_EdgeCodeStart);
#endif	// id386

#if id386fog
	//Sys_MakeCodeWriteable ((int)D_Draw16StartFog, (int)D_Draw16EndFog - (int)D_Draw16StartFog);
	R_FogPatch();
#endif

	D_Init ();
}


void R_Presets (void)
{
	if (COM_CheckParm ("-glsuck")){
//	Cvar_SetValue (r_overbrightBits, 0);
//	Cvar_SetValue (r_fullbrights, 0);
//	Cvar_SetValue (r_alphashift, 1);
//	Cvar_SetValue (r_filter, 1);
	}

};
/*
===============
R_NewMap
===============
*/
void FogStuffs (void);
void Fog_ParseWorldspawn (void);
extern int lightingavailable;
extern int lightingcantbeavailable;
void R_NewMap (void)
{
	int		i;

// clear out efrags in case the level hasn't been reloaded
// FIXME: is this one short?
	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
		cl.worldmodel->leafs[i].efrags = NULL;

	r_viewleaf = NULL;
	R_ClearParticles ();
	R_ClearFlares ();
	lightingavailable = 0;
	lightingcantbeavailable = 0;
	
	Fog_ParseWorldspawn();
	FogStuffs();	// gen new fog, read fog, etc etc

	r_cnumsurfs = r_maxsurfs->value;

	if (r_cnumsurfs <= MINSURFACES)
		r_cnumsurfs = MINSURFACES;

	if (r_cnumsurfs > NUMSTACKSURFACES)
	{
		surfaces = Hunk_AllocName (r_cnumsurfs * sizeof(surf_t), "surfaces");
		surface_p = surfaces;
		surf_max = &surfaces[r_cnumsurfs];
		r_surfsonstack = false;
	// surface 0 doesn't really exist; it's just a dummy because index 0
	// is used to indicate no edge attached to surface
		surfaces--;
//		R_SurfacePatch ();
	}
	else
	{
		r_surfsonstack = true;
	}

	r_maxedgesseen = 0;
	r_maxsurfsseen = 0;

	r_numallocatededges = r_maxedges->value;

	if (r_numallocatededges < MINEDGES)
		r_numallocatededges = MINEDGES;

	if (r_numallocatededges <= NUMSTACKEDGES)
	{
		auxedges = NULL;
	}
	else
	{
		auxedges = Hunk_AllocName (r_numallocatededges * sizeof(edge_t),
								   "edges");
	}

	r_dowarpold = false;
	r_viewchanged = false;
#ifdef PASSAGES
CreatePassages ();
#endif
#ifdef INTERPOL7
	R_FinalizeAliasVerts ();
#endif

	// leilei - sun hack
	// TODO: Determine origin of sun from the map's sunlight key
	// TODO2 : very obnoxious lens reflections
/*	
	{
		vec3_t	sun;
		sun[0] = 1750;
		sun[1] = 1750;
		sun[2] = 1750;
			R_FlareTest(sun, 1, 255, 255, 180, 0, NULL);
	}
*/
}


/*
===============
R_SetVrect
===============
*/
void R_SetVrect (vrect_t *pvrectin, vrect_t *pvrect, int lineadj)
{
	int		h;
	float	size;
#ifdef SPLIT
	int curplayer = cursplit = 2;
#endif
	qboolean full = false;
	
	if (scr_viewsize->value >= 100.0) {
		size = 100.0;
		full = true;
	} else
		size = scr_viewsize->value;

	size = scr_viewsize->value > 100 ? 100 : scr_viewsize->value;
	if (cl.intermission)
	{
		size = 100;
		lineadj = 0;
	}
	size /= 100.0;			// 2000-01-07 Border with viewsize 100 fix by Radix
	if (!cl_sbar->value && full)
		h = pvrectin->height;
	else
	h = pvrectin->height - lineadj;
		if (full)
		pvrect->width = pvrectin->width;
	else
	pvrect->width = pvrectin->width * size;
	if (pvrect->width < 96)
	{
		size = 96.0 / pvrectin->width;
		pvrect->width = 96;	// min for icons
	}
	pvrect->width &= ~7;
	pvrect->height = pvrectin->height * size;
	if (cl_sbar->value || !full) {
		if (pvrect->height > pvrectin->height - lineadj)
			pvrect->height = pvrectin->height - lineadj;
	} else
		if (pvrect->height > pvrectin->height)
			pvrect->height = pvrectin->height;

	pvrect->height &= ~1;

	pvrect->x = (pvrectin->width - pvrect->width)/2;
	
	if (full)
		pvrect->y = 0;
	else
		pvrect->y = (h - pvrect->height)/2;

	
	{
		if (lcd_x->value)
		{
			pvrect->y >>= 1;
			pvrect->height >>= 1;
		}
#ifdef SPLIT
		if (curplayer == 1){
			pvrect->y >>= 1;
			pvrect->height >>= 1;
		}
		else if (curplayer == 2){
			
			pvrect->y >>= 1;
			pvrect->height >>= 1;
		}
#endif
	}

		{
		if (v_detail->value == 1)
		{
			pvrect->y >>= 1;
			pvrect->height >>= 1;
		}
		if (v_detail->value == 2)
		{
			pvrect->x >>= 1;
			pvrect->width >>= 1;
		}

	}
}


/*
===============
R_ViewChanged

Called every time the vid structure or r_refdef changes.
Guaranteed to be called before the first refresh

===============
*/
extern cvar_t *scr_fov_adapt;
void R_ViewChanged (vrect_t *pvrect, int lineadj, float aspect)
{
	int		i;
	float	res_scale;

	r_viewchanged = true;
	lineadj *= (vid.conheight / vid.vconheight);	 // leilei - PLEASE FIX ME HERE!
//	R_SetVrect (pvrect, &r_refdef.vrect, lineadj);

	
	r_refdef.horizontalFieldOfView = 2.0 * tan (r_refdef.fov_x/360*M_PI);
	
	r_refdef.fvrectx = (float)r_refdef.vrect.x;
	r_refdef.fvrectx_adj = (float)r_refdef.vrect.x - 0.5;
	r_refdef.vrect_x_adj_shift20 = (r_refdef.vrect.x<<20) + (1<<19) - 1;
	r_refdef.fvrecty = (float)r_refdef.vrect.y;
	r_refdef.fvrecty_adj = (float)r_refdef.vrect.y - 0.5;
	r_refdef.vrectright = r_refdef.vrect.x + r_refdef.vrect.width;
	r_refdef.vrectright_adj_shift20 = (r_refdef.vrectright<<20) + (1<<19) - 1;
	r_refdef.fvrectright = (float)r_refdef.vrectright;
	r_refdef.fvrectright_adj = (float)r_refdef.vrectright - 0.5;
	r_refdef.vrectrightedge = (float)r_refdef.vrectright - 0.99;
	r_refdef.vrectbottom = r_refdef.vrect.y + r_refdef.vrect.height;
	r_refdef.fvrectbottom = (float)r_refdef.vrectbottom;
	r_refdef.fvrectbottom_adj = (float)r_refdef.vrectbottom - 0.5;

	r_refdef.aliasvrect.x = (int)(r_refdef.vrect.x * r_aliasuvscale);
	r_refdef.aliasvrect.y = (int)(r_refdef.vrect.y * r_aliasuvscale);
	r_refdef.aliasvrect.width = (int)(r_refdef.vrect.width * r_aliasuvscale);
	r_refdef.aliasvrect.height = (int)(r_refdef.vrect.height * r_aliasuvscale);
	r_refdef.aliasvrectright = r_refdef.aliasvrect.x +
			r_refdef.aliasvrect.width;
	r_refdef.aliasvrectbottom = r_refdef.aliasvrect.y +
			r_refdef.aliasvrect.height;

//	pixelAspect = aspect;
	xOrigin = r_refdef.xOrigin;
	yOrigin = r_refdef.yOrigin;

//	screenAspect = r_refdef.vrect.width*pixelAspect / r_refdef.vrect.height;

//	verticalFieldOfView = r_refdef.horizontalFieldOfView / screenAspect;
 	if (scr_fov_adapt->value)
	{
		pixelAspect = 1;
		verticalFieldOfView = 2.0 * tan (r_refdef.fov_y/360*M_PI);
	}
	else
	{
		pixelAspect = aspect;
		screenAspect = r_refdef.vrect.width*pixelAspect / r_refdef.vrect.height;
		verticalFieldOfView = r_refdef.horizontalFieldOfView / screenAspect;
	}


// values for perspective projection
// if math were exact, the values would range from 0.5 to to range+0.5
// hopefully they wll be in the 0.000001 to range+.999999 and truncate
// the polygon rasterization will never render in the first row or column
// but will definately render in the [range] row and column, so adjust the
// buffer origin to get an exact edge to edge fill
	xcenter = ((float)r_refdef.vrect.width * XCENTERING) +
			r_refdef.vrect.x - 0.5;
	aliasxcenter = xcenter * r_aliasuvscale;
	ycenter = ((float)r_refdef.vrect.height * YCENTERING) +
			r_refdef.vrect.y - 0.5;
	aliasycenter = ycenter * r_aliasuvscale;

	xscale = r_refdef.vrect.width / r_refdef.horizontalFieldOfView;
	aliasxscale = xscale * r_aliasuvscale;
	xscaleinv = 1.0 / xscale;
	yscale = xscale * pixelAspect;
	yscale = (scr_fov_adapt->value) ? r_refdef.vrect.height / verticalFieldOfView :
	xscale * pixelAspect;
	aliasyscale = yscale * r_aliasuvscale;
	yscaleinv = 1.0 / yscale;
	xscaleshrink = (r_refdef.vrect.width-6)/r_refdef.horizontalFieldOfView;
	yscaleshrink = xscaleshrink*pixelAspect;

    // blahtest
	
	

// left side clip
	screenedge[0].normal[0] = -1.0 / (xOrigin*r_refdef.horizontalFieldOfView);
	screenedge[0].normal[1] = 0;
	screenedge[0].normal[2] = 1;
	screenedge[0].type = PLANE_ANYZ;

// right side clip
	screenedge[1].normal[0] =
			1.0 / ((1.0-xOrigin)*r_refdef.horizontalFieldOfView);
	screenedge[1].normal[1] = 0;
	screenedge[1].normal[2] = 1;
	screenedge[1].type = PLANE_ANYZ;

// top side clip
	screenedge[2].normal[0] = 0;
	screenedge[2].normal[1] = -1.0 / (yOrigin*verticalFieldOfView);
	screenedge[2].normal[2] = 1;
	screenedge[2].type = PLANE_ANYZ;

// bottom side clip
	screenedge[3].normal[0] = 0;
	screenedge[3].normal[1] = 1.0 / ((1.0-yOrigin)*verticalFieldOfView);
	screenedge[3].normal[2] = 1;
	screenedge[3].type = PLANE_ANYZ;

	for (i=0 ; i<4 ; i++)
		VectorNormalize (screenedge[i].normal);

	res_scale = sqrt ((double)(r_refdef.vrect.width * r_refdef.vrect.height) /
						(320.0 * 152.0)) * (2.0 / r_refdef.horizontalFieldOfView);
	r_aliastransition = r_aliastransbase->value * res_scale;
	r_resfudge = r_aliastransadj->value * res_scale;

	if (scr_fov->value <= 90.0)
		r_fov_greater_than_90 = false;
	else
		r_fov_greater_than_90 = true;

// TODO: collect 386-specific code in one place
#if	id386
		
		
		if (lowworld){
		Sys_MakeCodeWriteable ((long)R_Surf8FastStart, (long)R_Surf8FastEnd - (long)R_Surf8FastStart);
		R_Surf8FastPatch ();
		colormap = vid.colormap;
		}
#if id386rgb
		else if (coloredlights)
		{

		Sys_MakeCodeWriteable ((long)R_Surf8RGBASMStart, (long)R_Surf8RGBASMEnd - (long)R_Surf8RGBASMStart);
		R_Surf8PatchRGB ();

		colormap = vid.colormap;
		}
#endif
		else
		{
		Sys_MakeCodeWriteable ((long)R_Surf8Start, (long)R_Surf8End - (long)R_Surf8Start);
		R_Surf8Patch ();
		colormap = vid.colormap;
		}
#endif	// id386

	D_ViewChanged ();
}

cvar_t	*r_novis;


/*
===============
R_MarkLeaves
===============
*/
void R_MarkLeaves (void)
{
	byte	fatvis[MAX_MAP_LEAFS/8];
	byte	*vis;
	mnode_t	*node;
	int		i;
	byte	solid[4096];

	if (r_oldviewleaf == r_viewleaf)
		return;

	

	r_visframecount++;
	r_oldviewleaf = r_viewleaf;
	if (reflectpass){
			vis = Mod_LeafPVS (r_viewleaf, cl.worldmodel);

	}
	else if (r_novis->value)
	{
		vis = solid;
		memset (solid, 0xff, (cl.worldmodel->numleafs+7)>>3);
	}
	else
	vis = Mod_LeafPVS (r_viewleaf, cl.worldmodel);

	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
	{
		if (vis[i>>3] & (1<<(i&7)))
		{
			node = (mnode_t *)&cl.worldmodel->leafs[i+1];
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
}
#ifdef VMTOC

/*
===============
R_DrawAliasEntity
===============
*/
static void R_DrawAliasEntity(qboolean viewmodel)
{
	float lightvec[3] = {-1, 0, 0}; // FIXME: remove and do real lighting
	
	int j;
	alight_t lighting;
	int lnum;
	dlight_t *dl;
	vec3_t dist;
	float add;

	if (!currententity->model)
		return;
		currententity->leifect = 0;
	if (viewmodel)
	{
		if (!r_drawviewmodel->value || chase_active->value || cl.intermission || reflectpass)
			return;
		if (cl.items & IT_INVISIBILITY)
			return;
		if (cl.stats[STAT_HEALTH] <= 0)
			return;
	}

	VectorCopy(currententity->origin, r_entorigin);
	VectorSubtract(r_origin, r_entorigin, modelorg);

// see if the bounding box lets us trivially reject, also sets trivial accept status
	if (!R_AliasCheckBBox())
		return;


	j = R_LightPoint(currententity->origin);

	if (viewmodel)
	{
	// always give some light on gun
		if (j < 24)
			j = 24;
	}

	lighting.ambientlight = j;
	lighting.shadelight = j;

	lighting.plightvec = lightvec;

	if (coloredlights){
	lighting.ramb = j;
	lighting.gamb = j;
	lighting.bamb = j;
	lighting.rlightvec = rlightvec;
	lighting.glightvec = glightvec;
	lighting.blightvec = blightvec;
	}
// add dynamic lights
	for (lnum = 0, dl = cl_dlights; lnum < MAX_DLIGHTS; lnum++, dl++)
	{
		if (!dl->radius)
			continue;
		if (dl->unmark) // leilei - skip world light
			continue;
		if (dl->die < cl.time)
			continue;

		VectorSubtract(currententity->origin, dl->origin, dist);
		add = dl->radius - Length(dist);
		if (add > 0){
			lighting.ambientlight += (int)add;
			lighting.ramb += ((int)add * (dl->color[0] / 255);
			lighting.gamb += ((int)add * (dl->color[1] / 255);
			lighting.bamb += ((int)add * (dl->color[2] / 255);

	}

// clamp lighting so it doesn't overbright as much
	if (lighting.ambientlight > 128)
		lighting.ambientlight = 128;
	if (lighting.ramb > 128)
		lighting.ramb = 128;
	if (lighting.gamb > 128)
		lighting.gamb = 128;
	if (lighting.bamb > 128)
		lighting.bamb = 128;
	if (lighting.ambientlight + lighting.shadelight > 192)
		lighting.shadelight = 192 - lighting.ambientlight;
	if (lighting.ramb + lighting.rlight > 192)
		lighting.rlight = 192 - lighting.ramb;
	if (lighting.gamb + lighting.glight > 192)
		lighting.glight = 192 - lighting.gamb;
	if (lighting.bamb + lighting.blight > 192)
		lighting.blight = 192 - lighting.bamb;
	
	R_AliasDrawModel(&lighting, viewmodel);
}

/*
===============
R_DrawViewModel
===============
*/
static void R_DrawViewModel(void)
{
	entity_t viewent;

	memset(&viewent, 0, sizeof(viewent));

	VectorCopy(cl.weapon_origin, viewent.origin);
	VectorCopy(cl.weapon_angles, viewent.angles);

	viewent.model = cl.model_precache[cl.stats[STAT_WEAPON]];
	viewent.frame = cl.stats[STAT_WEAPONFRAME];


// draw model
	currententity = &viewent;
#ifdef SCALEE
	currententity->scale2 = 1.0f;
#endif
	R_DrawAliasEntity(true);
}

/*
=============
R_DrawEntitiesOnList
=============
*/
void R_DrawEntitiesOnList(void)
{
	int i;

// if r_drawentities is false, don't draw entities (including weaponmodels)
	if (!r_drawentities->value)
		return;

// draw the regular weaponmodel

	R_DrawViewModel();

// draw entities
	for (i = 0; i < cl_numvisedicts; i++)
	{
		currententity = cl_visedicts[i];

		if (currententity == &cl_entities[cl.viewentity] && !chase_active->value && !reflectpass)
		{
		// don't draw the player
			continue;
		}

		switch (currententity->model->type)
		{
		case mod_sprite:
			VectorCopy(currententity->origin, r_entorigin);
			VectorSubtract(r_origin, r_entorigin, modelorg);
			R_DrawSprite();
			break;

		case mod_alias:
			if (currententity->viewmodel)
			{
				vec3_t oldorigin, oldangles;
				vec3_t forward, unused, up;
				matrix4x4_t mat, mat2, mat3;

				VectorCopy(currententity->origin, oldorigin);
				VectorCopy(currententity->angles, oldangles);

				Matrix4x4_CreateFromQuakeEntity(&mat, cl.weapon_origin[0], cl.weapon_origin[1], cl.weapon_origin[2], -cl.weapon_angles[0], cl.weapon_angles[1], cl.weapon_angles[2], 1.0f);
				Matrix4x4_CreateFromQuakeEntity(&mat2, oldorigin[0], oldorigin[1], oldorigin[2], -oldangles[0], oldangles[1], oldangles[2], 1.0f);
				Matrix4x4_Concat(&mat3, &mat, &mat2);

				Matrix4x4_ToVectors(&mat3, forward, unused, up, currententity->origin);
				AnglesFromVectors(currententity->angles, forward, up, true);

				R_DrawAliasEntity(true);

				VectorCopy(oldorigin, currententity->origin);
				VectorCopy(oldangles, currententity->angles);
			}
			else
			{

				R_DrawAliasEntity(false);

			}

			break;

		default:
			break;
		}
	}
}


#else
/*
=============
R_DrawEntitiesOnList
=============
*/
extern vec3_t	lightcolor;	// used by model rendering
extern cvar_t	*r_shading;
extern cvar_t	*temp1;
extern int deathcam_yesiamdead;
void R_DrawEntitiesOnList (void)
{
	int			i, j;
	vec3_t		rgb;
	int			lnum;
	alight_t	lighting;
// FIXME: remove and do real lighting
	float		lightvec[3] = {-1, 0, 0};
	float		rlightvec[3] = {-1, 0, 0};
	float		glightvec[3] = {-1, 0, 0};
	float		blightvec[3] = {-1, 0, 0};

	// TEST

//	float		rlightvec[3] = {3, 0, 0};
//	float		glightvec[3] = {0, 3, 0};
//	float		blightvec[3] = {0, 0, 3};

//	float		lightvec[3] = {-1, -1, -1};
	vec3_t		dist;
	float		add;

	if (!r_drawentities->value)
		return;

	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = cl_visedicts[i];

		if (currententity == &cl_entities[cl.viewentity])
// 2000-01-09 ChaseCam fix by FrikaC  start
		{
			if (!chase_active->value && !reflectpass && !deathcam_yesiamdead)
			{
// 2000-01-09 ChaseCam fix by FrikaC  end
				continue;	// don't draw the player
// 2000-01-09 ChaseCam fix by FrikaC  start
			}
			else
			{
				currententity->angles[0] *= 0.3;
			}
		}
// 2000-01-09 ChaseCam fix by FrikaC  end

		switch (currententity->model->type)
		{
		case mod_sprite:
			VectorCopy (currententity->origin, r_entorigin);
			VectorSubtract (r_origin, r_entorigin, modelorg);
			
			R_DrawSprite ();
			break;
//VectorSubtract( backEnd.viewParms.or.origin, point, local );
		case mod_alias:
			VectorCopy (currententity->origin, r_entorigin);
			VectorSubtract (r_origin, r_entorigin, modelorg);

		// see if the bounding box lets us trivially reject, also sets
		// trivial accept status
			if (R_AliasCheckBBox ())
			{
				
				j = R_LightPoint (currententity->origin);

				lighting.ambientlight = j;
				lighting.shadelight = j;
				lighting.plightvec = lightvec;
				if(coloredlights){
				lighting.ramb = (int)lightcolor[0];
				lighting.gamb = (int)lightcolor[1];
				lighting.bamb = (int)lightcolor[2];
				lighting.rlight = (int)lightcolor[0];
				lighting.glight = (int)lightcolor[1];
				lighting.blight = (int)lightcolor[2];
	
				
				lighting.rlightvec = rlightvec;
				lighting.glightvec = glightvec;
				lighting.blightvec = blightvec;
				


				for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
				{
					if (cl_dlights[lnum].die >= cl.time && !cl_dlights[lnum].unmark)
					{
						VectorSubtract (currententity->origin,
										cl_dlights[lnum].origin,
										dist);
						add = cl_dlights[lnum].radius - Length(dist);

						if (add > 0){
							lighting.ambientlight += add;
							if(coloredlights){
							lighting.ramb += ((int)add * (cl_dlights[lnum].color[0]));
							lighting.gamb += ((int)add * (cl_dlights[lnum].color[1]));
							lighting.bamb += ((int)add * (cl_dlights[lnum].color[2]));
							}
						}
					}
				}

				}
				else
				{

				for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
				{
					if (cl_dlights[lnum].die >= cl.time && !cl_dlights[lnum].unmark)
					{
						VectorSubtract (currententity->origin,
										cl_dlights[lnum].origin,
										dist);
						add = cl_dlights[lnum].radius - Length(dist);

						if (add > 0){
							lighting.ambientlight += add;
						}
					}
				}
				}
				if (!overbrightmdl){
			// clamp lighting so it doesn't overbright as much
				if (lighting.ambientlight > 128)
					lighting.ambientlight = 128;
				if (lighting.ambientlight + lighting.shadelight > 192)
					lighting.shadelight = 192 - lighting.ambientlight;
				if(coloredlights){
					if (lighting.ramb + lighting.ramb > 192)
					lighting.rlight = 192 - lighting.ramb;
					if (lighting.gamb + lighting.gamb > 192)
					lighting.glight = 192 - lighting.gamb;
					if (lighting.bamb + lighting.bamb > 192)
					lighting.blight = 192 - lighting.bamb;
					if(!overbrights){
					if (lighting.ramb + lighting.ramb > 128)
					lighting.rlight = 128 - lighting.ramb;
					if (lighting.gamb + lighting.gamb > 128)
					lighting.glight = 128 - lighting.gamb;
					if (lighting.bamb + lighting.bamb > 128)
					lighting.blight = 128 - lighting.bamb;

					}
				}
								
				


				}

				// HACK - apply flares to static fires and torches.
				// TODO: make this only work on makestatic (do it somewhere else? loadpointlight?)
#ifdef REALLYCRASHESITHINK
				if (!cls.demoplayback && !cl.paused && !reflectpass){		// leilei - HACK - don't try this at demo (Crash workaround)
			// Torch
			if (!strcmp (currententity->model->name, "progs/flame.mdl"))
				{
				vec3_t	offsetup;
				if(r_flamehack->value == 2)
					currententity->effects |= EF_MUZZLEHACK;
				
				if(r_flamehack->value == 3){
									currententity->effects |= EF_MUZZLEHACK;
					VectorCopy(currententity->origin, offsetup);
					offsetup[2] += 2;
				
					R_EntityFireParticles (currententity, offsetup, 0.7f); // this sucks atm......
				}			
			}
			// Flame
			// since some mods use this model for weapons, we have to do extra work
			// and make this an 'instant' flare
			if (!strcmp (currententity->model->name, "progs/flame2.mdl"))
				{
				vec3_t	offsetup;
				if (currententity->leifect == 12) break; // already a certain fire
				{
					currententity->leifect = 0;
			//		currententity->effects == NULL;	
				}
			
				if(r_flamehack->value == 2){
					currententity->effects |= EF_ADDITIVE;
					currententity->leifect = 0;
					
				}
				if(r_flamehack->value == 3){
					VectorCopy(currententity->origin, offsetup);
					offsetup[2] -= 12;
					currententity->leifect = 15;	// it's a fire.
					R_EntityFireParticles (currententity, offsetup, 1.0f); // this sucks atm......
				}
				
	
			}

			// Big Flame (marcher)
			if (!strcmp (currententity->model->name, "progs/flamebig.mdl"))
				{
				vec3_t	offsetup;
				int fer, fee; 
					int fa = 24;

				if(r_flamehack->value > 2)
					currententity->effects |= EF_ADDITIVE;
	/*		if (!currententity->gotaflare)
				if (r_flares->value > 1){
					VectorCopy(currententity->origin, offsetup);
					offsetup[2] += 12;

					for(fer=0;fer<fa;fer+=5){
						offsetup[2] += fer;
						R_FlareTest(offsetup,12,55,32,15,0, NULL);
							
					}
					currententity->gotaflare = 1;
				}
				*/
			}
	
			// Candle flame (marcher and rogue)
			if (!strcmp (currententity->model->name, "progs/candle.mdl") || !strcmp (currententity->model->name, "progs/flamecan.mdl"))
				{
				vec3_t	offsetup;

				if(r_flamehack->value > 2)
					currententity->effects |= EF_MUZZLEHACK;
			
				// Apply this to static entities only somehow.
		/*		if (!currententity->gotaflare)
				if (r_flares->value > 1){
						
					VectorCopy(currententity->origin, offsetup);
					offsetup[2] += 7;

					R_FlareTest(offsetup,12,47,16,9,0, NULL);
					currententity->gotaflare = 1;
				}
				*/
				}
				}
#endif

				R_AliasDrawModel (&lighting);
			}

			break;

		default:
			break;
		}
	}
}

extern int nolookups;
/*
=============
R_DrawViewModel
=============
*/
void R_DrawViewModel (void)
{

// FIXME: remove and do real lighting
	float		lightvec[3] = {-1, 0, 0};
	float		rlightvec[3] = {-1, 0, 0};
	float		glightvec[3] = {-1, 0, 0};
	float		blightvec[3] = {-1, 0, 0};
	int			j;
	int			lnum;
	vec3_t		dist;
	float		add;
	dlight_t	*dl;
		
	if (!r_drawviewmodel->value)
		return;


// 2000-01-09 ChaseCam fix by FrikaC  start
	if (chase_active->value || reflectpass || deathcam_yesiamdead)
	{
		return;
	}
// 2000-01-09 ChaseCam fix by FrikaC  end

	
	
	if (!r_muzzlehack->value && cl.items & IT_INVISIBILITY)
		return;

	if (cl.stats[STAT_HEALTH] <= 0)
		return;

	currententity = &cl.viewent;
	if (!currententity->model)
		return;

	VectorCopy (currententity->origin, r_entorigin);
	VectorSubtract (r_origin, r_entorigin, modelorg);

	VectorCopy (vup, viewlightvec);
	VectorInverse (viewlightvec);

	if (!nolookups)
	if (r_muzzlehack->value)
	currententity->muzzlehack = 1;

if (r_muzzlehack->value && cl.items & IT_INVISIBILITY)
	//currententity->alpha = 0.1;
	currententity->cloaked = 1;
else
	//currententity->alpha = 0;
	currententity->cloaked = 0;

	j = R_LightPoint (currententity->origin);

	if (j < 24)
		j = 24;		// always give some light on gun
	r_viewlighting.ambientlight = j;
	r_viewlighting.shadelight = j;
	if(coloredlights){
	r_viewlighting.ramb = lightcolor[0];
	r_viewlighting.gamb = lightcolor[1];
	r_viewlighting.bamb = lightcolor[2];

	r_viewlighting.rlight = (int)lightcolor[0];
	r_viewlighting.glight = (int)lightcolor[1];
	r_viewlighting.blight = (int)lightcolor[2];
	}
// add dynamic lights
	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		dl = &cl_dlights[lnum];
		if (!dl->radius)
			continue;
		if (dl->unmark) // leilei - skip world light
			continue;
		if (dl->die < cl.time)
			continue;

		VectorSubtract (currententity->origin, dl->origin, dist);
		add = dl->radius - Length(dist);
		if (add > 0){
			r_viewlighting.ambientlight += add;
			if(coloredlights){			
			r_viewlighting.ramb += ((int)add * (dl->color[0]));
			r_viewlighting.gamb += ((int)add * (dl->color[1]));
			r_viewlighting.bamb += ((int)add * (dl->color[2]));
			}
		}
	}

// clamp lighting so it doesn't overbright as much
	if(!overbrightmdl){
	if (r_viewlighting.ambientlight > 128)
		r_viewlighting.ambientlight = 128;
	if (r_viewlighting.ambientlight + r_viewlighting.shadelight > 192)
		r_viewlighting.shadelight = 192 - r_viewlighting.ambientlight;


	}
	
	
	r_viewlighting.plightvec = lightvec;

	if(coloredlights){
		if (r_viewlighting.ramb + r_viewlighting.rlight > 192)
	r_viewlighting.rlight = 192 - r_viewlighting.ramb;
		if (r_viewlighting.gamb + r_viewlighting.glight > 192)
	r_viewlighting.glight = 192 - r_viewlighting.gamb;
		if (r_viewlighting.bamb + r_viewlighting.blight > 192)
	r_viewlighting.blight = 192 - r_viewlighting.bamb;
					if(!overbrights){
					if (r_viewlighting.ramb + r_viewlighting.ramb > 128)
					r_viewlighting.rlight = 128 - r_viewlighting.ramb;
					if (r_viewlighting.gamb + r_viewlighting.gamb > 128)
					r_viewlighting.glight = 128 - r_viewlighting.gamb;
					if (r_viewlighting.bamb + r_viewlighting.bamb > 128)
					r_viewlighting.blight = 128 - r_viewlighting.bamb;

					if (r_viewlighting.rlight + r_viewlighting.ramb   > 128){
						r_viewlighting.rlight = 64;r_viewlighting.ramb = 64;}
					if (r_viewlighting.glight + r_viewlighting.gamb  > 128){
						r_viewlighting.glight = 64;r_viewlighting.gamb = 64;}
					if (r_viewlighting.blight + r_viewlighting.bamb  > 128){
						r_viewlighting.blight = 64;r_viewlighting.bamb = 64;}

					}
	r_viewlighting.rlightvec = rlightvec;
	r_viewlighting.glightvec = glightvec;
	r_viewlighting.blightvec = blightvec;
	}

	

	
	R_AliasDrawModel (&r_viewlighting);
}

#endif
/*
=============
R_BmodelCheckBBox
=============
*/
int R_BmodelCheckBBox (model_t *clmodel, float *minmaxs)
{
	int			i, *pindex, clipflags;
	vec3_t		acceptpt, rejectpt;
	double		d;

	clipflags = 0;

	if (currententity->angles[0] || currententity->angles[1]
		|| currententity->angles[2])
	{
		for (i=0 ; i<4 ; i++)
		{
			d = DotProduct (currententity->origin, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;

			if (d <= -clmodel->radius)
				return BMODEL_FULLY_CLIPPED;

			if (d <= clmodel->radius)
				clipflags |= (1<<i);
		}
	}
	else
	{
		for (i=0 ; i<4 ; i++)
		{
		// generate accept and reject points
		// FIXME: do with fast look-ups or integer tests based on the sign bit
		// of the floating point values

			pindex = pfrustum_indexes[i];

			rejectpt[0] = minmaxs[pindex[0]];
			rejectpt[1] = minmaxs[pindex[1]];
			rejectpt[2] = minmaxs[pindex[2]];

			d = DotProduct (rejectpt, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;

			if (d <= 0)
				return BMODEL_FULLY_CLIPPED;

			acceptpt[0] = minmaxs[pindex[3+0]];
			acceptpt[1] = minmaxs[pindex[3+1]];
			acceptpt[2] = minmaxs[pindex[3+2]];

			d = DotProduct (acceptpt, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;

			if (d <= 0)
				clipflags |= (1<<i);
		}
	}

	return clipflags;
}

extern int	r_wlightframecount;
/*
=============
R_DrawBEntitiesOnList
=============
*/
void R_DrawBEntitiesOnList (void)
{
	int			i, k, clipflags;
	vec3_t		oldorigin;
	model_t		*clmodel;
	float		minmaxs[6];

	if (!r_drawentities->value)
		return;

	VectorCopy (modelorg, oldorigin);
	insubmodel = true;
	r_dlightframecount = r_framecount;
	r_shadowframecount = r_framecount;
	r_wlightframecount = r_framecount;

	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = cl_visedicts[i];

		switch (currententity->model->type)
		{
		case mod_brush:

			clmodel = currententity->model;

		// see if the bounding box lets us trivially reject, also sets
		// trivial accept status
			// leilei - unrolled
			{
				minmaxs[0] = currententity->origin[0] +
						clmodel->mins[0];
				minmaxs[3+0] = currententity->origin[0] +
						clmodel->maxs[0];
				minmaxs[1] = currententity->origin[1] +
						clmodel->mins[1];
				minmaxs[3+1] = currententity->origin[1] +
						clmodel->maxs[1];
				minmaxs[2] = currententity->origin[2] +
						clmodel->mins[2];
				minmaxs[3+2] = currententity->origin[2] +
						clmodel->maxs[2];
			}

			clipflags = R_BmodelCheckBBox (clmodel, minmaxs);

			if (clipflags != BMODEL_FULLY_CLIPPED)
			{
				VectorCopy (currententity->origin, r_entorigin);
				VectorSubtract (r_origin, r_entorigin, modelorg);
			// FIXME: is this needed?
				VectorCopy (modelorg, r_worldmodelorg);

				r_pcurrentvertbase = clmodel->vertexes;

			// FIXME: stop transforming twice
				R_RotateBmodel ();

			// calculate dynamic lighting for bmodel if it's not an
			// instanced model
				if (clmodel->firstmodelsurface != 0)
				{
					
					for (k=0 ; k<MAX_DLIGHTS ; k++)
					{

						if ((cl_dlights[k].die < cl.time && !cl_dlights[k].unmark) ||
							(!cl_dlights[k].radius))
						{
							continue;
						}

						R_MarkLights (&cl_dlights[k], 1<<k,
							clmodel->nodes + clmodel->hulls[0].firstclipnode);
					}
					for (k=0 ; k<MAX_SHADOWS ; k++)
					{
						if ((cl_shadows[k].die < cl.time) ||
							(!cl_shadows[k].radius))
						{
							continue;
						}

						R_MarkShadows (&cl_shadows[k], 1<<k,
							clmodel->nodes + clmodel->hulls[0].firstclipnode);
					}
					for (k=0 ; k<MAX_WLIGHTS ; k++)
					{

						if ((cl_wlights[k].die < cl.time) ||
							(!cl_wlights[k].radius))
						{
							continue;
						}

						R_MarkWLights (&cl_wlights[k], 1<<k,
							clmodel->nodes + clmodel->hulls[0].firstclipnode);
					}

				}

			// if the driver wants polygons, deliver those. Z-buffering is on
			// at this point, so no clipping to the world tree is needed, just
			// frustum clipping
				if (r_drawpolys | r_drawculledpolys)
				{
					R_ZDrawSubmodelPolys (clmodel);
				}
				else
				{
					r_pefragtopnode = NULL;

				//	leilei - unrolled
					{
						r_emins[0] = minmaxs[0];
						r_emaxs[0] = minmaxs[3];
						r_emins[1] = minmaxs[1];
						r_emaxs[1] = minmaxs[4];
						r_emins[2] = minmaxs[2];
						r_emaxs[2] = minmaxs[5];
					}

					R_SplitEntityOnNode2 (cl.worldmodel->nodes);

					if (r_pefragtopnode)
					{
						currententity->topnode = r_pefragtopnode;

						if (r_pefragtopnode->contents >= 0)
						{
						// not a leaf; has to be clipped to the world BSP
							r_clipflags = clipflags;
							R_DrawSolidClippedSubmodelPolygons (clmodel);
						}
						else
						{
						// falls entirely in one leaf, so we just put all the
						// edges in the edge list and let 1/z sorting handle
						// drawing order
							R_DrawSubmodelPolygons (clmodel, clipflags);
						}

						currententity->topnode = NULL;
					}
				}

			// put back world rotation and frustum clipping
			// FIXME: R_RotateBmodel should just work off base_vxx
				VectorCopy (base_vpn, vpn);
				VectorCopy (base_vup, vup);
				VectorCopy (base_vright, vright);
				VectorCopy (base_modelorg, modelorg);
				VectorCopy (oldorigin, modelorg);
				R_TransformFrustum ();
			}

			break;

		default:
			break;
		}
	}

	insubmodel = false;
}


/*
================
R_EdgeDrawing
================
*/
void R_EdgeDrawing (void)
{
	edge_t	ledges[NUMSTACKEDGES +
				((CACHE_SIZE - 1) / sizeof(edge_t)) + 1];
	surf_t	lsurfs[NUMSTACKSURFACES +
				((CACHE_SIZE - 1) / sizeof(surf_t)) + 1];

	if (auxedges)
	{
		r_edges = auxedges;
	}
	else
	{
		r_edges =  (edge_t *)
				(((long)&ledges[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));
	}

	if (r_surfsonstack)
	{
		surfaces =  (surf_t *)
				(((long)&lsurfs[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));
		surf_max = &surfaces[r_cnumsurfs];
	// surface 0 doesn't really exist; it's just a dummy because index 0
	// is used to indicate no edge attached to surface
		surfaces--;
//		R_SurfacePatch ();
	}

	R_BeginEdgeFrame ();

	if (r_dspeeds->value)
	{
		rw_time1 = Sys_FloatTime ();
	}

	R_RenderWorld ();

	if (r_drawculledpolys)
		R_ScanEdges ();

// only the world can be drawn back to front with no z reads or compares, just
// z writes, so have the driver turn z compares on now
	D_TurnZOn ();

	if (r_dspeeds->value)
	{
		rw_time2 = Sys_FloatTime ();
		db_time1 = rw_time2;
	}

	R_DrawBEntitiesOnList ();

	if (r_dspeeds->value)
	{
		db_time2 = Sys_FloatTime ();
		se_time1 = db_time2;
	}

	if (!r_dspeeds->value)
	{
		VID_UnlockBuffer ();
		S_ExtraUpdate ();	// don't let sound get messed up if going slow
		VID_LockBuffer ();
	}

	if (!(r_drawpolys | r_drawculledpolys))
		R_ScanEdges ();
}

int low_widdth, low_heighht;
int	overbrightmdl;
int particleset;
int particlespray;
int particleblood;

/*
================
R_RenderView

r_refdef must be set before the first call
================
*/
int	amilow;
extern	int playersnd;	// leilei - dsp effects
int		reflectpass;	// leilei  - water reflection
int		waterinsight;	// leilei  - water pixel shader
vec3_t		reflectorg;	// leilei  - water reflection
extern pixel_t			*d_viewbuffer;	
#ifdef WATERREFLECTIONS
extern pixel_t			*d_reflectbuffer;	

extern pixel_t			*d_shadowbuffer;	
#endif
void R_TransferReflectBuffer (void)
{
#ifdef WATERREFLECTIONS
	int	refem;
			for (refem = 0; refem < (vid.width * vid.height); refem++){
				//*d_reflectbuffer++ = (d_viewbuffer + refem);
				d_reflectbuffer[refem] = d_viewbuffer[refem];

			}

#endif
};
#ifdef EXPREND
int	shadowpass;

void R_RenderShadows_ (void)
{
	
	R_SetupFrame ();

	R_MarkLeaves ();	// done here so we know if we're in water
	Sys_LowFPPrecision ();

	r_drawwater = 1;

	r_foundwater = r_drawwater = 0;
	
	R_EdgeDrawing ();
	R_DrawEntitiesOnList ();
	R_DrawViewModel ();
//	R_DrawParticles ();
	if (r_foundwater && !reflectpass)
	{
		r_drawwater = true;
	}
		R_EdgeDrawing ();
	Sys_HighFPPrecision ();
		
}



#endif
int	particletypeonly;
void R_RenderView_ (void)
{
	

	byte	warpbuffer[WARP_WIDTH * WARP_HEIGHT];
	byte	lowbuffer[LOW_WIDTH * LOW_HEIGHT];


	// reflections
#ifdef WATERREFLECTIONS
	byte	reflectbuffer[WARP_WIDTH * WARP_HEIGHT];
#endif

	r_warpbuffer = warpbuffer;
#ifdef WATERREFLECTIONS
	r_reflectbuffer = reflectbuffer;
#endif

	r_lowbuffer = lowbuffer;

	if (reflectpass){
			{
		float d;
		// leilei - water reflections
		// **- TODO -** proper mirror surface matrix
		// flip it good
		r_refdef.viewangles[PITCH] *= -1;
	

//	Con_Printf("%f %f %f mirror\n", mirror_plane->normal[0],mirror_plane->normal[1],mirror_plane->normal[2]);

		d = DotProduct (r_refdef.vieworg, mirror_plane->normal) - mirror_plane->dist + temp2->value;
	//	VectorMA (r_refdef.vieworg, -2*d, mirror_plane->normal, r_refdef.vieworg);
		VectorMA (r_refdef.vieworg, -2*d, mirror_plane->normal, r_refdef.vieworg);
		d = DotProduct (vpn, mirror_plane->normal);
	//	VectorMA (vpn, -2*d, mirror_plane->normal, vpn);
	VectorMA (vpn, -2*d, mirror_plane->normal, vpn);

//	r_refdef.viewangles[0] = -asin (vpn[2])/M_PI*180;
//	r_refdef.viewangles[1] = atan2 (vpn[1], vpn[0])/M_PI*180;
//	r_refdef.viewangles[2] = -r_refdef.viewangles[2];



		r_refdef.viewangles[ROLL] *= -1;
			}
	}
	if (r_timegraph->value || r_speeds->value || r_dspeeds->value)
		r_time1 = Sys_FloatTime ();

#ifdef SPLIT
	if (cursplit){
		sb_lines = 0;
		R_SetupFrameSplit ();
	}
	else
	R_SetupFrame ();
#else
	
	R_SetupFrame ();
#endif


	R_MarkLeaves ();	// done here so we know if we're in water

// make FDIV fast. This reduces timing precision after we've been running for a
// while, so we don't do it globally.  This also sets chop mode, and we do it
// here so that setup stuff like the refresh area calculations match what's
// done in screen.c
	
	Sys_LowFPPrecision ();

	if (!cl_entities[0].model || !cl.worldmodel)
		Sys_Error ("R_RenderView: NULL worldmodel");
if (reflectpass)
	r_drawwater = 0;
	if (!r_dspeeds->value)
	{
		VID_UnlockBuffer ();
		S_ExtraUpdate ();	// don't let sound get messed up if going slow
		VID_LockBuffer ();
	}
	r_foundwater = r_drawwater = 0;
	

	R_EdgeDrawing ();

	if (!r_dspeeds->value)
	{
		VID_UnlockBuffer ();
		S_ExtraUpdate ();	// don't let sound get messed up if going slow
		VID_LockBuffer ();
	}
if (!reflectpass)
	if (r_dspeeds->value)
	{
		se_time2 = Sys_FloatTime ();
		de_time1 = se_time2;
	}

	R_DrawEntitiesOnList ();

	if (r_dspeeds->value)
	{
		de_time2 = Sys_FloatTime ();
#ifndef VMTOC
		dv_time1 = de_time2;
	}

if(!reflectpass)
	R_DrawViewModel ();

	if (r_dspeeds->value)
	{
		dv_time2 = Sys_FloatTime ();
#endif
		dp_time1 = Sys_FloatTime ();
	}

	// render our normal particles... (run particle physics etc)
	
	R_DrawParticles ();

// test our flares.

		if (!reflectpass)
		R_TestFlares ();	// leilei - flare testing

//		if (foguse2)
//		R_ApplyFog();	// you're too slow!

	if(r_depthoffield->value)
	R_ApplyDof();	// you're too slow!
	
if (!reflectpass)
	if (r_dspeeds->value)
		dp_time2 = Sys_FloatTime ();

	// Manoel Kasimier - translucent water - begin
	
	if (r_foundwater && !reflectpass)
	{
		r_drawwater = true;
		R_EdgeDrawing ();
	}
	// Manoel Kasimier - translucent water - end
#ifdef WATERLOW
	if(reflectpass)
				D_CrapScreenReflection ();
#endif
	if (!reflectpass)
	if (amilow != r_virtualmode->value)
	{
		amilow = r_virtualmode->value;
		if (amilow > 5)
				amilow = 5;
		r_docrap = amilow;
		
		vid.recalc_refdef = 1;
	}

	//if (!reflectpass)
	R_DrawFlares ();

	if (!reflectpass){
		if (overbrightmdl != r_overbrightmdl->value)
			overbrightmdl = r_overbrightmdl->value;
		if (particleset != r_particleset->value)
			particleset = r_particleset->value;
		if (particlespray != r_particlespray->value)
			particlespray = r_particlespray->value;
		if (particleblood != r_particleblood->value)
			particleblood = r_particleblood->value;
	}
	

	// now, render out lens flares afterward (sort hack)

	// Screen Resolution Reduction	



	if(!reflectpass){
//	if (r_docrap)
	if (amilow)			// 160x100
		D_CrapScreenMH ();

	else 
	{
		if 		(r_dowarp){
		D_WarpScreen ();
		}
	}

	}	
	
	if (!reflectpass){
	if (r_viewleaf->contents <= CONTENTS_WATER)
		playersnd = 2;
	else
		playersnd = 0;

	V_SetContentsColor (r_viewleaf->contents);
	}
if (!reflectpass)
	if (r_alphashift->value)
		D_AlphaShift ();

	
	if (!reflectpass){
	if (r_timegraph->value)
		R_TimeGraph ();

	if (r_aliasstats->value)
		R_PrintAliasStats ();

	if (r_speeds->value)
		R_PrintTimes ();

	if (r_dspeeds->value)
		R_PrintDSpeeds ();

	if (r_reportsurfout->value && r_outofsurfaces)
		Con_Printf ("Short %d surfaces\n", r_outofsurfaces);

	if (r_reportedgeout->value && r_outofedges)
		Con_Printf ("Short roughly %d edges\n", r_outofedges * 2 / 3);
	}
// back to high floating-point precision
	Sys_HighFPPrecision ();
}
extern	int lightingavailable;
extern int lightingcantbeavailable;
void R_RenderView (void)
{
	int		dummy;
	int		delta;

	delta = (byte *)&dummy - r_stack_start;
#ifndef VMTOC
//	if (delta < -10000 || delta > 10000)
//		Sys_Error ("R_RenderView: called without enough stack"); // FIXME - TACK SBUG!
#endif
	if ( Hunk_LowMark() & 3 )
		Sys_Error ("Hunk is missaligned");

	if ( (long)(&dummy) & 3 )
		Sys_Error ("Stack is missaligned");

	if ( (long)(&r_warpbuffer) & 3 )
		Sys_Error ("Globals are missaligned");
#ifdef WATERREFLECTIONS
	if ( (long)(&r_reflectbuffer) & 3 )
		Sys_Error ("Globals are missaligned");
#endif
	

	if ( (long)(&r_lowbuffer) & 3 )
		Sys_Error ("Globals are missaligned");
#ifdef EXPREND
	if ( (long)(&r_shadowbuffer) & 3 )
		Sys_Error ("Globals are missaligned");

	shadowpass = 1;
	R_RenderShadows_ ();
	shadowpass = 0;
	R_RenderView_ ();
#else
	R_RenderView_ ();
#endif

	// lights crap!

	if (!lightingcantbeavailable && !lightingavailable && sv.worldmodel)
		TheForceLoadLighting();
}

/*
================
R_InitTurb
================
*/
void R_InitTurb (void)
{
	int		i;

	for (i=0 ; i<(SIN_BUFFER_SIZE) ; i++)
	{
		sintable[i] = AMP + sin(i*3.14159*2/CYCLE)*AMP;
		intsintable[i] = AMP2 + sin(i*3.14159*2/CYCLE)*AMP2;	// AMP2, not 20
		//atableofnothingtable[i] = AMP2 + sin(i*3.14159*2/CYCLE)*AMP2;	// AMP2, not 20
		atableofnothingtable[i] = AMP2 + 0.1 *AMP2;	// AMP2, not 20
	}
}

// from FTEQW, though I don't really use it
//  high quality fog mode with lots of more precision
// unfortunately lots of more slower
// I tried to enhance it using my same fog lookup tabes
// and added in LA-style dithering
//extern byte *addTable;
extern int truecolor;

extern qboolean r_dowarp;

extern pixel_t			*d_viewbuffer;
void R_ApplyFog(void)
{
	// test code for fog, the real implementation should use a lookup table
		byte		*pbuf;
		//byte *pbuf;
	short *zbuf;
	extern short *d_pzbuffer;
	int y, x, g;
	float v, f,j,h;
	int	  vd[64];
	int		var;
	for (y=0 ; y<scr_vrect.height ; y++)
	{
		int t, s, q;

	//	pbuf = (qbyte *)(vid.buffer + vid.rowbytes*y);
//		r_turb_prefst = (unsigned char *)((byte *)d_viewbuffer +(screenwidth * pspan->v) + pspan->u);
	//	pbuf = (byte *)(d_viewbuffer + vid.width*y);
		//pbuf = (byte *)(d_viewbuffer + vid.rowbytes*y);
		pbuf = (byte *)(d_viewbuffer + scr_vrect.y + vid.rowbytes * y);// + scr_vrect.width * y);
		zbuf = d_pzbuffer + (scr_vrect.width*y);
		t = (y & 1);// << 1;
		s = (y & 3) << 1;
		q = (y & 3) >> 1;
//		for (x=0 ; x<vid.width ; x++)
		for (x=0 ; x<vid.width ; x+=4)
		{
			if (!zbuf[x]){
				pbuf[x] = pbuf[x];
				pbuf[x+1] = pbuf[x+1];
				pbuf[x+2] = pbuf[x+2];
				pbuf[x+3] = pbuf[x+3];
			}
			else
			{
				v = 64.0f / zbuf[x];
				f = 64.0f / zbuf[x+1];
				j = 64.0f / zbuf[x+2];
				h = 64.0f / zbuf[x+3];
			//	v = zbuf[x];
				v = bound(0, v, 1);
				f = bound(0, f, 1);
				j = bound(0, j, 1);
				h = bound(0, h, 1);
				//v = bound(0, v, 32768);
				v *= 32768;
				f *= 32768;
				j *= 32768;
				h *= 32768;
				if ((x & 1) != t){
				pbuf[x] = host_fogmap[pbuf[x]+ (((int)v + 1024) >> 2 & 0xFF00)];
				pbuf[x+1] = host_fogmap[pbuf[x+1]+ (((int)f) >> 2 & 0xFF00)];
				pbuf[x+2] = host_fogmap[pbuf[x+2]+ (((int)j + 1024) >> 2 & 0xFF00)];
				pbuf[x+3] = host_fogmap[pbuf[x+3]+ (((int)h) >> 2 & 0xFF00)];
				}
				else{
				pbuf[x] = host_fogmap[pbuf[x]+ ((int)v >> 2 & 0xFF00)];
				pbuf[x+1] = host_fogmap[pbuf[x+1]+ (((int)f + 1024) >> 2 & 0xFF00)];
				pbuf[x+2] = host_fogmap[pbuf[x+2]+ (((int)j) >> 2 & 0xFF00)];
				pbuf[x+3] = host_fogmap[pbuf[x+3]+ (((int)h + 1024) >> 2 & 0xFF00)];
				}
			}
			
		
		}
//	}
	}
}
extern byte transTable[256][256];
extern byte addTable[256][256];
extern cvar_t *temp3;

void R_ApplyDorf (void)
{
		int		w, h;
	int		u,v;
	byte	*dest;
	int		*turb;
	byte	*rowptr[MAXHEIGHT+(AMP2*2)];
	int		column[MAXWIDTH+(AMP2*2)];
	float	wratio, hratio;

	w = r_refdef.vrect.width / 2;
	h = r_refdef.vrect.height;

	dest = vid.buffer + scr_vrect.y * vid.rowbytes + scr_vrect.x;
	
        for (v = 0; v < scr_vrect.height; v++, dest += vid.rowbytes)
        {
  
            int *mycol = dest;
			int *mycolf = dest;
            byte *mydest = dest;
	
			
            for (u = 0; u < scr_vrect.width; u += 4, mycol += 4, mydest += 4)
            {
	
				if (mydest > 208){
                mydest[0] = mydest[0];
                mydest[1] = mydest[0];
                mydest[2] = mydest[0];
				mydest[3] = mydest[0];

				}
			}
        }
    
	
}

void R_ApplyDof(void)
{
	// test code for a cheesy depth of field effect
		byte		*pbuf;
		//byte *pbuf;
	short *zbuf;
	extern short *d_pzbuffer;
	int y, x, g;
	float v, f,j,h, i;
	int	  vd[64];
	int	detail;
	float flr = vid.width / 320;
	float	ferv = scr_fov->value / 90;
	float	farv = scr_fov->value / 89 + 1 * -1;
	float	feev = 1 - ferv;
	int		var;
	
	float	zeom = vid.width * 0.4;
	if (scr_fov->value > 89) return;

	detail = (int)r_depthoffield->value;
	if (detail == 1){
		detail = (int)(vid.width / 120); // determine detail from our video mode.
	}

	//zeom = temp2->value;
	if (detail < 1)
		detail = 1; // 0 causes infinite loop crash.
	if (r_virtualmode->value)
		return; // i also crash.

	for (y=0 ; y<scr_vrect.height ; y++)
	{
		int t, s, q;

		pbuf = (byte *)(d_viewbuffer + scr_vrect.y + vid.rowbytes * y);// + scr_vrect.width * y);
	//pbuf = (byte *)(d_viewbuffer + scr_vrect.y + vid.rowbytes + scr_vrect.x);
		zbuf = d_pzbuffer + (scr_vrect.width*y);
	//	zbuf = d_pzbuffer + scr_vrect.y * vid.rowbytes + scr_vrect.x;

		for (x=0 ; x<vid.width ; x+=detail)
		{
			if (!zbuf[x]){
			return;
			}
			else
			{
				v = 92.0f / zbuf[x];
			
				i = 256.0f - v;
			//	v = bound(0, v, 1);
			//	v = bound(farv, v, ferv);
		//		v = bound(24, v, 0);
				
				v *= -zeom;
				v += zeom;
				v *= feev;


			//	pbuf[x] = pbuf[x + (rand()&(int)v)];
			//	if (v>1)
			//		pbuf[x] = transTable[pbuf[x + (rand()&(int)v)]][pbuf[x]];
			//	else if (v>2)
			//		pbuf[x] = transTable[pbuf[x]][pbuf[x + (rand()&(int)v)]];
			if (v > detail)
				{
					int eh;
				//	Con_Printf("%f = v\n", v);
					for(eh=0; eh<detail; eh++)
						pbuf[x+eh] = pbuf[x + (rand()&((int)v))];
				//pbuf[x] = host_fogmap[pbuf[x]+ (((int)v + 1024) >> 2 & 0xFF00)];
				}
			}
			
		
		}
//	}
	}
}


