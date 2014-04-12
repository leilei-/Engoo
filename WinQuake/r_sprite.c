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
// r_sprite.c

#include "quakedef.h"
#include "r_local.h"

static int				clip_current;
static vec5_t			clip_verts[2][MAXWORKINGVERTS];
static int				sprite_width, sprite_height;

spritedesc_t			r_spritedesc;
extern	float lensreflection;
particle_t		*currentparticle;
flare_t		*currentflare;
extern	int flared;	// tells the renderer this is a flared flare
/*
================
R_RotateSprite
================
*/
extern cvar_t *temp2;
void R_RotateSprite (float beamlength)
{
	vec3_t	vec;

	if (beamlength == 0.0)
		return;

	VectorScale (r_spritedesc.vpn, -beamlength, vec);
	VectorAdd (r_entorigin, vec, r_entorigin);
	VectorSubtract (modelorg, vec, modelorg);
}

void R_RotateSprite2 (float beamlength)
{
	vec3_t	vec;

	if (beamlength == 0.0)
		return;

	VectorScale (r_spritedesc.vpn, -beamlength, vec);
	VectorAdd (currentparticle->org, vec, currentparticle->org);
	VectorSubtract (modelorg, vec, modelorg);
}


void R_RotateSprite3 (float beamlength)
{
	vec3_t	vec;

	if (beamlength == 0.0)
		return;

	VectorScale (r_spritedesc.vpn, -beamlength, vec);
	VectorAdd (currentflare->org, vec, currentflare->org);
	VectorSubtract (modelorg, vec, modelorg);
}


/*
// for sparks
void R_RotateSprite2And (float beamlength)
{
	vec3_t	vec;
	vec3_t	verc;

//	if (beamlength == 0.0)
//		return;

	VectorScale (r_spritedesc.vpn, -beamlength, vec);
	VectorAdd (currentparticle->org, vec, currentparticle->org);
	VectorSubtract (modelorg, vec, modelorg);
	VectorAdd(currentparticle->org,verc,  currentparticle->vel); 
	VectorAdd (verc, vec, verc);
	VectorSubtract (modelorg, vec, modelorg);
}

void R_RotateSprite3What (float beamlength)
{
	vec3_t	vec;
	float lensref;
	lensref = temp2->value + 1;
///	if (beamlength == 0.0)
//		return;

	//VectorScale (r_spritedesc.vpn, -beamlength, vec);
	VectorScale (r_spritedesc.vpn, lensref, vec);
//	VectorAdd (currentflare->org, vec, currentflare->org);
	VectorSubtract (modelorg, vec, modelorg);

	
}
*/


/*
=============
R_ClipSpriteFace

Clips the winding at clip_verts[clip_current] and changes clip_current
Throws out the back side
==============
*/
int R_ClipSpriteFace (int nump, clipplane_t *pclipplane)
{
	int		i, outcount;
	float	dists[MAXWORKINGVERTS+1];
	float	frac, clipdist, *pclipnormal;
	float	*in, *instep, *outstep, *vert2;

	clipdist = pclipplane->dist;
	pclipnormal = pclipplane->normal;

// calc dists
	if (clip_current)
	{
		in = clip_verts[1][0];
		outstep = clip_verts[0][0];
		clip_current = 0;
	}
	else
	{
		in = clip_verts[0][0];
		outstep = clip_verts[1][0];
		clip_current = 1;
	}

	instep = in;
	for (i=0 ; i<nump ; i++, instep += sizeof (vec5_t) / sizeof (float))
	{
		dists[i] = DotProduct (instep, pclipnormal) - clipdist;
	}

	
// handle wraparound case
	dists[nump] = dists[0];
	Q_memcpy (instep, in, sizeof (vec5_t));

	
// clip the winding
	instep = in;
	outcount = 0;

	for (i=0 ; i<nump ; i++, instep += sizeof (vec5_t) / sizeof (float))
	{
		if (dists[i] >= 0)
		{
			Q_memcpy (outstep, instep, sizeof (vec5_t));
			outstep += sizeof (vec5_t) / sizeof (float);
			outcount++;
		}

		if (dists[i] == 0 || dists[i+1] == 0)
			continue;

		if ( (dists[i] > 0) == (dists[i+1] > 0) )
			continue;

	// split it into a new vertex
		frac = dists[i] / (dists[i] - dists[i+1]);

		vert2 = instep + sizeof (vec5_t) / sizeof (float);

		outstep[0] = instep[0] + frac*(vert2[0] - instep[0]);
		outstep[1] = instep[1] + frac*(vert2[1] - instep[1]);
		outstep[2] = instep[2] + frac*(vert2[2] - instep[2]);
		outstep[3] = instep[3] + frac*(vert2[3] - instep[3]);
		outstep[4] = instep[4] + frac*(vert2[4] - instep[4]);

		outstep += sizeof (vec5_t) / sizeof (float);
		outcount++;
	}

	return outcount;

}


