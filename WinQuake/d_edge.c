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
// d_edge.c

#include "quakedef.h"
#include "d_local.h"

static int	miplevel;


float		scale_for_mip;
int			screenwidth;
int			ubasestep, errorterm, erroradjustup, erroradjustdown;
int			vstartscan;
extern		int gonnareflect;
// FIXME: should go away
extern void			R_RotateBmodel (void);
extern void			R_TransformFrustum (void);
extern	float	oldwateralpha;

vec3_t		transformed_modelorg;

/*
==============
D_DrawPoly

==============
*/
void D_DrawPoly (void)
{
// this driver takes spans, not polygons

	// BULL SHIT!!!



}


/*
=============
D_MipLevelForScale
=============
*/
int D_MipLevelForScale (float scale)
{
	int		lmiplevel;

	if (scale >= d_scalemip[0] )
		lmiplevel = 0;
	else if (scale >= d_scalemip[1] )
		lmiplevel = 1;
	else if (scale >= d_scalemip[2] )
		lmiplevel = 2;
	else
		lmiplevel = 3;

	if (lmiplevel < d_minmip)
		lmiplevel = d_minmip;

	return lmiplevel;
}


/*
==============
D_DrawSolidSurface
==============
*/

// FIXME: clean this up

void D_DrawSolidSurface (surf_t *surf, int color)
{
	espan_t	*span;
	byte	*pdest;
	int		u, u2, pix;

	if (r_pixbytes == 4)
	{
		unsigned int	*p32dest;
		pix = d_8to24table[color];
		for (span=surf->spans ; span ; span=span->pnext)
		{
			p32dest = (unsigned int *)d_viewbuffer + screenwidth*span->v;
			u = span->u;
			u2 = span->u + span->count - 1;
			p32dest[u] = pix;

			for ( ; u <= u2 ; u++)
				p32dest[u] = pix;
		}
	}
	else if (r_pixbytes == 2)
	{
		unsigned short	*p16dest;
		pix = vid.colormap16[color];
		for (span=surf->spans ; span ; span=span->pnext)
		{
			p16dest = (unsigned short *)d_viewbuffer + screenwidth*span->v;
			u = span->u;
			u2 = span->u + span->count - 1;
			p16dest[u] = pix;

			for ( ; u <= u2 ; u++)
				p16dest[u] = pix;
		}
	}
	else
	{
	pix = (color<<24) | (color<<16) | (color<<8) | color;
	for (span=surf->spans ; span ; span=span->pnext)
	{
		pdest = (byte *)d_viewbuffer + screenwidth*span->v;
		u = span->u;
		u2 = span->u + span->count - 1;
		((byte *)pdest)[u] = pix;

		if (u2 - u < 8)
		{
			for (u++ ; u <= u2 ; u++)
				((byte *)pdest)[u] = pix;
					
		}
		else
		{
			for (u++ ; u & 3 ; u++)
				((byte *)pdest)[u] = pix;
			
			u2 -= 4;
			for ( ; u <= u2 ; u+=4)
				*(int *)((byte *)pdest + u) = pix;
			u2 += 4;
			for ( ; u <= u2 ; u++)
				((byte *)pdest)[u] = pix;
		}
	}
	}
}

/*
void D_GoThroughThisSurface (surf_t *surf)
{
	espan_t	*span;
	
	int		u, u2, pix;

	{
	for (span=surf->spans ; span ; span=span->pnext)
	{
		pdest = (byte *)d_viewbuffer + screenwidth*span->v;
		u = span->u;
		u2 = span->u + span->count - 1;
		((byte *)pdest)[u] = pix;

		if (u2 - u < 8)
		{
			for (u++ ; u <= u2 ; u++)
				((byte *)pdest)[u] = pix;
		}
		else
		{
			for (u++ ; u & 3 ; u++)
				((byte *)pdest)[u] = pix;

			u2 -= 4;
			for ( ; u <= u2 ; u+=4)
				*(int *)((byte *)pdest + u) = pix;
			u2 += 4;
			for ( ; u <= u2 ; u++)
				((byte *)pdest)[u] = pix;
		}
	}
	}
}

*/
// leilei - just really, take a pixel buffer into it.
void D_DrawSolidMirrorSurface (surf_t *surf, int color)
{
	espan_t	*span;
	byte	*pdest;
	byte	*pbest;
	int		u, u2, pix;

#ifdef WATERREFLECTIONS
	{
	pix = 4;
	for (span=surf->spans ; span ; span=span->pnext)
	{
		pdest = (byte *)d_viewbuffer + screenwidth*span->v;
		pbest = (byte *)d_reflectbuffer + screenwidth*span->v;
		u = span->u;
		u2 = span->u + span->count - 1;
		((byte *)pdest)[u] = pbest[u];

		if (u2 - u < 8)
		{
			for (u++ ; u <= u2 ; u++)
				((byte *)pdest)[u] = pbest[u];
		}
		else
		{
			for (u++ ; u & 3 ; u++)
				((byte *)pdest)[u] = pbest[u];

			u2 -= 4;
			for ( ; u <= u2 ; u+=4)
				*(int *)((byte *)pdest + u) = pbest[u];
			u2 += 4;
			for ( ; u <= u2 ; u++)
				((byte *)pdest)[u] = pbest[u];
		}
	}
	}
#endif
}



