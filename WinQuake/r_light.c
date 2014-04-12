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
// r_light.c

#include "quakedef.h"
#include "r_local.h"

int	r_dlightframecount;
int	r_shadowframecount;
int	r_wlightframecount;

/*
==================
R_AnimateLight
==================
*/
void R_AnimateLight (void)
{
	int			i,j,k;

//
// light animations
// 'm' is normal light, 'a' is no light, 'z' is double bright
	i = (int)(cl.time*10);
	for (j=0 ; j<MAX_LIGHTSTYLES ; j++)
	{
		if (!cl_lightstyle[j].length)
		{
			d_lightstylevalue[j] = 256;
			continue;
		}
		k = i % cl_lightstyle[j].length;
		k = cl_lightstyle[j].map[k] - 'a';
		k = k*22;
		d_lightstylevalue[j] = k;
	}
}


/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/

extern unsigned int dynlightenabled;
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	unsigned int			i;
	// LordHavoc: .lit support begin (actually this is just a major lighting speedup, no relation to color :)
	float		l, maxdist;
	int		s;
	int t;
	vec3_t		impact;
	
	
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix
	


loc0:
	 
	// LordHavoc: .lit support end

	if (node->contents < 0)
		return;

	if(!dynlightenabled)
		return;

	if (light->unmark)
		return;	// don't do it if we don't want to mark it


	splitplane = node->plane; // LordHavoc: original code
	VectorSubtract (light->origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix
	if (splitplane->type < 3)
		dist = origin_for_ent[splitplane->type] - splitplane->dist;
	else
		dist = DotProduct (origin_for_ent, splitplane->normal) - splitplane->dist; // mankrip - dynamic lights on moving brush models fix - edited
		
	if (dist > light->radius)
	{
		node = node->children[0];
		goto loc0;
	}
	if (dist < -light->radius)
	{
		node = node->children[1];
		goto loc0;
	}

	maxdist = light->radius*light->radius;

// mark the polygons
	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
	
		// leilei - unrolled
		impact[0] = origin_for_ent[0] - surf->plane->normal[0]*dist; 
		impact[1] = origin_for_ent[1] - surf->plane->normal[1]*dist; // MH dlight fix
		impact[2] = origin_for_ent[2] - surf->plane->normal[2]*dist; 

		
		// clamp center of light to corner and check brightness
		l = DotProduct (impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
		s = l+0.5;if (s < 0) s = 0;else if (s > surf->extents[0]) s = surf->extents[0];
		s = l - s;
		l = DotProduct (impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
		t = l+0.5;if (t < 0) t = 0;else if (t > surf->extents[1]) t = surf->extents[1];
		t = l - t;
		// compare to minimum light
		if ((s*s+t*t+dist*dist) < maxdist)
		{
			if (surf->dlightframe != r_dlightframecount) // not dynamic until now
			{
				surf->dlightbits = bit;
				surf->dlightframe = r_dlightframecount;
			}
			else // already dynamic
				surf->dlightbits |= bit;
		}
	}

	if (node->children[0]->contents >= 0)
		R_MarkLights (light, bit, node->children[0]); // LordHavoc: original code
	if (node->children[1]->contents >= 0)
		R_MarkLights (light, bit, node->children[1]); // LordHavoc: original code
	
}


extern unsigned int shadowhack;
void R_MarkShadows (shadow_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	unsigned int			i;
	// LordHavoc: .lit support begin (actually this is just a major lighting speedup, no relation to color :)
	float		l, maxdist;
	int			s, t;
	vec3_t		impact;
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix

loc0:
	


	if (node->contents < 0)
		return;

	if(!shadowhack)
		return;



	splitplane = node->plane; // LordHavoc: original code
	VectorSubtract (light->origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix
	if (splitplane->type < 3)
		dist = origin_for_ent[splitplane->type] - splitplane->dist;
	else
		dist = DotProduct (origin_for_ent, splitplane->normal) - splitplane->dist; // mankrip - dynamic lights on moving brush models fix - edited
	

	if (dist > light->radius)
	{
		node = node->children[0];
		goto loc0;
		// LordHavoc: .lit support end
	}
	if (dist < -light->radius)
	{
		node = node->children[1];
		goto loc0;
	}

	maxdist = light->radius*light->radius;

	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		
		
		
			impact[0] = origin_for_ent[0] - surf->plane->normal[0]*dist; 
			impact[1] = origin_for_ent[1] - surf->plane->normal[1]*dist;
			impact[2] = origin_for_ent[2] - surf->plane->normal[2]*dist;
		

		// clamp center of light to corner and check brightness
		l = DotProduct (impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
		s = l+0.5;if (s < 0) s = 0;else if (s > surf->extents[0]) s = surf->extents[0];
		s = l - s;
		l = DotProduct (impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
		t = l+0.5;if (t < 0) t = 0;else if (t > surf->extents[1]) t = surf->extents[1];
		t = l - t;
		if ((s*s+t*t+dist*dist) < maxdist)
		{
			if (surf->shadowframe != r_shadowframecount) // not dynamic until now
			{
				surf->shadowbits = bit;
				surf->shadowframe = r_shadowframecount;
			}
			else // already dynamic
				surf->shadowbits |= bit;
		}
	
	}

	
	if (node->children[0]->contents >= 0)
		R_MarkShadows (light, bit, node->children[0]); // LordHavoc: original code
	if (node->children[1]->contents >= 0)
	R_MarkShadows (light, bit, node->children[1]); // LordHavoc: original code
	
}


void R_MarkWLights (wlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	unsigned int			i;
	// LordHavoc: .lit support begin (actually this is just a major lighting speedup, no relation to color :)
	float		l, maxdist;
	int			s, t;
	vec3_t		impact;
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix

loc0:
	


	if (node->contents < 0)
		return;




	splitplane = node->plane; // LordHavoc: original code
	VectorSubtract (light->origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix
	if (splitplane->type < 3)
		dist = origin_for_ent[splitplane->type] - splitplane->dist;
	else
		dist = DotProduct (origin_for_ent, splitplane->normal) - splitplane->dist; // mankrip - dynamic lights on moving brush models fix - edited
	

	if (dist > light->radius)
	{
		node = node->children[0];
		goto loc0;
		// LordHavoc: .lit support end
	}
	if (dist < -light->radius)
	{
		node = node->children[1];
		goto loc0;
	}

	maxdist = light->radius*light->radius;

	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		
		
		
			impact[0] = origin_for_ent[0] - surf->plane->normal[0]*dist; 
			impact[1] = origin_for_ent[1] - surf->plane->normal[1]*dist;
			impact[2] = origin_for_ent[2] - surf->plane->normal[2]*dist;
		

		// clamp center of light to corner and check brightness
		l = DotProduct (impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
		s = l+0.5;if (s < 0) s = 0;else if (s > surf->extents[0]) s = surf->extents[0];
		s = l - s;
		l = DotProduct (impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
		t = l+0.5;if (t < 0) t = 0;else if (t > surf->extents[1]) t = surf->extents[1];
		t = l - t;
		if ((s*s+t*t+dist*dist) < maxdist)
		{
			if (surf->wlightframe != r_wlightframecount) // not dynamic until now
			{
				surf->wlightbits = bit;
				surf->wlightframe = r_wlightframecount;
			}
			else // already dynamic
				surf->wlightbits |= bit;
		}
	
	}

	
	if (node->children[0]->contents >= 0)
		R_MarkWLights (light, bit, node->children[0]); // LordHavoc: original code
	if (node->children[1]->contents >= 0)
	R_MarkWLights (light, bit, node->children[1]); // LordHavoc: original code
	
}




/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int		i, h;
	dlight_t	*l;
	
	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	l = cl_dlights;
	
	currententity = &cl_entities[0]; // mankrip - dynamic lights on moving brush models fix

	for (i=0 ; i<MAX_DLIGHTS ; i++, l++)
	{
		if (l->die < cl.time || !l->radius)
			continue;
		R_MarkLights ( l, 1<<i, cl.worldmodel->nodes );
	}

}

void R_PushShadows (void)
{
	int		i, h;
	shadow_t	*s;
	r_shadowframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	s = cl_shadows;

	currententity = &cl_entities[0]; // mankrip - dynamic lights on moving brush models fix

	for (h=0 ; h<MAX_SHADOWS ; h++, s++)
	{
		if (s->die < cl.time || !s->radius)
			continue;
		R_MarkShadows ( s, 1<<h, cl.worldmodel->nodes );
	}
}

void R_PushWlights (void)
{
	int		i, h;
	wlight_t	*s;
	r_wlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	s = cl_wlights;

	currententity = &cl_entities[0]; 

	for (h=0 ; h<MAX_WLIGHTS ; h++, s++)
	{
		if (s->die < cl.time || !s->radius)
			continue;
		R_MarkWLights ( s, 1<<h, cl.worldmodel->nodes );
	
	}
}
/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

// old function reserved for old uncolored and classic lighting.
int RecursivedLightPoint (mnode_t *node, vec3_t start, vec3_t end)
{
	int			r;
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	byte		*lightmap;
	unsigned	scale;
	int			maps;

	if (node->contents < 0)
		return -1;		// didn't hit anything

// calculate mid point

// FIXME: optimize for axial
	plane = node->plane;
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;

	if ( (back < 0) == side)
		return RecursivedLightPoint (node->children[side], start, end);

	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

// go down front side
	r = RecursivedLightPoint (node->children[side], start, mid);
	if (r >= 0)
		return r;		// hit something

	if ( (back < 0) == side )
		return -1;		// didn't hit anuthing

// check for impact on this node

	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{

		if (surf->flags & SURF_DRAWTILED)
			continue;	// no lightmaps

		tex = surf->texinfo;

		s = DotProduct (mid, tex->vecs[0]) + tex->vecs[0][3];
		t = DotProduct (mid, tex->vecs[1]) + tex->vecs[1][3];;

		if (s < surf->texturemins[0] ||
		t < surf->texturemins[1])
			continue;

		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];

		if ( ds > surf->extents[0] || dt > surf->extents[1] )
			continue;

		if (!surf->samples)
			return 0;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;
		r = 0;
		if (lightmap)
		{

			lightmap += dt * ((surf->extents[0]>>4)+1) + ds;

			for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
					maps++)
			{
				scale = d_lightstylevalue[surf->styles[maps]];
				r += *lightmap * scale;
				lightmap += ((surf->extents[0]>>4)+1) *
						((surf->extents[1]>>4)+1);
			}

			r >>= 8;
		}

		return r;
	}

// go down back side
	return RecursivedLightPoint (node->children[!side], mid, end);
}

int R_LightPoints (vec3_t p)
{
	vec3_t		end;
	int			r;

	if (!cl.worldmodel->lightdata)
		return 255;

	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;
	r = RecursiveLightPoint (cl.worldmodel->nodes, p, end);

	if (r == -1)
		r = 0;

	if (r < r_refdef.ambientlight)
		r = r_refdef.ambientlight;

	return r;
}


mplane_t		*lightplane;
vec3_t			lightspot;

// LordHavoc: .lit support begin
// LordHavoc: original code replaced entirely
int RecursiveLightPoint (vec3_t color, mnode_t *node, vec3_t start, vec3_t end)
{
	float		front, back, frac;
	vec3_t		mid;

loc0:
	if (node->contents < 0)
		return false;		// didn't hit anything
	
// calculate mid point
	if (node->plane->type < 3)
	{
		front = start[node->plane->type] - node->plane->dist;
		back = end[node->plane->type] - node->plane->dist;
	}
	else
	{
		front = DotProduct(start, node->plane->normal) - node->plane->dist;
		back = DotProduct(end, node->plane->normal) - node->plane->dist;
	}

	// LordHavoc: optimized recursion
	if ((back < 0) == (front < 0))
//		return RecursiveLightPoint (color, node->children[front < 0], start, end);
	{
		node = node->children[front < 0];
		goto loc0;
	}
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;
	
// go down front side
	if (RecursiveLightPoint (color, node->children[front < 0], start, mid))
		return true;	// hit something
	else
	{
		int i, ds, dt;
		msurface_t *surf;
	// check for impact on this node
		VectorCopy (mid, lightspot);
		lightplane = node->plane;

		surf = cl.worldmodel->surfaces + node->firstsurface;
		for (i = 0;i < node->numsurfaces;i++, surf++)
		{
			if (surf->flags & SURF_DRAWTILED)
				continue;	// no lightmaps

			ds = (int) ((float) DotProduct (mid, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3]);
			dt = (int) ((float) DotProduct (mid, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3]);

			if (ds < surf->texturemins[0] || dt < surf->texturemins[1])
				continue;
			
			ds -= surf->texturemins[0];
			dt -= surf->texturemins[1];
			
			if (ds > surf->extents[0] || dt > surf->extents[1])
				continue;

			if (surf->samples)
			{
				// LordHavoc: enhanced to interpolate lighting
				byte *lightmap;
				int maps, line3, dsfrac = ds & 15, dtfrac = dt & 15, r00 = 0, g00 = 0, b00 = 0, r01 = 0, g01 = 0, b01 = 0, r10 = 0, g10 = 0, b10 = 0, r11 = 0, g11 = 0, b11 = 0;
				float scale;
				line3 = ((surf->extents[0]>>4)+1)*3;

				lightmap = surf->samples + ((dt>>4) * ((surf->extents[0]>>4)+1) + (ds>>4))*3; // LordHavoc: *3 for color

				for (maps = 0;maps < MAXLIGHTMAPS && surf->styles[maps] != 255;maps++)
				{
					scale = (float) d_lightstylevalue[surf->styles[maps]] * 1.0 / 256.0;
					r00 += (float) lightmap[      0] * scale;g00 += (float) lightmap[      1] * scale;b00 += (float) lightmap[2] * scale;
					r01 += (float) lightmap[      3] * scale;g01 += (float) lightmap[      4] * scale;b01 += (float) lightmap[5] * scale;
					r10 += (float) lightmap[line3+0] * scale;g10 += (float) lightmap[line3+1] * scale;b10 += (float) lightmap[line3+2] * scale;
					r11 += (float) lightmap[line3+3] * scale;g11 += (float) lightmap[line3+4] * scale;b11 += (float) lightmap[line3+5] * scale;
					lightmap += ((surf->extents[0]>>4)+1) * ((surf->extents[1]>>4)+1)*3; // LordHavoc: *3 for colored lighting
				}

				color[0] += (float) ((int) ((((((((r11-r10) * dsfrac) >> 4) + r10)-((((r01-r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01-r00) * dsfrac) >> 4) + r00)));
				color[1] += (float) ((int) ((((((((g11-g10) * dsfrac) >> 4) + g10)-((((g01-g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01-g00) * dsfrac) >> 4) + g00)));
				color[2] += (float) ((int) ((((((((b11-b10) * dsfrac) >> 4) + b10)-((((b01-b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01-b00) * dsfrac) >> 4) + b00)));
			}
			return true; // success
		}

	// go down back side
		return RecursiveLightPoint (color, node->children[front >= 0], mid, end);
	}
}

vec3_t lightcolor; // LordHavoc: used by model rendering
int R_LightPoint (vec3_t p)
{
	vec3_t		end;
	int	r;
	if (coloredlights)
	{
		
	if (r_fullbright->value || !cl.worldmodel->lightdata)
	{
		lightcolor[0] = lightcolor[1] = lightcolor[2] = 255;
		return 255;
	}
	
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;

	lightcolor[0] = lightcolor[1] = lightcolor[2] = 0;
	RecursiveLightPoint (lightcolor, cl.worldmodel->nodes, p, end);
	
	return ((lightcolor[0] + lightcolor[1] + lightcolor[2]) * (1.0f / 3.0f));


	}
	else
	{
if (!cl.worldmodel->lightdata)
		return 255;
	
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;
	
	r = RecursivedLightPoint (cl.worldmodel->nodes, p, end);
	
	if (r == -1)
		r = 0;

	if (r < r_refdef.ambientlight)
		r = r_refdef.ambientlight;

	return r;


	}




}