/*
================
R_SetupAndDrawSprite
================
*/
void R_SetupAndDrawSprite ()
{
	int			i, nump;
	float		dot, scale, *pv;
	vec5_t		*pverts;
	vec3_t		left, up, right, down, transformed, local;
	emitpoint_t	outverts[MAXWORKINGVERTS+1], *pout;
	
	dot = DotProduct (r_spritedesc.vpn, modelorg); 

// backface cull
	if (dot >= 0)
		return;

// build the sprite poster in worldspace 
/*
	VectorScale (r_spritedesc.vright, r_spritedesc.pspriteframe->right, right);
	VectorScale (r_spritedesc.vup, r_spritedesc.pspriteframe->up, up);
	VectorScale (r_spritedesc.vright, r_spritedesc.pspriteframe->left, left);
	VectorScale (r_spritedesc.vup, r_spritedesc.pspriteframe->down, down);


*/	
	// Manoel Kasimier - QC Scale - edited - begin
#ifdef SCALEE
	VectorScale (r_spritedesc.vright,	r_spritedesc.pspriteframe->right*currententity->scale2, right);
	VectorScale (r_spritedesc.vright,	r_spritedesc.pspriteframe->left	*currententity->scale2, left);

	VectorScale (r_spritedesc.vup,		r_spritedesc.pspriteframe->up	*currententity->scale2, up);
	VectorScale (r_spritedesc.vup,		r_spritedesc.pspriteframe->down	*currententity->scale2, down);
#else
	VectorScale (r_spritedesc.vright,	r_spritedesc.pspriteframe->right, right);
	VectorScale (r_spritedesc.vright,	r_spritedesc.pspriteframe->left, left);

	VectorScale (r_spritedesc.vup,		r_spritedesc.pspriteframe->up, up);
	VectorScale (r_spritedesc.vup,		r_spritedesc.pspriteframe->down, down);
#endif
	// Manoel Kasimier - QC Scale - edited - end

	pverts = clip_verts[0];

	pverts[0][0] = r_entorigin[0] +  up[0] + left[0];
	pverts[0][1] = r_entorigin[1] + up[1] + left[1];
	pverts[0][2] = r_entorigin[2] + up[2] + left[2];
	pverts[0][3] = 0;
	pverts[0][4] = 0;

	pverts[1][0] = r_entorigin[0] + up[0] + right[0];
	pverts[1][1] = r_entorigin[1] + up[1] + right[1];
	pverts[1][2] = r_entorigin[2] + up[2] + right[2];
	pverts[1][3] = sprite_width;
	pverts[1][4] = 0;

	pverts[2][0] = r_entorigin[0] + down[0] + right[0];
	pverts[2][1] = r_entorigin[1] + down[1] + right[1];
	pverts[2][2] = r_entorigin[2] + down[2] + right[2];
	pverts[2][3] = sprite_width;
	pverts[2][4] = sprite_height;

	pverts[3][0] = r_entorigin[0] + down[0] + left[0];
	pverts[3][1] = r_entorigin[1] + down[1] + left[1];
	pverts[3][2] = r_entorigin[2] + down[2] + left[2];
	pverts[3][3] = 0;
	pverts[3][4] = sprite_height;

// clip
	nump = 4;
	clip_current = 0;

	for (i=0 ; i<4 ; i++)
	{
		nump = R_ClipSpriteFace (nump, &view_clipplanes[i]);
		if (nump < 3)
			return;
		if (nump >= MAXWORKINGVERTS)
			Sys_Error("R_SetupAndDrawSprite: too many points");
	}

// transform vertices into viewspace and project
	pv = &clip_verts[clip_current][0][0];
	r_spritedesc.nearzi = -99999;
	// leilei - sprite clip crash fix..
	{
		VectorSubtract (pv, r_origin, local);
		TransformVector (local, transformed);	
		
		if (transformed[2] < 4)		// leilei - try not to draw sprites too close, or we'll overflow the screen cache and crash!
			return;
	}
	for (i=0 ; i<nump ; i++)
	{
		VectorSubtract (pv, r_origin, local);
		TransformVector (local, transformed);

	if (transformed[2] < NEAR_CLIP)
			transformed[2] = NEAR_CLIP;


		pout = &outverts[i];
		pout->zi = 1.0 / transformed[2];
			
		if (pout->zi > r_spritedesc.nearzi)
			r_spritedesc.nearzi = pout->zi;

		pout->s = pv[3];
		pout->t = pv[4];
		
		scale = xscale * pout->zi;
		pout->u = (xcenter + scale * transformed[0]);

		scale = yscale * pout->zi;
		pout->v = (ycenter - scale * transformed[1]);

		pv += sizeof (vec5_t) / sizeof (*pv);
		
	}

// draw it
	r_spritedesc.nump = nump;
	r_spritedesc.pverts = outverts;
	D_DrawSprite (0);
}


