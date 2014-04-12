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
// r_alias.c: routines for setting up to draw alias models

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"	// FIXME: shouldn't be needed (is needed for patch
						// right now, but that should move)
#ifndef MHINTERPOL
#define LIGHT_MIN	5		// lowest light value we'll allow, to avoid the
							//  need for inner-loop light clamping
// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anorm_dots.h"
;
extern	int lightingavailable;
extern	int lightingcantbeavailable;

mtriangle_t		*ptriangles;
affinetridesc_t	r_affinetridesc;

#ifdef INTERPOL7
void 			*acolormap;	// FIXME: should go away
#else
void *			acolormap;	// FIXME: should go away
#endif
float			*somelightnormal;
trivertx_t		*r_apverts;
int				r_shadedots_quant;
float			*r_shadedots;
trivertx_t		*r_oldapverts;
// TODO: these probably will go away with optimized rasterization
mdl_t				*pmdl;

vec3_t				r_plightvec;
vec3_t				r_rlightvec;
vec3_t				r_glightvec;
vec3_t				r_blightvec;
int					r_ambientlight;
int					r_ramb;
int					r_gamb;
int					r_bamb;
float				r_shadelight;
float				r_rlight;
float				r_glight;
float				r_blight;
aliashdr_t			*paliashdr;
finalvert_t			*pfinalverts;
auxvert_t			*pauxverts;
static float		ziscale;
static model_t		*pmodel;
static vec3_t		alias_forward, alias_right, alias_up;

static maliasskindesc_t	*pskindesc;

int				r_amodels_drawn;
int				a_skinwidth;
int				r_anumverts;

float	aliastransform[3][4];

typedef struct {
	int	index0;
	int	index1;
} aedge_t;

static aedge_t	aedges[12] = {
{0, 1}, {1, 2}, {2, 3}, {3, 0},
{4, 5}, {5, 6}, {6, 7}, {7, 4},
{0, 5}, {1, 4}, {2, 7}, {3, 6}
};

#define NUMVERTEXNORMALS	162

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

#ifdef INTERPOL7
typedef struct aliaslerp_s
{
	float v[3];

	byte lastlightnormal;
	byte currlightnormal;
	float blend;
} aliaslerp_t;

finalvert_t		*r_finalverts;
auxvert_t		*r_auxverts;
aliaslerp_t		*r_aliaslerpverts;
aliaslerp_t		*lerpverts;

void R_AliasTransformAndProjectFinalVertsLerp (finalvert_t *fv, stvert_t *pstverts);
//void R_AliasTransformAndProjectFinalVerts_C (finalvert_t *fv, stvert_t *pstverts);
#endif


void R_AliasTransformAndProjectFinalVerts (finalvert_t *fv,	stvert_t *pstverts);
void R_AliasSetUpTransform (int trivial_accept);
void R_AliasTransformVector (vec3_t in, vec3_t out);
#ifdef INTERPOL7
void R_AliasTransformFinalVert (finalvert_t *fv, auxvert_t *av, aliaslerp_t *pverts, stvert_t *pstverts);
#else
void R_AliasTransformFinalVert (finalvert_t *fv, auxvert_t *av,	trivertx_t *pverts, stvert_t *pstverts);
#endif

void R_AliasProjectFinalVert (finalvert_t *fv, auxvert_t *av);

void R_AliasFrameBoundingBox (int frame, float time, trivertx_t **mins, trivertx_t **maxs)
{
	int	i, numposes;
	maliasgroup_t *paliasgroup;
	float *pintervals, fullinterval, targettime;

	if (paliashdr->frames[frame].type == ALIAS_SINGLE)
	{
		*mins = &paliashdr->frames[frame].bboxmin;
		*maxs = &paliashdr->frames[frame].bboxmax;
	}
	else
	{
		paliasgroup = (maliasgroup_t *)
			((byte *)paliashdr + paliashdr->frames[frame].frame);
		pintervals = (float *)((byte *)paliashdr + paliasgroup->intervals);
		numposes = paliasgroup->frames[frame].numframes;
		fullinterval = pintervals[numposes-1];

	//
	// when loading in Mod_LoadAliasGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
	//
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numposes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}
		
		*mins = &paliasgroup->frames[i].bboxmin;
		*maxs = &paliasgroup->frames[i].bboxmax;
	}
}

#ifdef INTERPOL7
int r_maxaliasverts = 0;
void R_CheckAliasVerts (int numverts)
{
	if (numverts > r_maxaliasverts) r_maxaliasverts = numverts;
}


void R_FinalizeAliasVerts (void)
{
	// called every map change - these go on the hunk so they get cleared between maps
	// because alias models go in the cache r_maxaliasverts can only grow, never shrink
	r_aliaslerpverts = (aliaslerp_t *) Hunk_Alloc (r_maxaliasverts * sizeof (aliaslerp_t));
	r_auxverts = (auxvert_t *) Hunk_Alloc (r_maxaliasverts * sizeof (auxvert_t));
	r_finalverts = (finalvert_t *) Hunk_Alloc ((r_maxaliasverts + ((CACHE_SIZE - 1) / sizeof (finalvert_t)) + 1) * sizeof (finalvert_t));

	Con_DPrintf ("%i alias verts\n", r_maxaliasverts);
}
#endif


