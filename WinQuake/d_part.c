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
// d_part.c: software driver module for drawing particles

#include "quakedef.h"
#include "d_local.h"


/*
==============
D_EndParticles
==============
*/
void D_EndParticles (void)
{
// not used by software driver
}


/*
==============
D_StartParticles
==============
*/
void D_StartParticles (void)
{
	
// not used by software driver
}


#if	!id386

/*
==============
D_DrawParticle
==============
*/
void D_DrawParticle (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v;

// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) ||
		(u > d_vrectright_particle) ||
		(v < d_vrecty) ||
		(u < d_vrectx))
	{
		return;
	}



	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;

	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = pparticle->color;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = pparticle->color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = pparticle->color;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = pparticle->color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = pparticle->color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = pparticle->color;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = pparticle->color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = pparticle->color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = pparticle->color;
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = pparticle->color;
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = pparticle->color;
				}
			}
		}
		break;
	}
}

#endif	// !id386
extern cvar_t *temp2;

// leilei - stupid experimental hack to draw our flare as a particle to check for visibility.
void D_TestOurFlare (flare_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v;

// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) ||
		(u > d_vrectright_particle) ||
		(v < d_vrecty) ||
		(u < d_vrectx))
	{
		return;
	}



	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;


	switch (pix)
	{

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			
		
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pparticle->amiseen = 1;
				}
				else
					pparticle->amiseen = 0;
			}
		}
		break;
	}
	
}


void D_DrawParticle_Fog (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v;
	
// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) ||
		(u > d_vrectright_particle) ||
		(v < d_vrecty) ||
		(u < d_vrectx))
	{
		return;
	}



	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;


	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = pparticle->color;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = pparticle->color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = pparticle->color;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = pparticle->color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = pparticle->color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = pparticle->color;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = pparticle->color;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = pparticle->color;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = pparticle->color;
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = pparticle->color;
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = pparticle->color;
				}
			}
		}
		break;
	}
}

// LEILEI - lame particle blending
extern pixel_t transTable[256][256];
extern pixel_t addTable[256][256];	 

/*
==============
D_DrawParticleLit

  useful for not-glow-in-the-dark stuff, like dirt, and guts
==============
*/
void D_DrawParticleLit (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, col;
	float alf;
	
// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) ||
		(u > d_vrectright_particle) ||
		(v < d_vrecty) ||
		(u < d_vrectx))
	{
		return;
	}

		col = (int)pparticle->color;
	
	//alf = R_LightPoint(pparticle->org);
	//alf = R_LightPoint(pparticle->org);
//	alf = 
//		8192 *
//		(R_LightPoint(pparticle->org) * -1) + 16384;
	alf = (R_LightPoint(pparticle->org) / 272);

//8192 * (pparticle->alpha * -1) + 16384;
	Con_Printf ("%f paticl\n", alf);

	
			
	col = ((byte *)acolormap)[col + ((int)alf & 0xFF00)];


	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;





	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = col;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = col;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = col;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = col;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = col;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = col;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = col;
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = col;
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = col;
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = col;
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = col;
				}
			}
		}
		break;
	}
}
/*
==============
D_DrawParticleA33
==============
*/
void D_DrawParticle_A33 (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, col;
	int	 alf;
// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) ||
		(u > d_vrectright_particle) ||
		(v < d_vrecty) ||
		(u < d_vrectx))
	{
		return;
	}

	col = (int)pparticle->color;

	
	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;


	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = transTable[col][pdest[0]];
				
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = transTable[col][pdest[0]];
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = transTable[col][pdest[1]];
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = transTable[col][pdest[0]];
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = transTable[col][pdest[1]];
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = transTable[col][pdest[2]];
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] =transTable[col][pdest[0]];
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = transTable[col][pdest[1]];
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] =transTable[col][pdest[2]];
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = transTable[col][pdest[3]];
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = transTable[col][pdest[i]];
				}
			}
		}
		break;
	}
}

void D_DrawParticle_A66 (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, col;
	
// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) ||
		(u > d_vrectright_particle) ||
		(v < d_vrecty) ||
		(u < d_vrectx))
	{
		return;
	}

	col = (int)pparticle->color;

	
	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;
	
	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = transTable[pdest[0]][col];
				
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = transTable[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = transTable[pdest[1]][col];
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = transTable[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = transTable[pdest[1]][col];
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = transTable[pdest[2]][col];
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] =transTable[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = transTable[pdest[1]][col];
			}

			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] =transTable[pdest[2]][col];
			}

			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = transTable[pdest[3]][col];
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = transTable[pdest[i]][col];
				}
			}
		}
		break;
	}
}