/*
================
R_GetSpriteframe
================
*/
mspriteframe_t *R_GetSpriteframe (msprite_t *psprite)
{
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;

	frame = currententity->frame;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		Con_Printf ("R_DrawSprite: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = cl.time + currententity->syncbase;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}


/*
================
R_DrawSprite
================
*/
void R_DrawSprite (void)
{
	int				i;
	msprite_t		*psprite;
	vec3_t			tvec;
	float			dot, angle, sr, cr;

	psprite = currententity->model->cache.data;

	r_spritedesc.pspriteframe = R_GetSpriteframe (psprite);
// LEIDTIED
	sprite_width = r_spritedesc.pspriteframe->width;
	sprite_height = r_spritedesc.pspriteframe->height;

// TODO: make this caller-selectable
	if (psprite->type == SPR_FACING_UPRIGHT)
	{
	// generate the sprite's axes, with vup straight up in worldspace, and
	// r_spritedesc.vright perpendicular to modelorg.
	// This will not work if the view direction is very close to straight up or
	// down, because the cross product will be between two nearly parallel
	// vectors and starts to approach an undefined state, so we don't draw if
	// the two vectors are less than 1 degree apart
		tvec[0] = -modelorg[0];
		tvec[1] = -modelorg[1];
		tvec[2] = -modelorg[2];
		VectorNormalize (tvec);
		dot = tvec[2];	// same as DotProduct (tvec, r_spritedesc.vup) because
						//  r_spritedesc.vup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		r_spritedesc.vup[0] = 0;
		r_spritedesc.vup[1] = 0;
		r_spritedesc.vup[2] = 1;
		r_spritedesc.vright[0] = tvec[1];
								// CrossProduct(r_spritedesc.vup, -modelorg,
		r_spritedesc.vright[1] = -tvec[0];
								// r_spritedesc.vright)
		r_spritedesc.vright[2] = 0;
		VectorNormalize (r_spritedesc.vright);
		r_spritedesc.vpn[0] = -r_spritedesc.vright[1];
		r_spritedesc.vpn[1] = r_spritedesc.vright[0];
		r_spritedesc.vpn[2] = 0;
					// CrossProduct (r_spritedesc.vright, r_spritedesc.vup,
					//  r_spritedesc.vpn)
	}
	else if (psprite->type == SPR_VP_PARALLEL)
	{
	// generate the sprite's axes, completely parallel to the viewplane. There
	// are no problem situations, because the sprite is always in the same
	// position relative to the viewer
		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vup[i] = vup[i];
			r_spritedesc.vright[i] = vright[i];
			r_spritedesc.vpn[i] = vpn[i];
		}
	}
	else if (psprite->type == SPR_VP_PARALLEL_UPRIGHT)
	{
	// generate the sprite's axes, with vup straight up in worldspace, and
	// r_spritedesc.vright parallel to the viewplane.
	// This will not work if the view direction is very close to straight up or
	// down, because the cross product will be between two nearly parallel
	// vectors and starts to approach an undefined state, so we don't draw if
	// the two vectors are less than 1 degree apart
		dot = vpn[2];	// same as DotProduct (vpn, r_spritedesc.vup) because
						//  r_spritedesc.vup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		r_spritedesc.vup[0] = 0;
		r_spritedesc.vup[1] = 0;
		r_spritedesc.vup[2] = 1;
		r_spritedesc.vright[0] = vpn[1];
										// CrossProduct (r_spritedesc.vup, vpn,
		r_spritedesc.vright[1] = -vpn[0];	//  r_spritedesc.vright)
		r_spritedesc.vright[2] = 0;
		VectorNormalize (r_spritedesc.vright);
		r_spritedesc.vpn[0] = -r_spritedesc.vright[1];
		r_spritedesc.vpn[1] = r_spritedesc.vright[0];
		r_spritedesc.vpn[2] = 0;

			
					// CrossProduct (r_spritedesc.vright, r_spritedesc.vup,
					  //r_spritedesc.vpn);
	}
	else if (psprite->type == SPR_ORIENTED)
	{
	// generate the sprite's axes, according to the sprite's world orientation
		AngleVectors (currententity->angles, r_spritedesc.vpn,
					  r_spritedesc.vright, r_spritedesc.vup);

	}

	else if (psprite->type == SPR_VP_PARALLEL_ORIENTED)
	{
	// generate the sprite's axes, parallel to the viewplane, but rotated in
	// that plane around the center according to the sprite entity's roll
	// angle. So vpn stays the same, but vright and vup rotate
		angle = currententity->angles[ROLL] * (M_PI*2 / 360);
		angle = currententity->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}
	}
	else
	{
		angle = currententity->angles[ROLL] * (M_PI*2 / 360);
		angle = currententity->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}

	//	Sys_Error ("R_DrawSprite: Bad sprite type %d", psprite->type);
	}

	R_RotateSprite (psprite->beamlength);

	R_SetupAndDrawSprite ();
}
