/*
================
R_AliasCheckBBox
================
*/
qboolean R_AliasCheckBBox (void)
{
	int					i, flags, frame, numv;
	aliashdr_t			*pahdr;
	float				zi, basepts[8][3], v0, v1, frac;
	finalvert_t			*pv0, *pv1, viewpts[16];
	auxvert_t			*pa0, *pa1, viewaux[16];
	maliasframedesc_t	*pframedesc;
	qboolean			zclipped, zfullyclipped;
	unsigned			anyclip, allclip;
	int					minz;

// expand, rotate, and translate points into worldspace

	currententity->trivial_accept = 0;
	pmodel = currententity->model;
	pahdr = Mod_Extradata (pmodel);
	pmdl = (mdl_t *)((byte *)pahdr + pahdr->model);

#ifdef INTERPOL7
		
	if (!r_lerpmodels->value)
		R_AliasSetUpTransform (currententity->trivial_accept);
		else
		R_AliasSetUpTransform (0);
#else
	R_AliasSetUpTransform (0);
#endif
// construct the base bounding box for this frame
	frame = currententity->frame;
// TODO: don't repeat this check when drawing?
	if ((frame >= pmdl->numframes) || (frame < 0))
	{
		Con_DPrintf ("No such frame %d %s\n", frame,
				pmodel->name);
		frame = 0;
	}

	pframedesc = &pahdr->frames[frame];

// x worldspace coordinates
	basepts[0][0] = basepts[1][0] = basepts[2][0] = basepts[3][0] =
			(float)pframedesc->bboxmin.v[0];
	basepts[4][0] = basepts[5][0] = basepts[6][0] = basepts[7][0] =
			(float)pframedesc->bboxmax.v[0];

// y worldspace coordinates
	basepts[0][1] = basepts[3][1] = basepts[5][1] = basepts[6][1] =
			(float)pframedesc->bboxmin.v[1];
	basepts[1][1] = basepts[2][1] = basepts[4][1] = basepts[7][1] =
			(float)pframedesc->bboxmax.v[1];

// z worldspace coordinates
	basepts[0][2] = basepts[1][2] = basepts[4][2] = basepts[5][2] =
			(float)pframedesc->bboxmin.v[2];
	basepts[2][2] = basepts[3][2] = basepts[6][2] = basepts[7][2] =
			(float)pframedesc->bboxmax.v[2];

	zclipped = false;
	zfullyclipped = true;

	minz = 9999;
	for (i=0; i<8 ; i++)
	{
		R_AliasTransformVector  (&basepts[i][0], &viewaux[i].fv[0]);

		if (viewaux[i].fv[2] < ALIAS_Z_CLIP_PLANE)
		{
		// we must clip points that are closer than the near clip plane
			viewpts[i].flags = ALIAS_Z_CLIP;
			zclipped = true;
		}
		else
		{
			if (viewaux[i].fv[2] < minz)
				minz = viewaux[i].fv[2];
			viewpts[i].flags = 0;
			zfullyclipped = false;
		}
	}


	if (zfullyclipped)
	{
		return false;	// everything was near-z-clipped
	}

	numv = 8;

	if (zclipped)
	{
	// organize points by edges, use edges to get new points (possible trivial
	// reject)
		for (i=0 ; i<12 ; i++)
		{
		// edge endpoints
			pv0 = &viewpts[aedges[i].index0];
			pv1 = &viewpts[aedges[i].index1];
			pa0 = &viewaux[aedges[i].index0];
			pa1 = &viewaux[aedges[i].index1];

		// if one end is clipped and the other isn't, make a new point
			if (pv0->flags ^ pv1->flags)
			{
				frac = (ALIAS_Z_CLIP_PLANE - pa0->fv[2]) /
					   (pa1->fv[2] - pa0->fv[2]);
				viewaux[numv].fv[0] = pa0->fv[0] +
						(pa1->fv[0] - pa0->fv[0]) * frac;
				viewaux[numv].fv[1] = pa0->fv[1] +
						(pa1->fv[1] - pa0->fv[1]) * frac;
				viewaux[numv].fv[2] = ALIAS_Z_CLIP_PLANE;
				viewpts[numv].flags = 0;
				numv++;
			}
		}
	}

// project the vertices that remain after clipping
	anyclip = 0;
	allclip = ALIAS_XY_CLIP_MASK;

// TODO: probably should do this loop in ASM, especially if we use floats
	for (i=0 ; i<numv ; i++)
	{
	// we don't need to bother with vertices that were z-clipped
		if (viewpts[i].flags & ALIAS_Z_CLIP)
			continue;

		zi = 1.0 / viewaux[i].fv[2];

	// FIXME: do with chop mode in ASM, or convert to float
		v0 = (viewaux[i].fv[0] * xscale * zi) + xcenter;
		v1 = (viewaux[i].fv[1] * yscale * zi) + ycenter;

		flags = 0;

		if (v0 < r_refdef.fvrectx)
			flags |= ALIAS_LEFT_CLIP;
		if (v1 < r_refdef.fvrecty)
			flags |= ALIAS_TOP_CLIP;
		if (v0 > r_refdef.fvrectright)
			flags |= ALIAS_RIGHT_CLIP;
		if (v1 > r_refdef.fvrectbottom)
			flags |= ALIAS_BOTTOM_CLIP;

		anyclip |= flags;
		allclip &= flags;
	}

	if (allclip)
		return false;	// trivial reject off one side

	currententity->trivial_accept = !anyclip & !zclipped;

	if (currententity->trivial_accept)
	{
		if (minz > (r_aliastransition + (pmdl->size * r_resfudge)))
		{
			currententity->trivial_accept |= 2;
		}
	}
#ifdef INTERPOL7
		// never trivial accept as it breaks the render with interpolation
	if (r_lerpmodels->value)
		currententity->trivial_accept = 0;

#endif
	return true;

}

/*
================
R_AliasTransformVector
================
*/
void R_AliasTransformVector (vec3_t in, vec3_t out)
{
	out[0] = DotProduct(in, aliastransform[0]) + aliastransform[0][3];
	out[1] = DotProduct(in, aliastransform[1]) + aliastransform[1][3];
	out[2] = DotProduct(in, aliastransform[2]) + aliastransform[2][3];
}


/*
================
R_AliasPreparePoints

General clipped case
================
*/
void R_AliasPreparePoints (void)
{
	
	int			i;
	stvert_t	*pstverts;
	finalvert_t	*fv;
	auxvert_t	*av;
	mtriangle_t	*ptri;
	finalvert_t	*pfv[3];

	pstverts = (stvert_t *)((byte *)paliashdr + paliashdr->stverts);
	r_anumverts = pmdl->numverts;
 	fv = pfinalverts;
	av = pauxverts;
#ifdef INTERPOL7
		lerpverts = r_aliaslerpverts;

				for (i = 0; i < r_anumverts; i++, fv++, av++, lerpverts++, pstverts++){
					R_AliasTransformFinalVert (fv, av, lerpverts, pstverts);
				
						if (av->fv[2] < ALIAS_Z_CLIP_PLANE)
							fv->flags |= ALIAS_Z_CLIP;
							else
							{
								 R_AliasProjectFinalVert (fv, av);
			
									if (fv->v[0] < r_refdef.aliasvrect.x)
										fv->flags |= ALIAS_LEFT_CLIP;
									if (fv->v[1] < r_refdef.aliasvrect.y)
										fv->flags |= ALIAS_TOP_CLIP;
									if (fv->v[0] > r_refdef.aliasvrectright)
										fv->flags |= ALIAS_RIGHT_CLIP;
									if (fv->v[1] > r_refdef.aliasvrectbottom)
										fv->flags |= ALIAS_BOTTOM_CLIP;
							}
						}

#else
				for (i=0 ; i<r_anumverts ; i++, fv++, av++, r_apverts++, pstverts++){
							R_AliasTransformFinalVert (fv, av, r_apverts, pstverts);
			if (av->fv[2] < ALIAS_Z_CLIP_PLANE)
			fv->flags |= ALIAS_Z_CLIP;
		else
		{
			 R_AliasProjectFinalVert (fv, av);

			if (fv->v[0] < r_refdef.aliasvrect.x)
				fv->flags |= ALIAS_LEFT_CLIP;
			if (fv->v[1] < r_refdef.aliasvrect.y)
				fv->flags |= ALIAS_TOP_CLIP;
			if (fv->v[0] > r_refdef.aliasvrectright)
				fv->flags |= ALIAS_RIGHT_CLIP;
			if (fv->v[1] > r_refdef.aliasvrectbottom)
				fv->flags |= ALIAS_BOTTOM_CLIP;
		}
	}

#endif
	

	
//
// clip and draw all triangles
//
	r_affinetridesc.numtriangles = 1;

	ptri = (mtriangle_t *)((byte *)paliashdr + paliashdr->triangles);
	for (i=0 ; i<pmdl->numtris ; i++, ptri++)
	{
		pfv[0] = &pfinalverts[ptri->vertindex[0]];
		pfv[1] = &pfinalverts[ptri->vertindex[1]];
		pfv[2] = &pfinalverts[ptri->vertindex[2]];

		if ( pfv[0]->flags & pfv[1]->flags & pfv[2]->flags & (ALIAS_XY_CLIP_MASK | ALIAS_Z_CLIP) )
			continue;		// completely clipped

		if ( ! ( (pfv[0]->flags | pfv[1]->flags | pfv[2]->flags) &
			(ALIAS_XY_CLIP_MASK | ALIAS_Z_CLIP) ) )
		{	// totally unclipped
			r_affinetridesc.pfinalverts = pfinalverts;
			r_affinetridesc.ptriangles = ptri;
			D_PolysetDraw ();
		}
		else
		{	// partially clipped
			if(coloredlights)
				R_AliasClipTriangleRGB (ptri);
			else
			R_AliasClipTriangle (ptri);
		}
	}

}