extern cvar_t *r_shift1;


void D_DrawParticle_Add (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, col, alf;
	
// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) ||
		(u > d_vrectright_particle) ||
		(v < d_vrecty) ||
		(u < d_vrectx))
	{
		return;
	}

	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;


	col = (int)pparticle->color;


	// Attempt to fade the additive out according to alpha
	// by colormap translation (!)
	// TODO: use 'glsuck' colormap once that works

	if(pparticle->alpha < 1)
	{
		alf = 8192 * (pparticle->alpha * -1) + 16384;
		col = ((byte *)acolormap)[col + (alf & 0xFF00)];
	}


	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
			
				pdest[0] = addTable[pdest[0]][col];
				
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
			
				pdest[0] = addTable[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
			
				pdest[1] = addTable[pdest[1]][col];
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
			
				pdest[0] = addTable[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
		
				pdest[1] = addTable[pdest[1]][col];
			}

			if (pz[2] <= izi)
			{
			
				pdest[2] = addTable[pdest[2]][col];
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
			
				pdest[0] =addTable[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
		
				pdest[1] = addTable[pdest[1]][col];
			}

			if (pz[2] <= izi)
			{
		
				pdest[2] =addTable[pdest[2]][col];
			}

			if (pz[3] <= izi)
			{
		
				pdest[3] = addTable[pdest[3]][col];
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
			
					pdest[i] = addTable[pdest[i]][col];
				}
			}
		}
		break;
	}
}

extern byte 		menumap[256][16];			// haha hack
void D_DrawParticle_Gel (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v, col, alf;
	int		gmcol;	

// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) ||
		(u > d_vrectright_particle) ||
		(v < d_vrecty) ||
		(u < d_vrectx))
	{
		return;
	}
			gmcol = pparticle->color; // sample a pixel (TODO: Average a pixel on load)
							if (gmcol < 16) gmcol = 0;
							else if (gmcol < 32) gmcol = 1;
							else if (gmcol < 48) gmcol = 2;
							else if (gmcol < 64) gmcol = 3;
							else if (gmcol < 80) gmcol = 4;
							else if (gmcol < 96) gmcol = 5;
							else if (gmcol < 112) gmcol = 6;
							else if (gmcol < 128) gmcol = 7;
							else if (gmcol < 144) gmcol = 8;
							else if (gmcol < 160) gmcol = 9;
							else if (gmcol < 176) gmcol = 10;
							else if (gmcol < 192) gmcol = 11;
							else if (gmcol < 208) gmcol = 12;
							else if (gmcol < 224) gmcol = 13;  // stupid elses huh?
							else if (gmcol < 240) gmcol = 14;
							else   gmcol = 2; // stupid.
	pparticle->color = gmcol;

	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;

	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;


	col = (int)pparticle->color;

	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
			
				pdest[0] = menumap[pdest[0]][col];
				
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
			
				pdest[0] = menumap[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
			
				pdest[1] = menumap[pdest[1]][col];
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
			
				pdest[0] = menumap[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
		
				pdest[1] = menumap[pdest[1]][col];
			}

			if (pz[2] <= izi)
			{
			
				pdest[2] = menumap[pdest[2]][col];
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
			
				pdest[0] = menumap[pdest[0]][col];
			}

			if (pz[1] <= izi)
			{
		
				pdest[1] = menumap[pdest[1]][col];
			}

			if (pz[2] <= izi)
			{
		
				pdest[2] = menumap[pdest[2]][col];
			}

			if (pz[3] <= izi)
			{
		
				pdest[3] = menumap[pdest[3]][col];
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<pix ; i++)
			{
				if (pz[i] <= izi)
				{
			
					pdest[i] = menumap[pdest[i]][col];
				}
			}
		}
		break;
	}
}

// fteqw says hi

void D_2dPos(vec3_t pos, int *u, int *v, int *z)
{
	float	zi;
	vec3_t	local, transformed;

	// transform point
	VectorSubtract (pos, r_origin, local);

	transformed[2] = DotProduct(local, r_ppn);		

	if (transformed[2] < PARTICLE_Z_CLIP)	//near clip
	{
		*u = -1;	//send it off the side intentionally.
		return;
	}

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);