extern float lensreflection;
void R_SetupAndDrawSprite2 (int pass)
{
	int			i, nump;
	float		dot, scale, *pv;
	vec5_t		*pverts;
	vec3_t		left, up, right, down, transformed, local;
	emitpoint_t	outverts[MAXWORKINGVERTS+1], *pout;
	vec3_t		forg;
	dot = DotProduct (r_spritedesc.vpn, modelorg); 

// backface cull
	if (dot >= 0)
		return;
	
	VectorCopy(currentparticle->org, forg);



// build the sprite poster in worldspace 

	VectorScale (r_spritedesc.vright,	r_spritedesc.pspriteframe->right*currentparticle->scale, right);
	VectorScale (r_spritedesc.vright,	r_spritedesc.pspriteframe->left	*currentparticle->scale, left);

	VectorScale (r_spritedesc.vup,		r_spritedesc.pspriteframe->up	*currentparticle->scaley, up);
	VectorScale (r_spritedesc.vup,		r_spritedesc.pspriteframe->down	*currentparticle->scaley, down);	 // leilei - oops

	pverts = clip_verts[0];

	pverts[0][0] = forg[0] + (up[0] + left[0]);
	pverts[0][1] = forg[1] + (up[1] + left[1]);
	pverts[0][2] = forg[2] + (up[2] + left[2]);
	pverts[0][3] = 0;
	pverts[0][4] = 0;

	pverts[1][0] = forg[0] + (up[0] + right[0]);
	pverts[1][1] = forg[1] + (up[1] + right[1]);
	pverts[1][2] = forg[2] + (up[2] + right[2]);
	pverts[1][3] = sprite_width;
	pverts[1][4] = 0;

	pverts[2][0] = forg[0] + (down[0] + right[0]);
	pverts[2][1] = forg[1] + (down[1] + right[1]);
	pverts[2][2] = forg[2] + (down[2] + right[2]);
	pverts[2][3] = sprite_width;
	pverts[2][4] = sprite_height;

	pverts[3][0] = forg[0] + (down[0] + left[0]);
	pverts[3][1] = forg[1] + (down[1] + left[1]);
	pverts[3][2] = forg[2] + (down[2] + left[2]);
	pverts[3][3] = 0;
	pverts[3][4] = sprite_height;

	
// clip
	nump = 4;
	clip_current = 0;

	for (i=0 ; i<4 ; i++)
	{
		nump = R_ClipSpriteFace (nump, &view_clipplanes[i]);
		if (nump < 3)
			return;
		if (nump >= MAXWORKINGVERTS)
			Sys_Error("R_SetupAndDrawSprite: too many points");
	}

// transform vertices into viewspace and project
	pv = &clip_verts[clip_current][0][0];
	r_spritedesc.nearzi = -99999;
	// leilei - sprite clip crash fix..
	{
		VectorSubtract (pv, r_origin, local);
		TransformVector (local, transformed);
	//	if (transformed[2] < 74 && currentparticle->stickable == 1){
	//		
	//		currentparticle->stickable == 2;
	//		
	//	}
/*		if (currentparticle->stickable == 2){
			VectorSubtract(forg, currentparticle->org, currentparticle->viewstuckangle);


		}
	
		if (transformed[2] < 74 && currentparticle->stickable == 1){
			currentparticle->stickable = 2;	// leilei - stick to screen particles
			currentparticle->frame = 1;
			currentparticle->type = pt_static;
			currentparticle->vel[0] = 0;
			currentparticle->vel[1] = 0;
			currentparticle->vel[2] = 0;

			VectorCopy(currentparticle->org, currentparticle->viewstuck);

			//VectorCopy(currentparticle->org, currentparticle->viewstuckangle);
			VectorCopy(currentparticle->org, currentparticle->viewstuckangle);
			currentparticle->viewstuckangle[0] = transformed[0];
			currentparticle->viewstuckangle[1] = transformed[1];
			currentparticle->viewstuck[0] = pv[3];
			currentparticle->viewstuck[1] = pv[4];

		}
*/
		if (transformed[2] < 5)		// leilei - no particle should ever be this close (performance reasons)
			return;
		
	}
	for (i=0 ; i<nump ; i++)
	{

		VectorSubtract (pv, r_origin, local);
		TransformVector (local, transformed);

	if (transformed[2] < NEAR_CLIP)
			transformed[2] = NEAR_CLIP;

		pout = &outverts[i];
		pout->zi = 1.0 / transformed[2];
			
		if (pout->zi > r_spritedesc.nearzi)
			r_spritedesc.nearzi = pout->zi;

		pout->s = pv[3];
		pout->t = pv[4];
/*		
		if (currentparticle->stickable == 2)
		{
		pout->s = currentparticle->viewstuck[0];
		pout->t = currentparticle->viewstuck[1];

		scale = xscale * pout->zi;
		pout->u = (xcenter + scale *  currentparticle->viewstuckangle[0]);

		scale = yscale * pout->zi;
		pout->v = (ycenter - scale  * currentparticle->viewstuckangle[1]);

		}

			else
			*/
		{
		scale = xscale * pout->zi;
		pout->u = (xcenter + scale * transformed[0]);

		scale = yscale * pout->zi;
		pout->v = (ycenter - scale  * transformed[1]);
		}

		pv += sizeof (vec5_t) / sizeof (*pv);
		
	}

// draw it
	r_spritedesc.nump = nump;
	r_spritedesc.pverts = outverts;


	D_DrawSprite (1);
}