//johnfitz -- struct for passing lerp information to drawing functions
typedef struct {
	short pose1;
	short pose2;
	float blend;
	vec3_t origin;
	vec3_t angles;
} lerpdata_t;
//johnfitz


/*
================
R_AliasSetUpTransform
================
*/
void R_AliasSetUpTransform (int trivial_accept)
{
	int				i;
	float			rotationmatrix[3][4], t2matrix[3][4];
	static float	tmatrix[3][4];
	static float	viewmatrix[3][4];
	vec3_t			angles;
#ifdef SCALEE
	float		scel;
	scel =  currententity->scale2;
#endif
// TODO: should really be stored with the entity instead of being reconstructed
// TODO: should use a look-up table
// TODO: could cache lazily, stored in the entity

	angles[ROLL] = currententity->angles[ROLL];
	angles[PITCH] = -currententity->angles[PITCH];
	angles[YAW] = currententity->angles[YAW];
	AngleVectors (angles, alias_forward, alias_right, alias_up);

// Manoel Kasimier - QC Scale - begin
#ifdef SCALEE
	tmatrix[0][0] = pmdl->scale[0] * scel;
	tmatrix[1][1] = pmdl->scale[1] * scel;
	tmatrix[2][2] = pmdl->scale[2] * scel;

	tmatrix[0][3] = pmdl->scale_origin[0] * scel;
	tmatrix[1][3] = pmdl->scale_origin[1] * scel;
	tmatrix[2][3] = pmdl->scale_origin[2] * scel;
	// Manoel Kasimier - QC Scale - end
#else
	tmatrix[0][0] = pmdl->scale[0];
	tmatrix[1][1] = pmdl->scale[1];
	tmatrix[2][2] = pmdl->scale[2];

	tmatrix[0][3] = pmdl->scale_origin[0];
	tmatrix[1][3] = pmdl->scale_origin[1];
	tmatrix[2][3] = pmdl->scale_origin[2];
#endif
// TODO: can do this with simple matrix rearrangement




// leilei - unrolled

		t2matrix[0][0] = alias_forward[0];
		t2matrix[0][1] = -alias_right[0];
		t2matrix[0][2] = alias_up[0];
		t2matrix[1][0] = alias_forward[1];
		t2matrix[1][1] = -alias_right[1];
		t2matrix[1][2] = alias_up[1];
		t2matrix[2][0] = alias_forward[2];
		t2matrix[2][1] = -alias_right[2];
		t2matrix[2][2] = alias_up[2];

	t2matrix[0][3] = -modelorg[0];
	t2matrix[1][3] = -modelorg[1];
	t2matrix[2][3] = -modelorg[2];

// FIXME: can do more efficiently than full concatenation
	R_ConcatTransforms (t2matrix, tmatrix, rotationmatrix);

// TODO: should be global, set when vright, etc., set
	VectorCopy (vright, viewmatrix[0]);
	VectorCopy (vup, viewmatrix[1]);
	VectorInverse (viewmatrix[1]);
	VectorCopy (vpn, viewmatrix[2]);


	R_ConcatTransforms (viewmatrix, rotationmatrix, aliastransform);

// do the scaling up of x and y to screen coordinates as part of the transform
// for the unclipped case (it would mess up clipping in the clipped case).
// Also scale down z, so 1/z is scaled 31 bits for free, and scale down x and y
// correspondingly so the projected x and y come out right
// FIXME: make this work for clipped case too?
	if (trivial_accept)
	{

		// leilei - unrolled

					aliastransform[0][0] *= aliasxscale *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[1][0] *= aliasyscale *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[2][0] *= 1.0 / ((float)0x8000 * 0x10000);


						aliastransform[0][1] *= aliasxscale *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[1][1] *= aliasyscale *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[2][1] *= 1.0 / ((float)0x8000 * 0x10000);


						aliastransform[0][2] *= aliasxscale *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[1][2] *= aliasyscale *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[2][2] *= 1.0 / ((float)0x8000 * 0x10000);


						aliastransform[0][3] *= aliasxscale *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[1][3] *= aliasyscale *
					(1.0 / ((float)0x8000 * 0x10000));
			aliastransform[2][3] *= 1.0 / ((float)0x8000 * 0x10000);

	}
}

#endif


#ifdef INTERPOL7
extern cvar_t *temp2;

int R_AliasLightVert (aliaslerp_t *pverts)
{
	float	temp;
	float	lightcos, *plightnormal;

	temp = r_ambientlight;

	plightnormal = r_avertexnormals[pverts->currlightnormal];
//	somelightnormal = r_avertexnormals[pverts->currlightnormal];
	lightcos = DotProduct (plightnormal, r_plightvec) * pverts->blend;


	if (lightcos < 0)
	{
		temp += (int) (r_shadelight * lightcos);

		// clamp; because we limited the minimum ambient and shading light, we
		// don't have to clamp low light, just bright
		if (temp < 0) temp = 0;
	}

	plightnormal = r_avertexnormals[pverts->lastlightnormal];
//	somelightnormal = r_avertexnormals[pverts->lastlightnormal];
	lightcos = DotProduct (plightnormal, r_plightvec) * (1.0f - pverts->blend);

	if (lightcos < 0)
	{
		temp += (int) (r_shadelight * lightcos);

		// clamp; because we limited the minimum ambient and shading light, we
		// don't have to clamp low light, just bright
		if (temp < 0) temp = 0;
	}
	return (int) temp;
}


// leilei - colored light versions!
int R_AliasLightVertRed (aliaslerp_t *pverts)
{
	float	temp;
	float	lightcos, *plightnormal;

	temp = r_ramb;

	plightnormal = r_avertexnormals[pverts->currlightnormal];
	//somelightnormal = r_avertexnormals[pverts->currlightnormal];
	lightcos = DotProduct (plightnormal, r_rlightvec) * pverts->blend;

	if (lightcos < 0)
	{
		temp += (int) (r_rlight * lightcos);
		if (temp < 0) temp = 0;
	}

	plightnormal = r_avertexnormals[pverts->lastlightnormal];
	//somelightnormal = r_avertexnormals[pverts->lastlightnormal];
	lightcos = DotProduct (plightnormal, r_rlightvec) * (1.0f - pverts->blend);

	if (lightcos < 0)
	{
		temp += (int) (r_rlight * lightcos);
		if (temp < 0) temp = 0;
	}


	// leilei - colored lighting on models


	// leilei - transfer

	return (int) temp;
}