/*
==============
D_CalcGradients
==============
*/
void D_CalcGradients (msurface_t *pface)
{
//	mplane_t	*pplane;	// 2001-12-10 Reduced compiler warnings by Jeff Ford
	float		mipscale;
	vec3_t		p_temp1;
	vec3_t		p_saxis, p_taxis;
	float		t;

//	pplane = pface->plane;	// 2001-12-10 Reduced compiler warnings by Jeff Ford

	mipscale = 1.0 / (float)(1 << miplevel);
	//mipscale = 12; // Low detail hack?!
	TransformVector (pface->texinfo->vecs[0], p_saxis);
	TransformVector (pface->texinfo->vecs[1], p_taxis);

	t = xscaleinv * mipscale;
	//t = 1024;
	d_sdivzstepu = p_saxis[0] * t;
	d_tdivzstepu = p_taxis[0] * t;

	t = yscaleinv * mipscale;
	//t = 1024;
	d_sdivzstepv = -p_saxis[1] * t;
	d_tdivzstepv = -p_taxis[1] * t;

	d_sdivzorigin = p_saxis[2] * mipscale - xcenter * d_sdivzstepu -
			ycenter * d_sdivzstepv;
	d_tdivzorigin = p_taxis[2] * mipscale - xcenter * d_tdivzstepu -
			ycenter * d_tdivzstepv;

	VectorScale (transformed_modelorg, mipscale, p_temp1);

	t = 0x10000*mipscale;
	
	sadjust = ((fixed16_t)(DotProduct (p_temp1, p_saxis) * 0x10000 + 0.5)) -
			((pface->texturemins[0] << 16) >> miplevel)
			+ pface->texinfo->vecs[0][3]*t;

  tadjust = ((fixed16_t)(DotProduct (p_temp1, p_taxis) * 0x10000 + 0.5)) -
			((pface->texturemins[1] << 16) >> miplevel)
			+ pface->texinfo->vecs[1][3]*t;

//
// -1 (-epsilon) so we never wander off the edge of the texture
//
	bbextents = ((pface->extents[0] << 16) >> miplevel) - 1;
	bbextentt = ((pface->extents[1] << 16) >> miplevel) - 1;

}
extern cvar_t temp1;