// flares
void R_SetupAndDrawSprite3 ()
{
	int			i, nump;
	float		agh;
	vec3_t		aggh;
	float		dot, scale, *pv;
	vec5_t		*pverts;
	vec3_t		left, up, right, down, transformed, local;
	emitpoint_t	outverts[MAXWORKINGVERTS+1], *pout;
	agh = temp2->value;

	VectorCopy(r_spritedesc.vpn, aggh);
	//dot = DotProduct (r_spritedesc.vpn, modelorg); 
	dot = DotProduct (aggh, modelorg); 
	

// backface cull
	if (dot >= 0)
		return;
	
// build the sprite poster in worldspace 
/*
	VectorScale (r_spritedesc.vright, r_spritedesc.pspriteframe->right, right);
	VectorScale (r_spritedesc.vup, r_spritedesc.pspriteframe->up, up);
	VectorScale (r_spritedesc.vright, r_spritedesc.pspriteframe->left, left);
	VectorScale (r_spritedesc.vup, r_spritedesc.pspriteframe->down, down);


*/	

	//VectorScale (r_spritedesc.vright, agh, r_spritedesc.vright);
	//VectorScale (r_spritedesc.vup, agh, r_spritedesc.vup);
	VectorScale (r_spritedesc.vright,	r_spritedesc.pspriteframe->right*currentflare->scale, right);
	VectorScale (r_spritedesc.vright,	r_spritedesc.pspriteframe->left	*currentflare->scale, left);

	VectorScale (r_spritedesc.vup,		r_spritedesc.pspriteframe->up	*currentflare->scaley, up);
	VectorScale (r_spritedesc.vup,		r_spritedesc.pspriteframe->down	*currentflare->scaley, down);

	pverts = clip_verts[0];

	pverts[0][0] = currentflare->org[0] + ( up[0] + left[0]);
	pverts[0][1] = currentflare->org[1] + (up[1] + left[1]);
	pverts[0][2] = currentflare->org[2] + (up[2] + left[2]);
	pverts[0][3] = 0;
	pverts[0][4] = 0;

	pverts[1][0] = currentflare->org[0] + (up[0] + right[0]);
	pverts[1][1] = currentflare->org[1] + (up[1] + right[1]);
	pverts[1][2] = currentflare->org[2] + (up[2] + right[2]);
	pverts[1][3] = sprite_width;
	pverts[1][4] = 0;

	pverts[2][0] = currentflare->org[0] + (down[0] + right[0]);
	pverts[2][1] = currentflare->org[1] + (down[1] + right[1]);
	pverts[2][2] = currentflare->org[2] + (down[2] + right[2]);
	pverts[2][3] = sprite_width;
	pverts[2][4] = sprite_height;

	pverts[3][0] = currentflare->org[0] + (down[0] + left[0]);
	pverts[3][1] = currentflare->org[1] +( down[1] + left[1]);
	pverts[3][2] = currentflare->org[2] + (down[2] + left[2]);
	pverts[3][3] = 0;
	pverts[3][4] = sprite_height;
/*
	{
		int ehhhh;
		for (ehhhh = 0; ehhhh < 4; ehhhh++)
		{
			pverts[ehhhh][0] *= lensreflection;
			pverts[ehhhh][1] *= lensreflection;
			pverts[ehhhh][2] *= lensreflection;
			pverts[ehhhh][3] *= lensreflection;
			pverts[ehhhh][4] *= lensreflection;

		}
	}
	*/


// clip
	nump = 4;
	clip_current = 0;

	for (i=0 ; i<4 ; i++)
	{
		nump = R_ClipSpriteFace (nump, &view_clipplanes[i]);
		if (nump < 3)
			return;
		if (nump >= MAXWORKINGVERTS)
			Sys_Error("R_SetupAndDrawSprite: too many points");
	}

// transform vertices into viewspace and project
	pv = &clip_verts[clip_current][0][0];
	r_spritedesc.nearzi = -99999;
	// leilei - sprite clip crash fix..
	{
		VectorSubtract (pv, r_origin, local);
		TransformVector (local, transformed);
		
		if (transformed[2] < 4)		// leilei - try not to draw sprites too close, or we'll overflow the screen cache and crash!
			return;
	}
	for (i=0 ; i<nump ; i++)
	{
		VectorSubtract (pv, r_origin, local);
		TransformVector (local, transformed);

	if (transformed[2] < NEAR_CLIP)
			transformed[2] = NEAR_CLIP;

		pout = &outverts[i];
		pout->zi = 1.0 / transformed[2];
			
		if (pout->zi > r_spritedesc.nearzi)
			r_spritedesc.nearzi = pout->zi;

		pout->s = pv[3];
		pout->t = pv[4];
		
		scale = xscale * pout->zi;
		pout->u = (xcenter + scale * transformed[0]);

		scale = yscale * pout->zi;
		pout->v = (ycenter - scale  * transformed[1]);

		pv += sizeof (vec5_t) / sizeof (*pv);
		
	}

// draw it
	r_spritedesc.nump = nump;
	r_spritedesc.pverts = outverts;
	D_DrawSprite (2);
}