int R_AliasLightVertGreen (aliaslerp_t *pverts)
{
	float	temp;
	float	lightcos, *plightnormal;

	temp = r_gamb;

	plightnormal = r_avertexnormals[pverts->currlightnormal];
	lightcos = DotProduct (plightnormal, r_glightvec) * pverts->blend;

	if (lightcos < 0)
	{
		temp += (int) (r_glight * lightcos);
		if (temp < 0) temp = 0;
	}

	plightnormal = r_avertexnormals[pverts->lastlightnormal];
	lightcos = DotProduct (plightnormal, r_glightvec) * (1.0f - pverts->blend);

	if (lightcos < 0)
	{
		temp += (int) (r_glight * lightcos);
		if (temp < 0) temp = 0;
	}
	// leilei - colored lighting on models

	// leilei - transfer

	return (int) temp;
}

int R_AliasLightVertBlue (aliaslerp_t *pverts)
{
	float	temp;
	float	lightcos, *plightnormal;

	temp = r_bamb;

	plightnormal = r_avertexnormals[pverts->currlightnormal];
	lightcos = DotProduct (plightnormal, r_blightvec) * pverts->blend;

	if (lightcos < 0)
	{
		temp += (int) (r_blight * lightcos);
		if (temp < 0) temp = 0;
	}

	plightnormal = r_avertexnormals[pverts->lastlightnormal];
	lightcos = DotProduct (plightnormal, r_blightvec) * (1.0f - pverts->blend);

	if (lightcos < 0)
	{
		temp += (int) (r_blight * lightcos);
		if (temp < 0) temp = 0;
	}
	// leilei - colored lighting on models

	// leilei - transfer

	return (int) temp;
}

#endif

/*
================
R_AliasTransformFinalVert

================
*/
extern cvar_t *temp2;
#ifdef INTERPOL7
void R_AliasTransformFinalVert (finalvert_t *fv, auxvert_t *av, aliaslerp_t *pverts, stvert_t *pstverts)
#else
void R_AliasTransformFinalVert (finalvert_t *fv, auxvert_t *av,	trivertx_t *pverts, stvert_t *pstverts)
#endif
{
	int		temp;
	float	lightcos, *plightnormal;

	

	av->fv[0] = DotProduct(pverts->v, aliastransform[0]) +	aliastransform[0][3];
	av->fv[1] = DotProduct(pverts->v, aliastransform[1]) +	aliastransform[1][3];
	av->fv[2] = DotProduct(pverts->v, aliastransform[2]) +	aliastransform[2][3];

	

	fv->v[2] = pstverts->s;
	fv->v[3] = pstverts->t;

	fv->flags = pstverts->onseam;
	
// lighting
#ifndef INTERPOL7
	plightnormal = r_avertexnormals[pverts->lightnormalindex];
	
	lightcos = DotProduct (plightnormal, r_plightvec);

	temp = r_ambientlight;

	if (lightcos < 0)
	{
		temp += (int)(r_shadelight * lightcos);

	// clamp; because we limited the minimum ambient and shading light, we
	// don't have to clamp low light, just bright
		if (temp < 0)
			temp = 0;
	}

	


	fv->v[4] = temp;

	if(coloredlights){
			// leilei - colored lighting on models
	float	rlightcos;
	float	glightcos;
	float	blightcos;
	int		tempr, tempg, tempb;

			// leilei - model lighting
	
	rlightcos = DotProduct (plightnormal, r_rlightvec);
	glightcos = DotProduct (plightnormal, r_glightvec);
	blightcos = DotProduct (plightnormal, r_blightvec);
	
	tempr = r_ramb;
	tempg = r_gamb;
	tempb = r_bamb;

	// leilei - clamp rgb 
	if (rlightcos < 0){ tempr += (int)(r_rlight * rlightcos);	if (tempr < 0) tempr = 0;}
	if (glightcos < 0){ tempg += (int)(r_glight * glightcos);	if (tempg < 0) tempg = 0;}
	if (blightcos < 0){ tempb += (int)(r_blight * blightcos);	if (tempb < 0) tempb = 0;}


	// leilei - transfer
	
	fv->v[6] = tempr;	// leilei- model colored lighting
	fv->v[7] = tempg;	// this transfers the lighting to the
	fv->v[8] = tempb;	// draw spans calls and such!
	

	}
#else
	
	


	fv->v[4] = R_AliasLightVert (pverts);
	if (coloredlights){
	fv->v[6] = R_AliasLightVertRed (pverts);
	fv->v[7] = R_AliasLightVertGreen (pverts);
	fv->v[8] = R_AliasLightVertBlue (pverts);
	}
	

	
#endif
}

#if	!id386broken

/*
================
R_AliasTransformAndProjectFinalVerts

================
*/

#ifdef INTERPOL7

/*
================
R_AliasTransformAndProjectFinalVerts
================
*/
void R_AliasTransformAndProjectFinalVertsLerp (finalvert_t *fv, stvert_t *pstverts)
{
	int			i;
	float		zi;
	aliaslerp_t *pverts;

	pverts = r_aliaslerpverts;

	for (i = 0; i < r_anumverts; i++, fv++, pverts++, pstverts++)
	{
		// transform and project
		zi = 1.0 / (DotProduct (pverts->v, aliastransform[2]) + aliastransform[2][3]);

		// x, y, and z are scaled down by 1/2**31 in the transform, so 1/z is
		// scaled up by 1/2**31, and the scaling cancels out for x and y in the
		// projection
		fv->v[5] = zi;
		fv->v[0] = ((DotProduct (pverts->v, aliastransform[0]) + aliastransform[0][3]) * zi) + aliasxcenter;
		fv->v[1] = ((DotProduct (pverts->v, aliastransform[1]) + aliastransform[1][3]) * zi) + aliasycenter;
		fv->v[2] = pstverts->s;
		fv->v[3] = pstverts->t;
		fv->flags = pstverts->onseam;

		// lighting
		fv->v[4] = R_AliasLightVert (pverts);
		if (coloredlights){
		fv->v[6] = R_AliasLightVertRed (pverts);
		fv->v[7] = R_AliasLightVertGreen (pverts);
		fv->v[8] = R_AliasLightVertBlue (pverts);
		}
	}
}

#endif

