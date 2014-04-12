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

// refresh.h -- public interface to refresh functions

#define	MAXCLIPPLANES	11

#define	TOP_RANGE		16			// soldier uniform colors
#define	BOTTOM_RANGE	96

//=============================================================================

typedef struct efrag_s
{
	struct mleaf_s		*leaf;
	struct efrag_s		*leafnext;
	struct entity_s		*entity;
	struct efrag_s		*entnext;
} efrag_t;

// hi, makaqu
byte r_foundwater, r_drawwater; // mk transwater

#include "matrixlib.h"


// !!! if this is changed, it must be changed in d_ifacea.h too !!!
typedef struct particle_s
{
// driver-usable fields
	vec3_t		org;
	float		color;
// drivers never touch the following fields
	struct particle_s	*next;
	vec3_t		vel;
	float		ramp;
	float		die;
	
	ptype_t		type;

	// ASMME - NEW STRUCTS
	float		alpha;			// quake2
	float		alphavel;
	int			blend;		// 0 - normal, 1 - additive, 2 - gellin 
	int			lit;		// is the model lit by the world? (hey, blame dp for this idea.)
	int			polor;		// real particle color (fog fix)
	int			trail;		// a particle trail!...maybe recursive.... I don't know yet!
	int			frame;		// frame on the sprite sheet (obsolete?)
	float		scale;		// scale of the particle.
	float		scaley;		// vertical scale of the particle. (for beams etcetcetc.)
	struct		model_s			*model;		// oh god. specific sprites! automatically precached?
	vec3_t		angles;	// this hurts
	int			sprtype;	// oh this is so fat
	float		scalexvel;
	float		scaleyvel;
	vec3_t		anglevel;

	// leilei - some sticky experiment where if the particle clips the view, it'll just stick instead.
	int			stickable;		// 0 - not sticky (normal) 1 - sticky ready 2 - is stuck to the view
	vec3_t		vstuck1;
	vec3_t		vstuck2;
} particle_t;
// !!! if thi


// !!! if this is changed, it must be changed in d_ifacea.h too !!!
typedef struct flare_s
{
// driver-usable fields
	vec3_t		org;
	float		color;
// drivers never touch the following fields
	struct flare_s	*next;
	vec3_t		vel;
	float		ramp;
	float		die;
	
	ptype_t		type;

	// ASMME - NEW STRUCTS
	float		alpha;			// quake2
	float		alphavel;
	int			blend;		// 0 - normal, 1 - additive, 2 - gellin 
	int			lit;		// is the model lit by the world? (hey, blame dp for this idea.)
	int			polor;		// real particle color (fog fix)
	int			trail;		// a particle trail!...maybe recursive.... I don't know yet!
	int			frame;		// frame on the sprite sheet (obsolete?)
	float		scale;		// scale of the flare.
	float		scaley;		// scale of the flare
	struct		model_s			*model;		// oh god. specific sprites! automatically precached?
	vec3_t		angles;	// this hurts
	int			sprtype;	// oh this is so fat
	float		scalexvel;
	float		scaleyvel;
	vec3_t		anglevel;
	int			amiseen;	// determines if flare is seen. test flare to this.
	struct entity_s *owner;
	qboolean	owned;	// am i owned? (checking for deleting particle if entity dies)
	struct		model_s			*oldmodel;		// for owner
	float		lensreflection;
} flare_t;

typedef struct entity_s
{
	float					framelerp;
	qboolean				forcelink;		// model changed

	int						update_type;

	entity_state_t			baseline;		// to fill in defaults in updates

	double					msgtime;		// time of last update
	vec3_t					msg_origins[2];	// last two updates (0 is newest)
	vec3_t					origin;

	vec3_t					msg_angles[2];	// last two updates (0 is newest)
	vec3_t					angles;

	struct model_s			*model;			// NULL = no model
	struct efrag_s			*efrag;			// linked list of efrags
	int						frame;
	int						oldframe;
	float					syncbase;		// for client-side animations

	//byte					*colormap;
	// h2
	byte					*colormap, *sourcecolormap;
	byte					colorshade;
	// h2
	int						effects;		// light, particals, etc
	int						skinnum;		// for Alias models
	int						visframe;		// last frame this entity was
											//  found in an active leaf
	int						number;
	int						dlightframe;	// dynamic lighting
	int						dlightbits;

// FIXME: could turn these into a union
	int						trivial_accept;
	struct mnode_s			*topnode;		// for bmodels, first world node
											//  that splits bmodel, or NULL if
											//  not split

	int flags;
	float					alpha;
	float					scale_start_time;
//	float					scale1;
#ifdef SCALEE
	float					scale2;
#endif
	float					glow_size;
	float					glow_red;	// tomaz glow stuff we probably will not use
	float					glow_green;
	float					glow_blue;
	vec3_t                  colormod;
#ifdef MHINTERPOL
		// frame interpolation
	float frame_start_time;
	int   lastpose;
	int   currpose;

	// even if you've already done the QER stuff you'll need this too!
	int   lastframe;
#endif
	struct model_s			*lastmodel;	// leilei - gun draw hack
	int						shadowsize;	// leilei - shadow size hack
	int						shadowframe; // leilei - shadow hack
	int						shadowbits;
	float					scale;
	int						glowcolor;
	float					glowsize;
#ifdef VMTOC
	qboolean			viewmodel;
#endif
#ifdef INTERPOL7
		// frame interpolation
	float frame_start_time;
	int   lastpose;
	int   currpose;

	// even if you've already done the QER stuff you'll need this too!
	int   lastframe;
#endif
	qboolean			muzzlehack;	// leilei - awful muzzleflash additive blend hack
	qboolean			imstatic;	// am i a static ent? (for flares)
	qboolean			gotaflare;	// is my flare function called already? (For statics)
	struct flare_s	*ourparticle;	// for flare
	float			flalpha;		// for flare
	float			flalphavel;		// for flare
	int				cloaked;		// leilei - experimental cloak effect
	int				isfading;		// leilei - for alpha
	float			partfinished;	// leilei - preventing particle spam
	int				leifect;		// leilei - effect hack :/
	vec3_t          shadowvec;		// leilei - fake direction shadow casting
	float			shadowdist;
	float			shadowopacity;
} entity_t;


// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vrect_t		vrect;				// subwindow in video for refresh
									// FIXME: not need vrect next field here?
	vrect_t		aliasvrect;			// scaled Alias version
	int			vrectright, vrectbottom;	// right & bottom screen coords
	int			aliasvrectright, aliasvrectbottom;	// scaled Alias versions
	float		vrectrightedge;			// rightmost right edge we care about,
										//  for use in edge list
	float		fvrectx, fvrecty;		// for floating-point compares
	float		fvrectx_adj, fvrecty_adj; // left and top edges, for clamping
	int			vrect_x_adj_shift20;	// (vrect.x + 0.5 - epsilon) << 20
	int			vrectright_adj_shift20;	// (vrectright + 0.5 - epsilon) << 20
	float		fvrectright_adj, fvrectbottom_adj;
										// right and bottom edges, for clamping
	float		fvrectright;			// rightmost edge, for Alias clamping
	float		fvrectbottom;			// bottommost edge, for Alias clamping
	float		horizontalFieldOfView;	// at Z = 1.0, this many X is visible
										// 2.0 = 90 degrees
	float		xOrigin;			// should probably always be 0.5
	float		yOrigin;			// between be around 0.3 to 0.5

	vec3_t		vieworg;
	vec3_t		viewangles;

	float		fov_x, fov_y;
	float			time;
	float			oldtime;
	int			ambientlight;

} refdef_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vrect_t		vrect;				// subwindow in video for refresh
									// FIXME: not need vrect next field here?
	vrect_t		aliasvrect;			// scaled Alias version
	int			vrectright, vrectbottom;	// right & bottom screen coords
	int			aliasvrectright, aliasvrectbottom;	// scaled Alias versions
	float		vrectrightedge;			// rightmost right edge we care about,
										//  for use in edge list
	float		fvrectx, fvrecty;		// for floating-point compares
	float		fvrectx_adj, fvrecty_adj; // left and top edges, for clamping
	int			vrect_x_adj_shift20;	// (vrect.x + 0.5 - epsilon) << 20
	int			vrectright_adj_shift20;	// (vrectright + 0.5 - epsilon) << 20
	float		fvrectright_adj, fvrectbottom_adj;
										// right and bottom edges, for clamping
	float		fvrectright;			// rightmost edge, for Alias clamping
	float		fvrectbottom;			// bottommost edge, for Alias clamping
	float		horizontalFieldOfView;	// at Z = 1.0, this many X is visible 
										// 2.0 = 90 degrees
	float		xOrigin;			// should probably always be 0.5
	float		yOrigin;			// between be around 0.3 to 0.5

	int			ambientlight;
} oldrefdef_t;

typedef struct mpic_s
{
	int			width;
	short		height;
	byte		alpha;
	byte		pad;
	byte		data[4];	// variable sized
} mpic_t;

extern	oldrefdef_t	r_oldrefdef;

//
// refresh
//
extern	int		reinit_surfcache;


extern	refdef_t	r_refdef;
extern vec3_t	r_origin, vpn, vright, vup;

extern	struct texture_s	*r_notexture_mip;


void R_Init (void);
void R_InitTextures (void);
void R_InitEfrags (void);
void R_RenderView (void);		// must set r_refdef first
void R_ViewChanged (vrect_t *pvrect, int lineadj, float aspect);
								// called whenever r_refdef or vid change
void R_InitSky (struct texture_s *mt);	// called at level load

void R_AddEfrags (entity_t *ent);
void R_RemoveEfrags (entity_t *ent);

void R_NewMap (void);


void R_ParseParticleEffect (void);
void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);

void R_RocketTrail (vec3_t start, vec3_t end, int type);
// leilei-new particels
void R_RailTrail (vec3_t start, vec3_t end);
void R_Smoke (vec3_t org, vec3_t dir, int color, int count);
void R_RunWarticleEffect (vec3_t org, vec3_t dir, int color, int count);

#ifdef QUAKE2
void R_DarkFieldParticles (entity_t *ent);
#endif
void R_EntityParticles (entity_t *ent);
void R_BlobExplosion (vec3_t org);
void R_ParticleExplosion (vec3_t org);
void R_ParticleExplosion2 (vec3_t org, int colorStart, int colorLength);
void R_LavaSplash (vec3_t org);
void R_TeleportSplash (vec3_t org);

void R_PushDlights (void);


//
// surface cache related
//
extern	int		reinit_surfcache;	// if 1, surface cache is currently empty and
extern qboolean	r_cache_thrash;	// set if thrashing the surface cache

int	D_SurfaceCacheForRes (int width, int height);
void D_FlushCaches (void);
void D_DeleteSurfaceCache (void);
void D_InitCaches (void *buffer, int size);
void R_SetVrect (vrect_t *pvrect, vrect_t *pvrectin, int lineadj);


extern byte coltranslate[256];				// TranslateToCustomPal 