mspriteframe_t *R_GetSpriteframe2 (msprite_t *psprite)
{
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;

	frame = currentparticle->frame;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		Con_Printf ("R_DrawSprite: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = currentparticle->die;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}



// pass:

// 0 - modulates and subtracts
// 1 - alphas and everything else
// 2 - additives
int currentparticlepass;
void R_DrawSprite2 ()
{
	int				i;
	msprite_t		*psprite;
	vec3_t			tvec;
	float			dot, angle, sr, cr;
	int				pass = currentparticlepass;

	// leilei - stupid pass control
	if (currentparticle->blend == 0		&& currentparticlepass != 1) return;
	else if (currentparticle->blend == 1 && currentparticlepass != 2) return;
	else if (currentparticle->blend == 2 && currentparticlepass != 1) return;
	else if (currentparticle->blend == 3 && currentparticlepass != 1) return;
	else if (currentparticle->blend == 4 && currentparticlepass != 1) return;
	else if (currentparticle->blend == 5 && currentparticlepass != 2) return;
	else if (currentparticle->blend == 6 && currentparticlepass != 3) return;
	else if (currentparticle->blend == 7 && currentparticlepass != 1) return;
	else if (currentparticle->blend == 8 && currentparticlepass != 2) return;
	else if (currentparticle->blend == 9 && currentparticlepass != 1) return;
	else if (currentparticle->blend == 10 && currentparticlepass != 1) return;
//	else	return;	// nah.


	psprite = currentparticle->model->cache.data;

	r_spritedesc.pspriteframe = R_GetSpriteframe2 (psprite);
// LEIDTIED
	sprite_width = r_spritedesc.pspriteframe->width;
	sprite_height = r_spritedesc.pspriteframe->height;
	// We override the sprite type because we can. And if you ain't down with t...forget it

// TODO: make this caller-selectable
	if (currentparticle->sprtype == SPR_FACING_UPRIGHT)
	{
	// generate the sprite's axes, with vup straight up in worldspace, and
	// r_spritedesc.vright perpendicular to modelorg.
	// This will not work if the view direction is very close to straight up or
	// down, because the cross product will be between two nearly parallel
	// vectors and starts to approach an undefined state, so we don't draw if
	// the two vectors are less than 1 degree apart
		tvec[0] = -modelorg[0];
		tvec[1] = -modelorg[1];
		tvec[2] = -modelorg[2];
		VectorNormalize (tvec);
		dot = tvec[2];	// same as DotProduct (tvec, r_spritedesc.vup) because
						//  r_spritedesc.vup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		r_spritedesc.vup[0] = 0;
		r_spritedesc.vup[1] = 0;
		r_spritedesc.vup[2] = 1;
		r_spritedesc.vright[0] = tvec[1];
								// CrossProduct(r_spritedesc.vup, -modelorg,
		r_spritedesc.vright[1] = -tvec[0];
								// r_spritedesc.vright)
		r_spritedesc.vright[2] = 0;
		VectorNormalize (r_spritedesc.vright);
		r_spritedesc.vpn[0] = -r_spritedesc.vright[1];
		r_spritedesc.vpn[1] = r_spritedesc.vright[0];
		r_spritedesc.vpn[2] = 0;
					// CrossProduct (r_spritedesc.vright, r_spritedesc.vup,
					//  r_spritedesc.vpn)
	}
	else if (currentparticle->sprtype == SPR_VP_PARALLEL)
	{
	// generate the sprite's axes, completely parallel to the viewplane. There
	// are no problem situations, because the sprite is always in the same
	// position relative to the viewer
		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vup[i] = vup[i];
			r_spritedesc.vright[i] = vright[i];
			r_spritedesc.vpn[i] = vpn[i];
		}
	}
	else if (currentparticle->sprtype == SPR_VP_PARALLEL_UPRIGHT)
	{
	// generate the sprite's axes, with vup straight up in worldspace, and
	// r_spritedesc.vright parallel to the viewplane.
	// This will not work if the view direction is very close to straight up or
	// down, because the cross product will be between two nearly parallel
	// vectors and starts to approach an undefined state, so we don't draw if
	// the two vectors are less than 1 degree apart
		dot = vpn[2];	// same as DotProduct (vpn, r_spritedesc.vup) because
						//  r_spritedesc.vup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		r_spritedesc.vup[0] = 0;
		r_spritedesc.vup[1] = 0;
		r_spritedesc.vup[2] = 1;
		r_spritedesc.vright[0] = vpn[1];
										// CrossProduct (r_spritedesc.vup, vpn,
		r_spritedesc.vright[1] = -vpn[0];	//  r_spritedesc.vright)
		r_spritedesc.vright[2] = 0;
		VectorNormalize (r_spritedesc.vright);
		r_spritedesc.vpn[0] = -r_spritedesc.vright[1];
		r_spritedesc.vpn[1] = r_spritedesc.vright[0];
		r_spritedesc.vpn[2] = 0;

			
					// CrossProduct (r_spritedesc.vright, r_spritedesc.vup,
					  //r_spritedesc.vpn);
	}
	else if (currentparticle->sprtype == SPR_ORIENTED)
	{

	// generate the sprite's axes, according to the sprite's world orientation
		AngleVectors (currentparticle->angles, r_spritedesc.vpn,
					  r_spritedesc.vright, r_spritedesc.vup);


	}
	else if (currentparticle->sprtype == SPR_VP_PARALLEL_ORIENTED)
	{
	// generate the sprite's axes, parallel to the viewplane, but rotated in
	// that plane around the center according to the sprite particle's roll
	// angle. So vpn stays the same, but vright and vup rotate
		angle = currentparticle->angles[ROLL] * (M_PI*2 / 360);
		angle = currentparticle->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}
	}
		else if (currentparticle->sprtype == 98)
	{
			/*
		tvec[0] = -modelorg[0];
		tvec[1] = -modelorg[1];
		tvec[2] = -modelorg[2];
		VectorNormalize (tvec);
		dot = tvec[2];	// same as DotProduct (tvec, r_spritedesc.vup) because
						//  r_spritedesc.vup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;

		//r_spritedesc.vup[0] = 0;
		//r_spritedesc.vup[1] = 0;
		//r_spritedesc.vup[2] = 1;

		r_spritedesc.vright[0] = tvec[1];
								
		r_spritedesc.vright[1] = -tvec[0];
		r_spritedesc.vright[2] = 0;
		VectorNormalize (r_spritedesc.vright);
		r_spritedesc.vpn[0] = -r_spritedesc.vright[1];
		r_spritedesc.vpn[1] = r_spritedesc.vright[0];
		r_spritedesc.vpn[2] = 0;
					// CrossProduct (r_spritedesc.vright, r_spritedesc.vup,
					//  r_spritedesc.vpn)
		// attempt to do the beamy beam thingy beam.
	
	//				  
*/

//		AngleVectors (currentparticle->angles, r_spritedesc.vpn,
//					  r_spritedesc.vright, r_spritedesc.vup);
		AngleVectors (currentparticle->angles, r_spritedesc.vpn,
					   r_spritedesc.vup, r_spritedesc.vup);

					 CrossProduct (r_spritedesc.vup, r_spritedesc.vright, r_spritedesc.vpn);
			angle = currentparticle->angles[ROLL] * (M_PI*2 / 360);
		angle = currentparticle->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
		//	r_spritedesc.vpn[i] = vpn[i] * -sr + vup[i] * cr;
		//	r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
		//	r_spritedesc.vup[i] += vright[i] * -sr + vup[i] * cr;
		}
	}
	else if (currentparticle->sprtype == 5)
	{
	// like above but try to occlude check some how
	// angle. So vpn stays the same, but vright and vup rotate
		angle = currentparticle->angles[ROLL] * (M_PI*2 / 360);
		angle = currentparticle->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}
	}
	else
	{
	// like above but try to occlude check some how
	// angle. So vpn stays the same, but vright and vup rotate
		angle = currentparticle->angles[ROLL] * (M_PI*2 / 360);
		angle = currentparticle->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}