void R_AliasTransformAndProjectFinalVerts (finalvert_t *fv, stvert_t *pstverts)
{

	int			i, temp;

	float		lightcos, *plightnormal, zi;


	trivertx_t	*pverts;

	pverts = r_apverts;

	for (i=0 ; i<r_anumverts ; i++, fv++, pverts++, pstverts++)
	{
	// transform and project
		zi = 1.0 / (DotProduct(pverts->v, aliastransform[2]) +
				aliastransform[2][3]);

	// x, y, and z are scaled down by 1/2**31 in the transform, so 1/z is
	// scaled up by 1/2**31, and the scaling cancels out for x and y in the
	// projection
		fv->v[5] = zi;

		fv->v[0] = ((DotProduct(pverts->v, aliastransform[0]) +
				aliastransform[0][3]) * zi) + aliasxcenter;
		fv->v[1] = ((DotProduct(pverts->v, aliastransform[1]) +
				aliastransform[1][3]) * zi) + aliasycenter;

		fv->v[2] = pstverts->s;
		fv->v[3] = pstverts->t;
		fv->flags = pstverts->onseam;

	// lighting


		plightnormal = r_avertexnormals[pverts->lightnormalindex];
		//somelightnormal = r_avertexnormals[pverts->lightnormalindex];
		
		lightcos = DotProduct (plightnormal, r_plightvec);


		temp = r_ambientlight;

		if (lightcos < 0){temp += (int)(r_shadelight * lightcos);if (temp < 0)	temp = 0;}
	

		fv->v[4] = temp;

		if(coloredlights){
					// leilei - colored lighting on models
				int tempr, tempg, tempb;
	float	rlightcos;
	float	glightcos;
	float	blightcos;
	// lighting
	
		rlightcos = DotProduct (plightnormal, r_rlightvec);
		glightcos = DotProduct (plightnormal, r_glightvec);
		blightcos = DotProduct (plightnormal, r_blightvec);
	
		tempr = r_ramb;
		tempg = r_gamb;
		tempb = r_bamb;


	// leilei - clamp rgb 
	if (rlightcos < 0){ tempr += (int)(r_rlight * rlightcos);	if (tempr < 0) tempr = 0;}
	if (glightcos < 0){ tempg += (int)(r_glight * glightcos);	if (tempg < 0) tempg = 0;}
	if (blightcos < 0){ tempb += (int)(r_blight * blightcos);	if (tempb < 0) tempb = 0;}
	
		fv->v[6] = tempr;	// leilei- model colored lighting
		fv->v[7] = tempg;	// this transfers the lighting to the
		fv->v[8] = tempb;	// draw spans calls and such!

		}
	}
}


#endif

/*
================
R_AliasProjectFinalVert
================
*/
void R_AliasProjectFinalVert (finalvert_t *fv, auxvert_t *av)
{
	float	zi;

// project points
	zi = 1.0 / av->fv[2];

	fv->v[5] = zi * ziscale;

	fv->v[0] = (av->fv[0] * aliasxscale * zi) + aliasxcenter;
	fv->v[1] = (av->fv[1] * aliasyscale * zi) + aliasycenter;

}


/*
================
R_AliasPrepareUnclippedPoints
================
*/
void R_AliasPrepareUnclippedPoints (void)
{
	stvert_t	*pstverts;
	finalvert_t	*fv;

	pstverts = (stvert_t *)((byte *)paliashdr + paliashdr->stverts);
	r_anumverts = pmdl->numverts;
// FIXME: just use pfinalverts directly?
	fv = pfinalverts;
#ifdef INTERPOL7
	R_AliasTransformAndProjectFinalVertsLerp (fv, pstverts);
#else
	R_AliasTransformAndProjectFinalVerts (fv, pstverts);
#endif

	if (r_affinetridesc.drawtype)
		D_PolysetDrawFinalVerts (fv, r_anumverts);

	r_affinetridesc.pfinalverts = pfinalverts;
	r_affinetridesc.ptriangles = (mtriangle_t *)
			((byte *)paliashdr + paliashdr->triangles);
	r_affinetridesc.numtriangles = pmdl->numtris;

	D_PolysetDraw ();
}

/*
===============
R_AliasSetupSkin
===============
*/
void R_AliasSetupSkin (void)
{
	int					skinnum;
	int					i, numskins;
	maliasskingroup_t	*paliasskingroup;
	float				*pskinintervals, fullskininterval;
	float				skintargettime, skintime;

	skinnum = currententity->skinnum;
	if ((skinnum >= pmdl->numskins) || (skinnum < 0))
	{
		Con_DPrintf ("R_AliasSetupSkin: no such skin # %d\n", skinnum);
		skinnum = 0;
	}

	pskindesc = ((maliasskindesc_t *)
			((byte *)paliashdr + paliashdr->skindesc)) + skinnum;
	a_skinwidth = pmdl->skinwidth;

	if (pskindesc->type == ALIAS_SKIN_GROUP)
	{
		paliasskingroup = (maliasskingroup_t *)((byte *)paliashdr +
				pskindesc->skin);
		pskinintervals = (float *)
				((byte *)paliashdr + paliasskingroup->intervals);
		numskins = paliasskingroup->numskins;
		fullskininterval = pskinintervals[numskins-1];

		skintime = cl.time + currententity->syncbase;

	// when loading in Mod_LoadAliasSkinGroup, we guaranteed all interval
	// values are positive, so we don't have to worry about division by 0
		skintargettime = skintime -
				((int)(skintime / fullskininterval)) * fullskininterval;

		for (i=0 ; i<(numskins-1) ; i++)
		{
			if (pskinintervals[i] > skintargettime)
				break;
		}

		pskindesc = &paliasskingroup->skindescs[i];
	}

	r_affinetridesc.pskindesc = pskindesc;
	r_affinetridesc.pskin = (void *)((byte *)paliashdr + pskindesc->skin);
	r_affinetridesc.skinwidth = a_skinwidth;
	r_affinetridesc.seamfixupX16 =  (a_skinwidth >> 1) << 16;
	r_affinetridesc.skinheight = pmdl->skinheight;
}

/*
================
R_AliasSetupLighting_new

  Ripped out of Makaqu 1.3.1
  Hacked to support the colored lighting and more 
================
*/
// Manoel Kasimier - begin
void R_ParticleWat (vec3_t org, int colr); // debugging particles

extern cvar_t *temp2;
// Heavily edited Makaqu Chiaroscuro code, made mostly for trying to sample our pointlighting
// to make models look better in the world. but may look bad on bsps with light entities stripped
// basically this is chiaroscuro on crack