/*
==============
D_DrawSurfaces
==============
*/
extern int	waterinsight;
extern vec3_t			sbaseaxis[3], tbaseaxis[3];
extern vec3_t		reflectorg;	
extern cvar_t	*d_mipdetail;
extern cvar_t	*r_waterquality;
void D_DrawSurfaces (void)
{
	surf_t			*s;
	msurface_t		*pface;
	surfcache_t		*pcurrentcache;
	vec3_t			world_transformed_modelorg;
	vec3_t			local_modelorg;

	currententity = &cl_entities[0];
	TransformVector (modelorg, transformed_modelorg);
	VectorCopy (transformed_modelorg, world_transformed_modelorg);

// TODO: could preset a lot of this at mode set time
	if (r_drawflat->value)
	{
		for (s = &surfaces[1] ; s<surface_p ; s++)
		{
			if (!s->spans)
				continue;

			d_zistepu = s->d_zistepu;
			d_zistepv = s->d_zistepv;
			d_ziorigin = s->d_ziorigin;

			D_DrawSolidSurface (s, (int)s->data & 0xFF);
			D_DrawZSpans (s->spans);
	
		}
	}
	else
	{
		for (s = &surfaces[1] ; s<surface_p ; s++)
		{
			if (!s->spans)
				continue;

			r_drawnpolycount++;
			// Manoel Kasimier - translucent water - begin
			
			if (r_drawwater && (!(s->flags & SURF_DRAWTRANSLUCENT)))
				continue;
			// Manoel Kasimier - translucent water - end
			
			d_zistepu = s->d_zistepu;
			d_zistepv = s->d_zistepv;
			d_ziorigin = s->d_ziorigin;
			waterinsight = 0;
			if (s->flags & SURF_DRAWSKY)
			{
				
				if (!r_skymade)
				{
					R_MakeSky ();
				}

				D_DrawSkyScans8 (s->spans);
				D_DrawZSpans (s->spans);
			}
			else if (s->flags & SURF_DRAWBACKGROUND)
			{
		
			// set up a gradient for the background surface that places it
			// effectively at infinity distance from the viewpoint
				d_zistepu = 0;
				d_zistepv = 0;
				d_ziorigin = -0.9;

				D_DrawSolidSurface (s, (int)r_clearcolor->value & 0xFF);
				D_DrawZSpans (s->spans);
			}
			/*
			else if (s->flags & SURF_MIRROR)
			{
		
			// it is a mirror.
				d_zistepu = 0;
				d_zistepv = 0;
			//	d_ziorigin = -0.9;

				D_DrawSolidMirrorSurface (s, (int)r_clearcolor->value & 55);
				D_DrawZSpans (s->spans);
			}
			*/
			else if (s->flags & SURF_DRAWTURB)
			{
//				if(reflectpass)
//						return;	// don't reflect our reflect
				pface = s->data;
			//	if (reflectpass)
				

				miplevel = 0;
				cacheblock = (pixel_t *)
						((byte *)pface->texinfo->texture +
						pface->texinfo->texture->offsets[0]);
				cachewidth = 64;

				if (s->insubmodel)
				{
				// FIXME: we don't want to do all this for every polygon!
				// TODO: store once at start of frame
					currententity = s->entity;	//FIXME: make this passed in to
												// R_RotateBmodel ()
					VectorSubtract (r_origin, currententity->origin,
							local_modelorg);
					TransformVector (local_modelorg, transformed_modelorg);

					R_RotateBmodel ();	// FIXME: don't mess with the frustum,
										// make entity passed in
				}
				
				D_CalcGradients (pface);
				if (pface->plane->normal[2] != 1)
					s->flags = s->flags ^ SURF_MIRROR; // leilei - revoke reflection privs from water that isnt up
#ifdef WATERREFLECTIONS
				if (s->flags & SURF_MIRROR && r_waterquality->value > 1)	{		// leilei - water reflections
				mirror_plane = pface->plane;
				waterinsight = 1;	// leilei - only allow waters that face up to reflect
				gonnareflect = 1;
				Turbulent8Reflect (s->spans);
				}
				else
#endif
				{

				waterinsight = 0;
				Turbulent8 (s->spans);
				}
	
				
			if (!r_drawwater) // Manoel Kasimier - translucent water
				D_DrawZSpans (s->spans);

				if (s->insubmodel)
				{
				//
				// restore the old drawing state
				// FIXME: we don't want to do this every time!
				// TODO: speed up
				//
					currententity = &cl_entities[0];
					VectorCopy (world_transformed_modelorg,
								transformed_modelorg);
					VectorCopy (base_vpn, vpn);
					VectorCopy (base_vup, vup);
					VectorCopy (base_vright, vright);
					VectorCopy (base_modelorg, modelorg);
					R_TransformFrustum ();
				}
			}
			else
			{
				if (s->insubmodel)
				{
				// FIXME: we don't want to do all this for every polygon!
				// TODO: store once at start of frame
					currententity = s->entity;	//FIXME: make this passed in to
												// R_RotateBmodel ()
					VectorSubtract (r_origin, currententity->origin, local_modelorg);
					TransformVector (local_modelorg, transformed_modelorg);

					R_RotateBmodel ();	// FIXME: don't mess with the frustum,
										// make entity passed in
				}

			
				pface = s->data;
			if (pface->flags & SURF_FRACTAL)
				{
					miplevel = 0;
					if (pface->cachespots[miplevel])
						pface->cachespots[miplevel]->texture = NULL;
				}
			else if (pface->flags & SURF_DONTMIP64 && d_mipdetail->value == 64)
				{
					miplevel = 0;
					if (pface->cachespots[miplevel])
						pface->cachespots[miplevel]->texture = NULL;
				}
			else{
				miplevel = D_MipLevelForScale (s->nearzi * scale_for_mip
				* pface->texinfo->mipadjust);
				
			}

			// FIXME: make this passed in to D_CacheSurface





				pcurrentcache = D_CacheSurface (pface, miplevel);

				cacheblock = (pixel_t *)pcurrentcache->data;
				cachewidth = pcurrentcache->width;

				cacheheight = pcurrentcache->height;
				D_CalcGradients (pface);
				mirror_plane = pface->plane;

		
#ifdef WATERREFLECTIONS
			if (s->flags & SURF_MIRROR)	// we draw some special mirroring spans here......
			{
				mirror_plane = pface->plane;
				waterinsight = 1; // i'm not water!

				D_DrawSpans8_Mirror_C_Filter (s->spans);

	
	
				//Con_Printf("%f %f %f Mirror\n", mirror_plane->normal[0],mirror_plane->normal[1],mirror_plane->normal[2]);
			}
			else
#endif
				//D_DrawSpans8_C_FilterAlter (s->spans);
				(*d_drawspans) (s->spans);
			
				D_DrawZSpans (s->spans);

			//	D_DrawSpans8_C_Fogger (s->spans);
	
				if (s->insubmodel)
				{
				//
				// restore the old drawing state
				// FIXME: we don't want to do this every time!
				// TODO: speed up
				//
					currententity = &cl_entities[0];
					VectorCopy (world_transformed_modelorg,
								transformed_modelorg);
					VectorCopy (base_vpn, vpn);
					VectorCopy (base_vup, vup);
					VectorCopy (base_vright, vright);
					VectorCopy (base_modelorg, modelorg);
					R_TransformFrustum ();
				}
	
			}
		}
	}
}