//		Sys_Error ("R_DrawSprite: Bad sprite type %d", psprite->type);
	}
	if(currentparticle->sprtype == 5)flared = 1; else		flared = 0;
	R_RotateSprite2 (psprite->beamlength);
	//R_RotateSprite2And (psprite->beamlength);
	
	// leilei - stupid pass control
		
	R_SetupAndDrawSprite2 (pass);
}













mspriteframe_t *R_GetSpriteframe3 (msprite_t *psprite)
{
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;

	frame = currentflare->frame;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		Con_Printf ("R_DrawSprite3: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = currentflare->die;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}



// Flares only do one type.
void R_DrawSprite3 (void)
{
	int				i;
	msprite_t		*psprite;
	vec3_t			tvec;
	float			dot, angle, sr, cr;

	psprite = currentflare->model->cache.data;

	r_spritedesc.pspriteframe = R_GetSpriteframe3 (psprite);
// LEIDTIED
	sprite_width = r_spritedesc.pspriteframe->width;
	sprite_height = r_spritedesc.pspriteframe->height;

	lensreflection =	currentflare->lensreflection;
	lensreflection = 1;
	
//	sprite_width *= lensreflection;
//	sprite_height *= lensreflection;
	{
	// generate the sprite's axes, completely parallel to the viewplane. There
	// are no problem situations, because the sprite is always in the same
	// position relative to the viewer
		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vup[i] = vup[i];
			r_spritedesc.vright[i] = vright[i];
			r_spritedesc.vpn[i] = vpn[i];
			
		}
	}
/*	else if (currentflare->sprtype == SPR_VP_PARALLEL_UPRIGHT)
	{
	// generate the sprite's axes, with vup straight up in worldspace, and
	// r_spritedesc.vright parallel to the viewplane.
	// This will not work if the view direction is very close to straight up or
	// down, because the cross product will be between two nearly parallel
	// vectors and starts to approach an undefined state, so we don't draw if
	// the two vectors are less than 1 degree apart
		dot = vpn[2];	// same as DotProduct (vpn, r_spritedesc.vup) because
						//  r_spritedesc.vup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		r_spritedesc.vup[0] = 0;
		r_spritedesc.vup[1] = 0;
		r_spritedesc.vup[2] = 1;
		r_spritedesc.vright[0] = vpn[1];
										// CrossProduct (r_spritedesc.vup, vpn,
		r_spritedesc.vright[1] = -vpn[0];	//  r_spritedesc.vright)
		r_spritedesc.vright[2] = 0;
		VectorNormalize (r_spritedesc.vright);
		r_spritedesc.vpn[0] = -r_spritedesc.vright[1];
		r_spritedesc.vpn[1] = r_spritedesc.vright[0];
		r_spritedesc.vpn[2] = 0;

			
					// CrossProduct (r_spritedesc.vright, r_spritedesc.vup,
					  //r_spritedesc.vpn);
	}
	else if (currentflare->sprtype == SPR_ORIENTED)
	{
	// generate the sprite's axes, according to the sprite's world orientation
		AngleVectors (currentflare->angles, r_spritedesc.vpn,
					  r_spritedesc.vright, r_spritedesc.vup);
	}
	else if (currentflare->sprtype == SPR_VP_PARALLEL_ORIENTED)
	{
	// generate the sprite's axes, parallel to the viewplane, but rotated in
	// that plane around the center according to the sprite particle's roll
	// angle. So vpn stays the same, but vright and vup rotate
		angle = currentflare->angles[ROLL] * (M_PI*2 / 360);
		angle = currentflare->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}
	}
	else if (currentflare->sprtype == 5)
	{
	// like above but try to occlude check some how
	// angle. So vpn stays the same, but vright and vup rotate
		angle = currentflare->angles[ROLL] * (M_PI*2 / 360);
		angle = currentflare->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}
	}
	else
	{
	// like above but try to occlude check some how
	// angle. So vpn stays the same, but vright and vup rotate
		angle = currentflare->angles[ROLL] * (M_PI*2 / 360);
		angle = currentflare->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}
//		Sys_Error ("R_DrawSprite: Bad sprite type %d", psprite->type);
	}
	*/
		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
		
		
		}
	if(currentflare->sprtype == 5 ||  currentflare->sprtype == SPR_VP_PARALLEL_UPRIGHT)flared = 1; else		flared = 0;
	R_RotateSprite3 (psprite->beamlength);
	
		
	R_SetupAndDrawSprite3 ();
}