extern vec3_t lightcolor; // LordHavoc: used by model rendering
void R_AliasSetupLighting_enhanced (alight_t *plighting)
{
	float		fakegrid; // leilei
	float		lightvec[3];
	float		rlightvec[3];
	float		glightvec[3];
	float		blightvec[3];
	float		colr;
	float		colg;
	float		colb;
	vec3_t		dist;
	vec3_t		distr, distg, distb;
	float		add, radd, gadd, badd;
	float		amblevel, shadlevel;
	int			lnum;
	
	dlight_t	*dl;
	vec3_t		t;
	

	//begin
	lightvec[0] = 0;
	lightvec[1] = 0;
	lightvec[2] = 0;
	if (coloredlights){
	rlightvec[0] = 0;
	rlightvec[1] = 0;
	rlightvec[2] = 0;
	glightvec[0] = 0;
	glightvec[1] = 0;
	glightvec[2] = 0;
	blightvec[0] = 0;
	blightvec[1] = 0;
	blightvec[2] = 0;



	}

	VectorCopy(currententity->origin, t);
	r_shadelight = R_LightPoint (t);
	r_rlight = lightcolor[0];
	r_glight = lightcolor[1];
	r_blight = lightcolor[2];
	if (r_shading->value == 3){
		amblevel = 0.01;
		r_shadelight = r_ambientlight * amblevel  ;
		r_rlight = r_ramb  * amblevel;
		r_glight = r_gamb  * amblevel;
		r_blight = r_bamb * amblevel;
		shadlevel = 3.4;

	}
	else{
		amblevel = 0.3;
		shadlevel = 1;
	}

	// add dynamic lights including world lights
	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		float scale, scaler, scaleg, scaleb;
		dl = &cl_dlights[lnum];
		if (!dl->radius)
			continue;
		if (dl->die < cl.time)
			continue;
		colr = dl->color[0];
		colg = dl->color[1];
		colb = dl->color[2];
		VectorSubtract (t, dl->origin, dist);
		VectorSubtract (t, dl->origin, distr);
		VectorSubtract (t, dl->origin, distg);
		VectorSubtract (t, dl->origin, distb);


		add =  dl->radius - Length(dist);
		radd =  dl->radius * colr - Length(distr);
		gadd =  dl->radius * colg - Length(distg);
		badd =  dl->radius * colb - Length(distb);
		scale = 1.0-(Length(dist)/dl->radius);

		if (dl->unmark)
		scale *= scale * 0.1f * shadlevel;	// leilei - have soft lighting for world
		else
		scale *= scale * 0.4f * shadlevel;	// leilei - slightly more harsh for dyn lights

		
		scaler = 1-(Length(distr)/dl->radius);
		scaleg = 1-(Length(distg)/dl->radius);
		scaleb = 1-(Length(distb)/dl->radius);

		if (dl->unmark){
			scaler *= scaler * 0.054f * shadlevel;
			scaleg *= scaleg * 0.054f * shadlevel;
			scaleb *= scaleb * 0.054f * shadlevel;

		}
		else
		{
			if (r_shading->value == 3){
			scaler *= scaler * (0.054f) * shadlevel;
			scaleg *= scaleg * (0.054f) * shadlevel;
			scaleb *= scaleb * (0.054f) * shadlevel;
			}	else	{
			scaler *= scaler * (0.44f) * shadlevel;
			scaleg *= scaleg * (0.44f) * shadlevel;
			scaleb *= scaleb * (0.44f) * shadlevel;
			}
		}


		if (add > 0)
		{

			
			r_shadelight += add*(scale);
			r_rlight += radd*(scale);
			r_glight += gadd*(scale);
			r_blight += badd*(scale);
			VectorScale (dist, scale, dist);
			VectorScale (distr, scaler, distr);
			VectorScale (distg, scaleg, distg);
			VectorScale (distb, scaleb, distb);
			

			if (dl->dark)
			{
				VectorSubtract(lightvec, dist, lightvec);
			}
			else{
				VectorAdd(lightvec, dist, lightvec);
				VectorAdd(rlightvec, distr, rlightvec);
				VectorAdd(glightvec, distg, glightvec);
				VectorAdd(blightvec, distb, blightvec);
			}
		}
	}

	r_ambientlight = plighting->ambientlight * amblevel;


	r_ambientlight = (255 - r_ambientlight) << VID_CBITS;

	if (r_ambientlight < LIGHT_MIN)
		r_ambientlight = LIGHT_MIN;
	




	if (r_shadelight < 0)
		r_shadelight = 0;
	else
		r_shadelight = (float)(r_shadelight * VID_GRADES);

		r_rlight = (float)(r_rlight * VID_GRADES);
		r_glight = (float)(r_glight * VID_GRADES);
		r_blight = (float)(r_blight * VID_GRADES);

	// rotate the lighting vector into the model's frame of reference
	r_plightvec[0] = DotProduct (lightvec, alias_forward);
	r_plightvec[1] = -DotProduct (lightvec, alias_right);
	r_plightvec[2] = DotProduct (lightvec, alias_up);

	if (coloredlights){
	r_ramb = plighting->ramb*	amblevel;
	r_gamb = plighting->gamb*	amblevel;
	r_bamb = plighting->bamb* 	amblevel;

	if (r_ramb < 0)
			r_ramb = 0;
	if (r_gamb < 0)
			r_gamb = 0;
	if (r_bamb < 0)
			r_bamb = 0;

	r_ramb = (255 - r_ramb)<< VID_CBITS;
	if (r_ramb < LIGHT_MIN)	r_ramb = LIGHT_MIN;

	r_gamb = (255 - r_gamb)<< VID_CBITS;
	if (r_gamb < LIGHT_MIN)	r_gamb = LIGHT_MIN;

	r_bamb = (255 - r_bamb)<< VID_CBITS;
	if (r_bamb < LIGHT_MIN)	r_bamb = LIGHT_MIN;



			
	r_rlightvec[0] = DotProduct		(rlightvec, alias_forward);
	r_rlightvec[1] = -DotProduct	(rlightvec, alias_right);
	r_rlightvec[2] = DotProduct		(rlightvec, alias_up);

	r_glightvec[0] = DotProduct		(glightvec, alias_forward);
	r_glightvec[1] = -DotProduct	(glightvec, alias_right);
	r_glightvec[2] = DotProduct		(glightvec, alias_up);

	r_blightvec[0] = DotProduct		(blightvec, alias_forward);
	r_blightvec[1] = -DotProduct	(blightvec, alias_right);
	r_blightvec[2] = DotProduct		(blightvec, alias_up);



	}
	// shadow hack...
	{
		int e;
		
		for (e=0;e<3;e++){

			currententity->shadowvec[e] = r_plightvec[e];
			if(coloredlights){
				currententity->shadowvec[e] = rlightvec[e] * 0.333;
				currententity->shadowvec[e] += glightvec[e] * 0.333;
				currententity->shadowvec[e] += blightvec[e] * 0.333;

			}
		}
		currententity->shadowdist;
		if (coloredlights)
		currententity->shadowopacity = currententity->shadowvec[0] + currententity->shadowvec[1] + currententity->shadowvec[2] / 6;
			else
		currententity->shadowopacity = add / 255 * -1;
	}
};


/*
================
R_AliasSetupLighting
================
*/
void R_AliasSetupLighting (alight_t *plighting)
{

	// I don't know what happened to this originally, so let's lightpoint...

	

// guarantee that no vertex will ever be lit below LIGHT_MIN, so we don't have
// to clamp off the bottom
	r_ambientlight = plighting->ambientlight;

	if (r_ambientlight < LIGHT_MIN)
		r_ambientlight = LIGHT_MIN;

	r_ambientlight = (255 - r_ambientlight) << VID_CBITS;

	if (r_ambientlight < LIGHT_MIN)
		r_ambientlight = LIGHT_MIN;



	r_shadelight = plighting->shadelight;
	
	if (r_shadelight < 0) r_shadelight = 0;

	r_shadelight *= VID_GRADES;


// rotate the lighting vector into the model's frame of reference
	r_plightvec[0] = DotProduct (plighting->plightvec, alias_forward);
	r_plightvec[1] = -DotProduct (plighting->plightvec, alias_right);
	r_plightvec[2] = DotProduct (plighting->plightvec, alias_up);

	if (coloredlights){
	r_ramb = plighting->ramb;
	r_gamb = plighting->gamb;
	r_bamb = plighting->bamb;

	r_ramb = (255 - r_ramb) << VID_CBITS;
	if (r_ramb < LIGHT_MIN)	r_ramb = LIGHT_MIN;

	r_gamb = (255 - r_gamb) << VID_CBITS;
	if (r_gamb < LIGHT_MIN)	r_gamb = LIGHT_MIN;

	r_bamb = (255 - r_bamb) << VID_CBITS;
	if (r_bamb < LIGHT_MIN)	r_bamb = LIGHT_MIN;

	r_rlight = plighting->rlight;
	r_glight = plighting->blight;
	r_blight = plighting->glight;
	
	if (r_rlight < 0) r_rlight = 0;
	if (r_glight < 0) r_glight = 0;
	if (r_blight < 0) r_blight = 0;

	r_rlight *= VID_GRADES;
	r_glight *= VID_GRADES;
	r_blight *= VID_GRADES;
	

	r_rlightvec[0] = DotProduct (plighting->rlightvec, alias_forward);
	r_rlightvec[1] = -DotProduct (plighting->rlightvec, alias_right);
	r_rlightvec[2] = DotProduct (plighting->rlightvec, alias_up);

	r_glightvec[0] = DotProduct (plighting->glightvec, alias_forward);
	r_glightvec[1] = -DotProduct (plighting->glightvec, alias_right);
	r_glightvec[2] = DotProduct (plighting->glightvec, alias_up);

	r_blightvec[0] = DotProduct (plighting->blightvec, alias_forward);
	r_blightvec[1] = -DotProduct (plighting->blightvec, alias_right);
	r_blightvec[2] = DotProduct (plighting->blightvec, alias_up);
	}
}