// project the point
	zi = 1.0 / transformed[2];
	*u = (int)(xcenter + zi * transformed[0] + 0.5);
	*v = (int)(ycenter - zi * transformed[1] + 0.5);

	*z = (int)(zi * 0x8000);
}
vec_t VI2Length(int x, int y)
{
	float	length;
	length = (float)x*x + (float)y*y;	
	length = sqrt (length);
	return length;
}


void D_DrawSparkTrans (vec3_t src, vec3_t dest, unsigned int pcolour, int blend)	//draw a line in 3d space, 8bpp
{
	byte	*pdest;
	short	*pz;
	int		count, u1, v1, z1;
	float	alfa;
	int u2, v2, z2;
	int	coll, forg;
	int du, dv, dz;

//	coll = pcolour;
//	if (pcolour<0)
		
	//	return; // hmm, corruption!
//	if (pcolour>255)
		
//		return; // hmm, corruption!
//	alfa = palpha; // tODO : Alpha fade along length
	D_2dPos(src, &u1, &v1, &z1);
	D_2dPos(dest, &u2, &v2, &z2);

	if ((v1 > d_vrectbottom_particle) || 
		(u1 > d_vrectright_particle) ||
		(v1 < d_vrecty) ||
		(u1 < d_vrectx))
	{
		return;
	}

	if ((v2 > d_vrectbottom_particle) || 
		(u2 > d_vrectright_particle) ||
		(v2 < d_vrecty) ||
		(u2 < d_vrectx))
	{
		return;
	}	

	du = u2 - u1;
	dv = v2 - v1;
	dz = z2 - z1;

	if (!du && !dv)
		count = 1;
	else
	{
		count = VI2Length(du, dv);
		if (!count)
			count = 1;
	}

	du *= 256*256;
	dv *= 256*256;
	dz *= 256*256;
	u1 = u1<<16;
	v1 = v1<<16;
	z1 = z1<<16;
	du /= count;
	dv /= count;
	dz /= count;

	if (blend == 1) // additive
	{
		do
		{		
			pz = d_pzbuffer + (d_zwidth * (v1>>16)) + (u1>>16);

			if (*pz <= z1>>16)
			{
	//				*pz = z1>>16;
				pdest = d_viewbuffer + d_scantable[v1>>16] + (u1>>16);
				*pdest = pcolour;
				//*pdest = addTable[(byte)pdest][pcolour];
			}

			u1 += du;
			v1 += dv;
			z1 += dz;
		} while (count--);
	}
/*	else if (alfa < 0.66f)// alpha
	{
		do
		{		
			pz = d_pzbuffer + (d_zwidth * (v1>>16)) + (u1>>16);

			if (*pz <= z1>>16)
			{
	//				*pz = z1>>16;
				pdest = d_viewbuffer + d_scantable[v1>>16] + (u1>>16);
				//pdest = transTable[coll][*pdest];
								*pdest = coll;
			}

			u1 += du;
			v1 += dv;
			z1 += dz;
		} while (count--);
	}
	else if (alfa < 0.33f)// alpha
	{
		do
		{		
			pz = d_pzbuffer + (d_zwidth * (v1>>16)) + (u1>>16);

			if (*pz <= z1>>16)
			{
	//				*pz = z1>>16;
				pdest = d_viewbuffer + d_scantable[v1>>16] + (u1>>16);
				//pdest = transTable[*pdest][coll];
				*pdest = coll;
			}

			u1 += du;
			v1 += dv;
			z1 += dz;
		} while (count--);
	}
*/
	else // merge blend
	{
		
		do
		{		
			pz = d_pzbuffer + (d_zwidth * (v1>>16)) + (u1>>16);
			if (foguse){ forg = (short)pz /2048;	if (forg > 32762)	forg = 32762;
			
					pcolour = host_fogmap[pcolour  + (forg >> fogrange & 0xFF00)];
			} // leilei - fog
			if (*pz <= z1>>16)
			{
	//				*pz = z1>>16;
				pdest = d_viewbuffer + d_scantable[v1>>16] + (u1>>16);
				*pdest = pcolour;
			}

			u1 += du;
			v1 += dv;
			z1 += dz;
		} while (count--);
	}
}