// stripped function just for ambient lighting only!
void R_AliasSetupLightingSimple (alight_t *plighting)
{

// guarantee that no vertex will ever be lit below LIGHT_MIN, so we don't have
// to clamp off the bottom
	r_ambientlight = plighting->ambientlight;

	if (r_ambientlight < LIGHT_MIN)
		r_ambientlight = LIGHT_MIN;

	r_ambientlight = (255 - r_ambientlight) << VID_CBITS;

	if (r_ambientlight < LIGHT_MIN)
		r_ambientlight = LIGHT_MIN;

	r_shadelight = 0;
	r_plightvec[0] = 0;
	r_plightvec[1] = 0;
	r_plightvec[2] = 0;

	if (coloredlights){
	r_ramb = plighting->ramb;
	r_gamb = plighting->gamb;
	r_bamb = plighting->bamb;

	r_ramb = (255 - r_ramb) << VID_CBITS;
	if (r_ramb < LIGHT_MIN)	r_ramb = LIGHT_MIN;

	r_gamb = (255 - r_gamb) << VID_CBITS;
	if (r_gamb < LIGHT_MIN)	r_gamb = LIGHT_MIN;

	r_bamb = (255 - r_bamb) << VID_CBITS;
	if (r_bamb < LIGHT_MIN)	r_bamb = LIGHT_MIN;

	r_rlight = 0;
	r_glight = 0;
	r_blight = 0;
	
	if (r_rlight < 0) r_rlight = 0;
	if (r_glight < 0) r_glight = 0;
	if (r_blight < 0) r_blight = 0;

	r_rlightvec[0] = 0;
	r_rlightvec[1] = 0;
	r_rlightvec[2] = 0;

	r_glightvec[0] = 0;
	r_glightvec[1] = 0;
	r_glightvec[2] = 0;

	r_blightvec[0] = 0;
	r_blightvec[1] = 0;
	r_blightvec[2] = 0;
	}
}

#ifdef INTERPOL7

void R_BoundPoseSingle (entity_t *ent, mdl_t *m)
{
	if (ent->currpose < 0) ent->currpose = 0;
	if (ent->currpose >= m->numframes) ent->currpose = m->numframes - 1;

	if (ent->lastpose < 0) ent->lastpose = 0;
	if (ent->lastpose >= m->numframes) ent->lastpose = m->numframes - 1;
}


void R_BoundPoseGroup (entity_t *ent, maliasgroup_t *m)
{
	if (ent->currpose < 0) ent->currpose = 0;
	if (ent->currpose >= m->numframes) ent->currpose = m->numframes - 1;

	if (ent->lastpose < 0) ent->lastpose = 0;
	if (ent->lastpose >= m->numframes) ent->lastpose = m->numframes - 1;
}

#endif

#ifdef INTERPOL7

extern cvar_t *temp2;
/*
=================
R_AliasSetupFrame

set currverts and lastverts and blend between the two frames
=================
*/
void R_AliasSetupFrame (entity_t *ent)
{
	int				frame;
	int				i, numframes;
	maliasgroup_t	*paliasgroup;
	float			*pintervals, fullinterval, targettime, time;
	trivertx_t		*currverts;
	trivertx_t		*lastverts;
	int				pose;
	float			blend;
	float			frame_interval;

	frame = ent->frame;

	if (!r_lerpmodels->value){
		ent->currpose = -1;
		ent->lastpose = -1;
		ent->lastframe = frame;
		ent->frame_start_time = cl.time;
		//ent->lastpose = ent->currpose = pose;
		blend = 0;
	}

	if ((frame >= pmdl->numframes) || (frame < 0))
	{
		Con_DPrintf ("R_AliasSetupFrame: no such frame %d\n", frame);
		frame = 0;
	}

	if (paliashdr->frames[frame].type == ALIAS_SINGLE)
	{
		pose = frame;
		frame_interval = 0.1f;
			

		// test for switching from a frame group to a single
		if (paliashdr->frames[ent->lastframe].type != ALIAS_SINGLE)
		{
			ent->currpose = -1;
			ent->lastpose = -1;
			ent->lastframe = frame;
		}
	}
	else
	{
		paliasgroup = (maliasgroup_t *) ((byte *) paliashdr + paliashdr->frames[frame].frame);
		pintervals = (float *) ((byte *) paliashdr + paliasgroup->intervals);

		numframes = paliasgroup->numframes;
		
		

		fullinterval = pintervals[numframes-1];
		time = cl.time + ent->syncbase;

		// when loading in Mod_LoadAliasGroup, we guaranteed all interval values
		// are positive, so we don't have to worry about division by 0
		targettime = time - ((int) (time / fullinterval)) * fullinterval;

		for (i = 0; i < (numframes - 1); i++)
			if (pintervals[i] > targettime)
				break;

		pose = i;

		frame_interval = fullinterval / paliasgroup->numframes;

		// test for switching between multiple frame groups in the same ent
		// also valid for switching from a single frame to a group frame
		if (ent->lastframe != frame)
		{
			ent->currpose = -1;
			ent->lastpose = -1;
			ent->lastframe = frame;
		}
		//	}
	}

	if (ent->currpose == -1 || ent->lastpose == -1)
	{
		// new entity; no blending yet
		ent->frame_start_time = cl.time;
		ent->lastpose = ent->currpose = pose;
		blend = 0;
	}
	else if (ent->lastpose == ent->currpose && ent->currpose == 0 && ent != &cl.viewent)
	{
		// "dying throes" interpolation bug - begin a new sequence with both poses the same
		// this happens when an entity is spawned client-side
		ent->frame_start_time = cl.time;
		ent->lastpose = ent->currpose = pose;
		blend = 0;
	}
	else if (pose == 0 && ent == &cl.viewent)
	{
		// don't interpolate from previous pose on frame 0 of the viewent
		ent->frame_start_time = cl.time;
		ent->lastpose = ent->currpose = pose;
		blend = 0;
	}

	else if (ent->currpose != pose)
	{
		// going to a new pose
		ent->frame_start_time = cl.time;
		ent->lastpose = ent->currpose;
		ent->currpose = pose;
		blend = 0;
	}
	else
	{
		// blending between 2 poses
		blend = (cl.time - ent->frame_start_time) / frame_interval;
	}

	if (cl.paused || blend > 1) blend = 1;

	if (paliashdr->frames[frame].type == ALIAS_SINGLE)
	{
		R_BoundPoseSingle (ent, pmdl);

		currverts = (trivertx_t *) ((byte *) paliashdr + paliashdr->frames[ent->currpose].frame);
		lastverts = (trivertx_t *) ((byte *) paliashdr + paliashdr->frames[ent->lastpose].frame);
	}
	else
	{
		R_BoundPoseGroup (ent, paliasgroup);

		currverts = (trivertx_t *) ((byte *) paliashdr + paliasgroup->frames[ent->currpose].frame);
		lastverts = (trivertx_t *) ((byte *) paliashdr + paliasgroup->frames[ent->lastpose].frame);
	}

	lerpverts = r_aliaslerpverts;

	for (i = 0; i < pmdl->numverts; i++, currverts++, lastverts++, lerpverts++)
	{
//		int eh;

		// leilei - muzzleflash fix hack?
			// unfortunately screws up scrag tails so don't do this
//		for (eh = 0; eh < 3; eh++){
//			if ((currverts->v[eh] - lastverts->v[eh]) > temp2->value)
//				blend = 0;
//		}
		lerpverts->v[0] = currverts->v[0] * blend + lastverts->v[0] * (1.0f - blend);
		lerpverts->v[1] = currverts->v[1] * blend + lastverts->v[1] * (1.0f - blend);
		lerpverts->v[2] = currverts->v[2] * blend + lastverts->v[2] * (1.0f - blend);

		lerpverts->currlightnormal = currverts->lightnormalindex;
		lerpverts->lastlightnormal = lastverts->lightnormalindex;
		lerpverts->blend = blend;
	}
}



#else

/*
=================
R_AliasSetupFrame

set r_apverts
=================
*/
void R_AliasSetupFrame (void)
{
	int				frame;
	int				i, numframes;
	maliasgroup_t	*paliasgroup;
	float			*pintervals, fullinterval, targettime, time;

	frame = currententity->frame;
	if ((frame >= pmdl->numframes) || (frame < 0))
	{
		Con_DPrintf ("R_AliasSetupFrame: no such frame %d\n", frame);
		frame = 0;
	}

	if (paliashdr->frames[frame].type == ALIAS_SINGLE)
	{
		r_apverts = (trivertx_t *)
				((byte *)paliashdr + paliashdr->frames[frame].frame);
		return;
	}
	
	paliasgroup = (maliasgroup_t *)
				((byte *)paliashdr + paliashdr->frames[frame].frame);
	pintervals = (float *)((byte *)paliashdr + paliasgroup->intervals);
	numframes = paliasgroup->numframes;
	fullinterval = pintervals[numframes-1];

	time = cl.time + currententity->syncbase;

//
// when loading in Mod_LoadAliasGroup, we guaranteed all interval values
// are positive, so we don't have to worry about division by 0
//
	targettime = time - ((int)(time / fullinterval)) * fullinterval;

	for (i=0 ; i<(numframes-1) ; i++)
	{
		if (pintervals[i] > targettime)
			break;
	}

	r_apverts = (trivertx_t *)
				((byte *)paliashdr + paliasgroup->frames[i].frame);
}

#endif

void R_AliasFrameVerts (int frame, float time, trivertx_t **verts)
{
	int	i, numposes;
	maliasgroup_t *paliasgroup;
	float *pintervals, fullinterval, targettime;

	if (paliashdr->frames[frame].type == ALIAS_SINGLE)
	{
		*verts = (trivertx_t *)((byte *)paliashdr + paliashdr->frames[frame].frame);
	}
	else 
	{
		paliasgroup = (maliasgroup_t *)
			((byte *)paliashdr + paliashdr->frames[frame].frame);
		pintervals = (float *)((byte *)paliashdr + paliasgroup->intervals);
		numposes = paliasgroup->frames[frame].numframes;
		fullinterval = pintervals[numposes-1];

	//
	// when loading in Mod_LoadAliasGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
	//
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numposes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		*verts = (trivertx_t *)((byte *)paliashdr + paliasgroup->frames[i].frame);
	}
}



extern byte *transTable;
extern byte *addTable;
extern byte *addTable33;

extern cvar_t *temp2;
/*
================
R_AliasDrawModel
================
*/
#ifdef VMTOC
void R_AliasDrawModel (alight_t *plighting, qboolean viewmodel)
#else
void R_AliasDrawModel (alight_t *plighting)
#endif
{
// h2
	int		mls;
	int		i, j;
	byte	*dest, *source, *sourceA;
// h2

	finalvert_t		finalverts[MAXALIASVERTS +
						((CACHE_SIZE - 1) / sizeof(finalvert_t)) + 1];
	auxvert_t		auxverts[MAXALIASVERTS];

	r_amodels_drawn++;

// cache align
	pfinalverts = (finalvert_t *)
			(((long)&finalverts[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));


#ifdef INTERPOL7
	if(r_lerpmodels->value)
	pauxverts = &r_auxverts[0];
	else
	pauxverts = &auxverts[0];
#else
	pauxverts = &auxverts[0];
#endif
	paliashdr = (aliashdr_t *)Mod_Extradata (currententity->model);
	pmdl = (mdl_t *)((byte *)paliashdr + paliashdr->model);

	R_AliasSetupSkin ();
#ifdef INTERPOL7
	if (r_lerpmodels->value)
	R_AliasSetUpTransform (0);
		else
	R_AliasSetUpTransform (currententity->trivial_accept);
//	R_AliasSetUpTransform (currententity->trivial_accept);
#else
	R_AliasSetUpTransform (currententity->trivial_accept);
#endif
	if (r_shading->value > 1 && (lightingavailable))
		R_AliasSetupLighting_enhanced (plighting);	// leilei - further hacked
	else if (r_shading->value > 1 && !lightingavailable)
		R_AliasSetupLighting (plighting);	// no lighting available so we fall back
		else
		if (r_shading->value)
		R_AliasSetupLighting (plighting);
		else
			R_AliasSetupLightingSimple (plighting);
#ifdef INTERPOL7
		R_AliasSetupFrame (currententity);
#else
		R_AliasSetupFrame ();
#endif


	if (!currententity->colormap)
		Sys_Error ("R_AliasDrawModel: !currententity->colormap");

	if (!coloredlights){
	r_affinetridesc.drawtype = (currententity->trivial_accept == 3) &&
			r_recursiveaffinetriangles;
	}
	
	if (r_affinetridesc.drawtype)
	{
		D_PolysetUpdateTables ();		// FIXME: precalc...

	}
	else
	{
#if	id386broken
		D_Aff8Patch (currententity->colormap);
#endif
	}

	acolormap = currententity->colormap;
#ifdef VMTOC
//	if (viewmodel)
//		ziscale = (float)0x8000 * (float)0x10000 * 3.0;
//	else
	 if (viewmodel)
		ziscale = (float)0x8000 * (float)0x10000 * 3.0;
	else
		ziscale = (float)0x8000 * (float)0x10000;
#else
	if (currententity != &cl.viewent)
		ziscale = (float)0x8000 * (float)0x10000;
	 else
		ziscale = (float)0x8000 * (float)0x10000 * 3.0;
#endif
	if (currententity->trivial_accept)
		R_AliasPrepareUnclippedPoints ();
	else
		R_AliasPreparePoints ();
}



