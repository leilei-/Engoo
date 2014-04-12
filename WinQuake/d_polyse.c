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
// d_polyset.c: routines for drawing sets of polygons sharing the same
// texture (used for Alias models)

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

#define COLMODEL

// TODO: put in span spilling to shrink list size
// !!! if this is changed, it must be changed in d_polysa.s too !!!
#define DPS_MAXSPANS			MAXHEIGHT+1
									// 1 extra for spanpackage that marks end
extern int	coloredlights;
extern cvar_t *temp1;
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct {
	void			*pdest;
	short			*pz;
	int				count;
	byte			*ptex;
	int				sfrac, tfrac, light, zi;

	// NEW STRUCTS
	int				lightr;
	int				lightg;
	int				lightb;

	//int				s, t, u, v;
} spanpackage_t;

typedef struct {
	int		isflattop;
	int		numleftedges;
	int		*pleftedgevert0;
	int		*pleftedgevert1;
	int		*pleftedgevert2;
	int		numrightedges;
	int		*prightedgevert0;
	int		*prightedgevert1;
	int		*prightedgevert2;
} edgetable;

int	r_p0[9], r_p1[9], r_p2[9];

byte		*d_pcolormap;
extern cvar_t *r_filter;
int			d_aflatcolor;
int			d_xdenom;
extern int kernel[2][2][2];
edgetable	*pedgetable;

edgetable	edgetables[12] = {
	{0, 1, r_p0, r_p2, NULL, 2, r_p0, r_p1, r_p2 },
	{0, 2, r_p1, r_p0, r_p2,   1, r_p1, r_p2, NULL},
	{1, 1, r_p0, r_p2, NULL, 1, r_p1, r_p2, NULL},
	{0, 1, r_p1, r_p0, NULL, 2, r_p1, r_p2, r_p0 },
	{0, 2, r_p0, r_p2, r_p1,   1, r_p0, r_p1, NULL},
	{0, 1, r_p2, r_p1, NULL, 1, r_p2, r_p0, NULL},
	{0, 1, r_p2, r_p1, NULL, 2, r_p2, r_p0, r_p1 },
	{0, 2, r_p2, r_p1, r_p0,   1, r_p2, r_p0, NULL},
	{0, 1, r_p1, r_p0, NULL, 1, r_p1, r_p2, NULL},
	{1, 1, r_p2, r_p1, NULL, 1, r_p0, r_p1, NULL},
	{1, 1, r_p1, r_p0, NULL, 1, r_p2, r_p0, NULL},
	{0, 1, r_p0, r_p2, NULL, 1, r_p0, r_p1, NULL},
};

// FIXME: some of these can become statics
int				a_sstepxfrac, a_tstepxfrac, r_lstepx, a_ststepxwhole;
int				r_sstepx, r_tstepx, r_lstepy, r_sstepy, r_tstepy;
int				r_zistepx, r_zistepy;
int				r_lrstepx, r_lrstepy;
int				r_lgstepx, r_lgstepy;
int				r_lbstepx, r_lbstepy;
int				d_aspancount, d_countextrastep;

spanpackage_t			*a_spans;
spanpackage_t			*d_pedgespanpackage;
static int				ystart;
byte					*d_pdest, *d_ptex;
short					*d_pz;
int						d_sfrac, d_tfrac, d_light, d_zi;
int						d_lightrgb[3];
int						d_ptexextrastep, d_sfracextrastep;
int						d_tfracextrastep, d_lightextrastep, d_pdestextrastep;
int						d_lightbasestep, d_pdestbasestep, d_ptexbasestep;
int						d_lightbasestepr, d_lightbasestepg, d_lightbasestepb;
int						d_lightextrastepr, d_lightextrastepg, d_lightextrastepb;
int						d_sfracbasestep, d_tfracbasestep;
int						d_ziextrastep, d_zibasestep;
int						d_pzextrastep, d_pzbasestep;

typedef struct {
	int		quotient;
	int		remainder;
} adivtab_t;

static adivtab_t	adivtab[32*32] = {
#include "adivtab.h"
};

byte	*skintable[MAX_LBM_HEIGHT];
int		skinwidth;
byte	*skinstart;

void D_PolysetDrawSpans8 (spanpackage_t *pspanpackage);
void D_PolysetDrawSpans8_C (spanpackage_t *pspanpackage);
void D_PolysetCalcGradients (int skinwidth);
void D_DrawSubdiv (void);
void D_DrawNonSubdiv (void);
void D_PolysetRecursiveTriangle (int *p1, int *p2, int *p3);
void D_PolysetSetEdgeTable (void);
void D_RasterizeAliasPolySmooth (void);
void D_RasterizeAliasPolySmoothFilter (void);
void D_PolysetScanLeftEdge (int height);

#if	!id386broken

/*
================
D_PolysetDraw
================
*/
void D_PolysetDraw (void)
{
	spanpackage_t	spans[DPS_MAXSPANS + 1 +
			((CACHE_SIZE - 1) / sizeof(spanpackage_t)) + 1];
						// one extra because of cache line pretouching

	a_spans = (spanpackage_t *)
			(((long)&spans[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));

	
	if (r_affinetridesc.drawtype)
	{
		D_DrawSubdiv ();
	}
	else
	{
		D_DrawNonSubdiv ();
	}
	
}


/*
================
D_PolysetDrawFinalVerts
================
*/
void D_PolysetDrawFinalVerts (finalvert_t *fv, int numverts)
{
	int		i, z;
	short	*zbuf;

	for (i=0 ; i<numverts ; i++, fv++)
	{
	// valid triangle coordinates for filling can include the bottom and
	// right clip edges, due to the fill rule; these shouldn't be drawn
		if ((fv->v[0] < r_refdef.vrectright) &&
			(fv->v[1] < r_refdef.vrectbottom))
		{
			z = fv->v[5]>>16;
			zbuf = zspantable[fv->v[1]] + fv->v[0];
			if (z >= *zbuf)
			{
				int		pix;

				*zbuf = z;
				pix = skintable[fv->v[3]>>16][fv->v[2]>>16];
				pix = ((byte *)acolormap)[pix + (fv->v[4] & 0xFF00) ];			//  
				d_viewbuffer[d_scantable[fv->v[1]] + fv->v[0]] = pix;

			}
		}
	}
}


/*
================
D_DrawSubdiv
================
*/
void D_DrawSubdiv (void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	int				i;
	int				lnumtriangles;

	pfv = r_affinetridesc.pfinalverts;
	ptri = r_affinetridesc.ptriangles;
	lnumtriangles = r_affinetridesc.numtriangles;

	for (i=0 ; i<lnumtriangles ; i++)
	{
		index0 = pfv + ptri[i].vertindex[0];
		index1 = pfv + ptri[i].vertindex[1];
		index2 = pfv + ptri[i].vertindex[2];

		if (((index0->v[1]-index1->v[1]) *
			 (index0->v[0]-index2->v[0]) -
			 (index0->v[0]-index1->v[0]) *
			 (index0->v[1]-index2->v[1])) >= 0)
		{
			continue;
		}
		// TODO - colormod hack
		d_pcolormap = &((byte *)acolormap)[index0->v[4] & 0xFF00];

		if (ptri[i].facesfront)
		{
			D_PolysetRecursiveTriangle(index0->v, index1->v, index2->v);
		}
		else
		{
			int		s0, s1, s2;

			s0 = index0->v[2];
			s1 = index1->v[2];
			s2 = index2->v[2];

			if (index0->flags & ALIAS_ONSEAM)
				index0->v[2] += r_affinetridesc.seamfixupX16;
			if (index1->flags & ALIAS_ONSEAM)
				index1->v[2] += r_affinetridesc.seamfixupX16;
			if (index2->flags & ALIAS_ONSEAM)
				index2->v[2] += r_affinetridesc.seamfixupX16;

			D_PolysetRecursiveTriangle(index0->v, index1->v, index2->v);

			index0->v[2] = s0;
			index1->v[2] = s1;
			index2->v[2] = s2;
		}
	}
}


/*
================
D_DrawNonSubdiv
================
*/
void D_DrawNonSubdiv (void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	int				i;
	int				lnumtriangles;

	pfv = r_affinetridesc.pfinalverts;
	ptri = r_affinetridesc.ptriangles;
	lnumtriangles = r_affinetridesc.numtriangles;

	for (i=0 ; i<lnumtriangles ; i++, ptri++)
	{
		index0 = pfv + ptri->vertindex[0];
		index1 = pfv + ptri->vertindex[1];
		index2 = pfv + ptri->vertindex[2];

		d_xdenom = (index0->v[1]-index1->v[1]) *
				(index0->v[0]-index2->v[0]) -
				(index0->v[0]-index1->v[0])*(index0->v[1]-index2->v[1]);

		if (d_xdenom >= 0)
		{
			continue;
		}

		r_p0[0] = index0->v[0];		// u
		r_p0[1] = index0->v[1];		// v
		r_p0[2] = index0->v[2];		// s
		r_p0[3] = index0->v[3];		// t
		r_p0[4] = index0->v[4];		// light
		r_p0[5] = index0->v[5];		// iz
		
		r_p1[0] = index1->v[0];
		r_p1[1] = index1->v[1];
		r_p1[2] = index1->v[2];
		r_p1[3] = index1->v[3];
		r_p1[4] = index1->v[4];
		r_p1[5] = index1->v[5];


		r_p2[0] = index2->v[0];
		r_p2[1] = index2->v[1];
		r_p2[2] = index2->v[2];
		r_p2[3] = index2->v[3];
		r_p2[4] = index2->v[4];
		r_p2[5] = index2->v[5];

		if (coloredlights){
		r_p0[6] = index0->v[6];		// lightr
		r_p0[7] = index0->v[7];		// lightg
		r_p0[8] = index0->v[8];		// lightb
		r_p1[6] = index1->v[6];		// lightr
		r_p1[7] = index1->v[7];		// lightg
		r_p1[8] = index1->v[8];		// lightb
		r_p2[6] = index2->v[6];		// lightr
		r_p2[7] = index2->v[7];		// lightg
		r_p2[8] = index2->v[8];		// lightb
		}
		

		if (!ptri->facesfront)
		{
			if (index0->flags & ALIAS_ONSEAM)
				r_p0[2] += r_affinetridesc.seamfixupX16;
			if (index1->flags & ALIAS_ONSEAM)
				r_p1[2] += r_affinetridesc.seamfixupX16;
			if (index2->flags & ALIAS_ONSEAM)
				r_p2[2] += r_affinetridesc.seamfixupX16;
		}

		D_PolysetSetEdgeTable ();
		if (r_filter->value)
		D_RasterizeAliasPolySmooth ();
		else
		D_RasterizeAliasPolySmooth ();
	}
}


/*
================
D_PolysetRecursiveTriangle
================
*/
void D_PolysetRecursiveTriangle (int *lp1, int *lp2, int *lp3)
{
	int		*temp;
	int		d;
	int		new[9];
	int		z;
	short	*zbuf;

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;

	d = lp3[0] - lp2[0];
	if (d < -1 || d > 1)
		goto split2;
	d = lp3[1] - lp2[1];
	if (d < -1 || d > 1)
		goto split2;

	d = lp1[0] - lp3[0];
	if (d < -1 || d > 1)
		goto split3;
	d = lp1[1] - lp3[1];
	if (d < -1 || d > 1)
	{
split3:
		temp = lp1;
		lp1 = lp3;
		lp3 = lp2;
		lp2 = temp;

		goto split;
	}

	return;			// entire tri is filled

split2:
	temp = lp1;
	lp1 = lp2;
	lp2 = lp3;
	lp3 = temp;

split:
// split this edge
	new[0] = (lp1[0] + lp2[0]) >> 1;
	new[1] = (lp1[1] + lp2[1]) >> 1;
	new[2] = (lp1[2] + lp2[2]) >> 1;
	new[3] = (lp1[3] + lp2[3]) >> 1;
	new[5] = (lp1[5] + lp2[5]) >> 1;

// draw the point if splitting a leading edge
	if (lp2[1] > lp1[1])
		goto nodraw;
	if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
		goto nodraw;


	z = new[5]>>16;
	zbuf = zspantable[new[1]] + new[0];
	if (z >= *zbuf)
	{
		int		pix;

		*zbuf = z;
		pix = d_pcolormap[skintable[new[3]>>16][new[2]>>16]];
		d_viewbuffer[d_scantable[new[1]] + new[0]] = pix;
	}

nodraw:
// recursively continue
	D_PolysetRecursiveTriangle (lp3, lp1, new);
	D_PolysetRecursiveTriangle (lp3, new, lp2);
}

#endif	// !id386


/*
================
D_PolysetUpdateTables
================
*/
void D_PolysetUpdateTables (void)
{
	int		i;
	byte	*s;

	if (r_affinetridesc.skinwidth != skinwidth ||
		r_affinetridesc.pskin != skinstart)
	{
		skinwidth = r_affinetridesc.skinwidth;
		skinstart = r_affinetridesc.pskin;
		s = skinstart;
		for (i=0 ; i<MAX_LBM_HEIGHT ; i++, s+=skinwidth)
			skintable[i] = s;
	}
}


#if	!id386broken

/*
===================
D_PolysetScanLeftEdge
====================
*/
void D_PolysetScanLeftEdge (int height)
{

	do
	{
		d_pedgespanpackage->pdest = d_pdest;
		d_pedgespanpackage->pz = d_pz;
		d_pedgespanpackage->count = d_aspancount;
		d_pedgespanpackage->ptex = d_ptex;

		d_pedgespanpackage->sfrac = d_sfrac;
		d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?

		d_pedgespanpackage->light = d_light;
		d_pedgespanpackage->zi = d_zi;

		// leilei -  colored lights on models
if (coloredlights){
		d_pedgespanpackage->lightr = d_lightrgb[0];
		d_pedgespanpackage->lightg = d_lightrgb[1];
		d_pedgespanpackage->lightb = d_lightrgb[2];
		
}


		d_pedgespanpackage++;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_pdest += d_pdestextrastep;
			d_pz += d_pzextrastep;
			d_aspancount += d_countextrastep;
			d_ptex += d_ptexextrastep;
			d_sfrac += d_sfracextrastep;
			d_ptex += d_sfrac >> 16;

			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracextrastep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += r_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			
			d_light += d_lightextrastep ;

			d_zi += d_ziextrastep;
			d_lightrgb[0] += d_lightextrastepr;
			d_lightrgb[1] += d_lightextrastepg;
			d_lightrgb[2] += d_lightextrastepb;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_pdest += d_pdestbasestep;
			d_pz += d_pzbasestep;
			d_aspancount += ubasestep;
			d_ptex += d_ptexbasestep;
			d_sfrac += d_sfracbasestep;
			d_ptex += d_sfrac >> 16;
			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracbasestep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += r_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			
			d_light += d_lightbasestep;
			d_zi += d_zibasestep;
			d_lightrgb[0] += d_lightbasestepr;
			d_lightrgb[1] += d_lightbasestepg;
			d_lightrgb[2] += d_lightbasestepb;		
		}
	} while (--height);
}

#endif	// !id386

/*
===================
D_PolysetScanLeftEdge
====================
*/
void D_PolysetScanLeftEdgeFilter (int height)
{

	do
	{
		d_pedgespanpackage->pdest = d_pdest;
		d_pedgespanpackage->pz = d_pz;
		d_pedgespanpackage->count = d_aspancount;
		d_pedgespanpackage->ptex = d_ptex;

		d_pedgespanpackage->sfrac = d_sfrac;
		d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?

		d_pedgespanpackage->light = d_light;
		d_pedgespanpackage->zi = d_zi;

		// leilei -  colored lights on models
if (coloredlights){
		d_pedgespanpackage->lightr = d_lightrgb[0];
		d_pedgespanpackage->lightg = d_lightrgb[1];
		d_pedgespanpackage->lightb = d_lightrgb[2];
		
}


		d_pedgespanpackage++;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_pdest += d_pdestextrastep;
			d_pz += d_pzextrastep;
			d_aspancount += d_countextrastep;
			d_ptex += d_ptexextrastep;
			d_sfrac += d_sfracextrastep;
			d_ptex += d_sfrac >> 16;

			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracextrastep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += r_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			
			d_light += d_lightextrastep;

			d_zi += d_ziextrastep;
			d_lightrgb[0] += d_lightextrastepr;
			d_lightrgb[1] += d_lightextrastepg;
			d_lightrgb[2] += d_lightextrastepb;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_pdest += d_pdestbasestep;
			d_pz += d_pzbasestep;
			d_aspancount += ubasestep;
			d_ptex += d_ptexbasestep;
			d_sfrac += d_sfracbasestep;
			d_ptex += d_sfrac >> 16;
			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracbasestep;

			if (d_tfrac & 0x10000)
			{
				d_ptex += r_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			
			d_light += d_lightbasestep;
			d_zi += d_zibasestep;
			d_lightrgb[0] += d_lightbasestepr;
			d_lightrgb[1] += d_lightbasestepg;
			d_lightrgb[2] += d_lightbasestepb;		
		}
	} while (--height);
}

/*
===================
D_PolysetSetUpForLineScan
====================
*/
void D_PolysetSetUpForLineScan(fixed8_t startvertu, fixed8_t startvertv,
		fixed8_t endvertu, fixed8_t endvertv)
{
	double		dm, dn;
	int			tm, tn;
	adivtab_t	*ptemp;

// TODO: implement x86 version

	errorterm = -1;

	tm = endvertu - startvertu;
	tn = endvertv - startvertv;

	if (((tm <= 16) && (tm >= -15)) &&
		((tn <= 16) && (tn >= -15)))
	{
		ptemp = &adivtab[((tm+15) << 5) + (tn+15)];
		ubasestep = ptemp->quotient;
		erroradjustup = ptemp->remainder;
		erroradjustdown = tn;
	}
	else
	{
		dm = (double)tm;
		dn = (double)tn;

		FloorDivMod (dm, dn, &ubasestep, &erroradjustup);

		erroradjustdown = dn;
	}
}


#if	!id386broken

/*
================
D_PolysetCalcGradients
================
*/

extern cvar_t *temp1;
void D_PolysetCalcGradients (int skinwidth)
{
	float	xstepdenominv, ystepdenominv, t0, t1;
	float	p01_minus_p21, p11_minus_p21, p00_minus_p20, p10_minus_p20;

	p00_minus_p20 = r_p0[0] - r_p2[0];
	p01_minus_p21 = r_p0[1] - r_p2[1];
	p10_minus_p20 = r_p1[0] - r_p2[0];
	p11_minus_p21 = r_p1[1] - r_p2[1];

	xstepdenominv = 1.0 / (float)d_xdenom;

	ystepdenominv = -xstepdenominv;

// ceil () for light so positive steps are exaggerated, negative steps
// diminished,  pushing us away from underflow toward overflow. Underflow is
// very visible, overflow is very unlikely, because of ambient lighting
	t0 = r_p0[4] - r_p2[4];
	t1 = r_p1[4] - r_p2[4];
	r_lstepx = (int)
			ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_lstepy = (int)
			ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);

	if(coloredlights){
	t0 = r_p0[6] - r_p2[6];
	t1 = r_p1[6] - r_p2[6];
	r_lrstepx = (int)
			ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_lrstepy = (int)
			ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);

	t0 = r_p0[7] - r_p2[7];
	t1 = r_p1[7] - r_p2[7];
	r_lgstepx = (int)
			ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_lgstepy = (int)
			ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);

	t0 = r_p0[8] - r_p2[8];
	t1 = r_p1[8] - r_p2[8];
	r_lbstepx = (int)
			ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_lbstepy = (int)
			ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);


	}


	t0 = r_p0[2] - r_p2[2];
	t1 = r_p1[2] - r_p2[2];
	r_sstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
			xstepdenominv);
	r_sstepy = (int)((t1 * p00_minus_p20 - t0* p10_minus_p20) *
			ystepdenominv);

	t0 = r_p0[3] - r_p2[3];
	t1 = r_p1[3] - r_p2[3];
	r_tstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
			xstepdenominv);
	r_tstepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) *
			ystepdenominv);

	t0 = r_p0[5] - r_p2[5];
	t1 = r_p1[5] - r_p2[5];
	r_zistepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
			xstepdenominv);
	r_zistepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) *
			ystepdenominv);



#if	id386broken
	a_sstepxfrac = r_sstepx << 16;
	a_tstepxfrac = r_tstepx << 16;
#else
	a_sstepxfrac = r_sstepx & 0xFFFF;
	a_tstepxfrac = r_tstepx & 0xFFFF;
#endif

	a_ststepxwhole = skinwidth * (r_tstepx >> 16) + (r_sstepx >> 16);
}

#endif	// !id386


#if 0

#endif


#if	!id386broken




/*
================
D_PolysetDrawSpans8
================
*/
void D_PolysetDrawSpans8 (spanpackage_t *pspanpackage)
{
		return;		// we use an alternate 'c' version for now . ASMME: spanspanspanspan
}

#endif	// !id386
extern vec3_t	lightcolor;	

extern byte gelmap[256];
extern byte bumpmap[256];
extern pixel_t transTable[256][256];		
extern pixel_t addTable[256][256];		// Additive Blending
extern pixel_t mulTable[256][256];	 // Color Mod!
extern vec3_t colormod;
extern int	hqlite;	
extern int	ditheredrend;		// dithering



// Huge "enhanced stuff" C function for alias models
// only should be called when we need it (for effect flags, alpha, colored lighting)
// TODO: Split this into 3 functions and asm each as such:
// D_PolySetDrawSpans8Add
// D_PolySetDrawSpans8Alpha33
// D_PolySetDrawSpans8Alpha66
// D_PolySetDrawSpans8AlphaTest
// D_PolySetDrawSpans8Alpha15
// D_PolySetDrawSpans15Add
// D_PolySetDrawSpans15Alpha33
// D_PolySetDrawSpans15Alpha66
// D_PolySetDrawSpans15AlphaTest
// D_PolySetDrawSpans15Alpha15
// D_PolySetDrawSpans18Add
// D_PolySetDrawSpans18Alpha33
// D_PolySetDrawSpans18Alpha66
// D_PolySetDrawSpans18AlphaTest
// D_PolySetDrawSpans18Alpha18


extern int kernel[2][2][2];


void D_PolysetDrawSpans8RGB_C_Filter (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lpbuf;
	byte	*lptex;

	int		lsfrac, ltfrac;
	int		llight;
	int		llightrgb[3];
	int		colme[3];
	int  rgb;
	int		lzi;
	short	*lpz;
	unsigned char *pix24;	// leilei - colored lighting
	int trans[3];
		if (currententity->effects & EF_NODRAW || currententity->leifect)	
			return; // haha don't do it
	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;
			if(coloredlights){
			llightrgb[0] = pspanpackage->lightr;
			llightrgb[1] = pspanpackage->lightg;
			llightrgb[2] = pspanpackage->lightb;
			}
				
		do
			{
				if ((lzi >> 16) >= *lpz)
				{


					
				// Blending 
					if (gamemode == GAME_KUROK || *lptex != 255)	{
	
					if (currententity->effects & EF_ADDITIVE)
					*lpdest = addTable[((byte *)acolormap)[*lptex + (llight & 0xFF00)]][*lpdest];
					else{
						if (coloredlights){



				if (*lptex < host_fullbrights || currententity->effects & EF_FULLBRIGHT)
						{ 	// Preserve the fullbrights

								pix24 = (unsigned char *)&d_8to24table[((byte *)acolormap)[*lptex + (8192 & 0xFF00)]];
								// TODO FIXME: colored light vectors similar to dp105/q3
							{
							trans[0] = (pix24[0] * (16384 - llightrgb[0])) >> 15;
							trans[1] = (pix24[1] * (16384 - llightrgb[1])) >> 15;
						 	trans[2] = (pix24[2] * (16384 - llightrgb[2])) >> 15;
							
							if (trans[0] < 0) trans[0] = 0;	if (trans[1] < 0) trans[1] = 0;	if (trans[2] < 0) trans[2] = 0;
							if (trans[0] > 63) trans[0] = 63; if (trans[1] > 63) trans[1] = 63;	if (trans[2] > 63) trans[2] = 63;
								// 18-Bit lighting - Alpha Blending
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[palmap2[trans[0]][trans[1]][trans[2]]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][palmap2[trans[0]][trans[1]][trans[2]]];
							}
							else
								// 18-Bit lighting - Vanilla Blending
							*lpdest = palmap2[trans[0]][trans[1]][trans[2]];
														}
													
						}
						else
							// 15-Bit lighting - Alpha Blending - Fullbright colors
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex]];
							}
							else
								// 15-Bit lighting - Vanilla Blending - Fullbright colors
						
		if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
					else
						*lpdest = ((byte *)acolormap)[*lptex];
						}
						else

							// 8-Bit lighting - Alpha Blending
							if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex + (llight & 0xFF00)]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex + (llight & 0xFF00)]];
							}
							else
							// 8-Bit lighting - Vanilla stuff
							if (currententity->effects & EF_FULLBRIGHT)
								*lpdest = ((byte *)acolormap)[*lptex];
							else if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
								else
								
								*lpdest = ((byte *)acolormap)[*lptex + (llight & 0xFF00)];
								
								
							//	*lpdest++ = *(lpwast + idiths + iditht * r_affinetridesc.skinwidth);
								*lpz = lzi >> 16;
							}
						
					}
						
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				if(coloredlights){
				llightrgb[0] += r_lrstepx;
				llightrgb[1] += r_lgstepx;
				llightrgb[2] += r_lbstepx;
				}
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				
				ltfrac += a_tstepxfrac;
		
		
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);

		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}


extern cvar_t *temp2;
extern cvar_t *temp3;

void D_PolysetDrawSpans8RGB_C (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lpbuf;
	byte	*lptex;
	int			forg;			// leilei - fog
	int		lsfrac, ltfrac;
	int		llight;
	int		llightrgb[3];
	int		colme[3];
	int  rgb;
	int		lzi;
	int		lzf;	// leilei - fog
	float	zf;		// leilei - fog
	short	*lpz;
	unsigned char *pix24;	// leilei - colored lighting
	int trans[3];

	if (currententity->effects & EF_NODRAW || currententity->leifect)	
			return; // haha don't do it
/*
	// FOG
	if (foguse)
	{
		vec3_t	local, transformed;
		float	zi;
		int	izi;
			// transform point
		VectorSubtract (currententity->origin, r_origin, local);

		transformed[0] = DotProduct(local, r_pright);
		transformed[1] = DotProduct(local, r_pup);
		transformed[2] = DotProduct(local, r_ppn);

		zf = 1.0 / transformed[2];
		lzf = (int)(zf * 0x8000);
	//	forg = (float)lzf * -128 + 32768;
		
		if (forg > 32364)	forg = 32364; 
		if (forg < 0)		forg = 0;
	}
*/
	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;
			if(coloredlights){
			llightrgb[0] = pspanpackage->lightr;
			llightrgb[1] = pspanpackage->lightg;
			llightrgb[2] = pspanpackage->lightb;
			}
				
		do
			{
				if (foguse){ forg = lzi / 1024 * -3 + 16384 * (2.6 ) / 1.5;	if (forg > 32762)	forg = 32762; if (forg < 0)	forg = 0;
				*lptex = host_fogmap[*lptex + (forg >> 2 & 0xFF00)];
				
				} // leilei - fog FIXME- dumb math
				if ((lzi >> 16) >= *lpz)
				{

				// Blending 
					if (gamemode == GAME_KUROK || *lptex != 255)	{
	
					if (currententity->effects & EF_ADDITIVE)
					*lpdest = addTable[((byte *)acolormap)[*lptex + (llight & 0xFF00)]][*lpdest];
					else{
						if (coloredlights){



				if (*lptex < host_fullbrights || currententity->effects & EF_FULLBRIGHT)
						{ 	// Preserve the fullbrights

								pix24 = (unsigned char *)&d_8to24table[((byte *)acolormap)[*lptex + (8192 & 0xFF00)]];
								// TODO FIXME: colored light vectors similar to dp105/q3
							{
							trans[0] = (pix24[0] * (16384 - llightrgb[0])) >> 15;
							trans[1] = (pix24[1] * (16384 - llightrgb[1])) >> 15;
						 	trans[2] = (pix24[2] * (16384 - llightrgb[2])) >> 15;
							
							if (trans[0] < 0) trans[0] = 0;	if (trans[1] < 0) trans[1] = 0;	if (trans[2] < 0) trans[2] = 0;
							if (trans[0] > 63) trans[0] = 63; if (trans[1] > 63) trans[1] = 63;	if (trans[2] > 63) trans[2] = 63;
								// 18-Bit lighting - Alpha Blending
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[palmap2[trans[0]][trans[1]][trans[2]]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][palmap2[trans[0]][trans[1]][trans[2]]];
							}
							else
								// 18-Bit lighting - Vanilla Blending
							*lpdest = palmap2[trans[0]][trans[1]][trans[2]];
														}
													
						}
						else
							// 15-Bit lighting - Alpha Blending - Fullbright colors
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex]];
							}
							else
								// 15-Bit lighting - Vanilla Blending - Fullbright colors
						
		if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
					else
						*lpdest = ((byte *)acolormap)[*lptex];
						}
						else

							// 8-Bit lighting - Alpha Blending
							if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex + (llight & 0xFF00)]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex + (llight & 0xFF00)]];
							}
							else
							// 8-Bit lighting - Vanilla stuff
							if (currententity->effects & EF_FULLBRIGHT)
								*lpdest = ((byte *)acolormap)[*lptex];
							else if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
								else
								*lpdest = ((byte *)acolormap)[*lptex + (llight & 0xFF00)];

							//	if (foguse)
							//	 *lpdest = host_fogmap[*lpdest + (forg >> 2 & 0xFF00)];
								*lpz = lzi >> 16;
							}
					}
				
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				if(coloredlights){
				llightrgb[0] += r_lrstepx;
				llightrgb[1] += r_lgstepx;
				llightrgb[2] += r_lbstepx;
				}
				
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
				
			} while (--lcount);

		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

extern int fogrange;
extern cvar_t	*r_shadedither;
int	shadeq;
extern	cvar_t	*r_shadequality;
extern	cvar_t	*r_shinygrays;
extern	cvar_t	*temp1;

extern byte menumap[256][16];	

void D_PolysetDrawSpans8_Low_C (spanpackage_t *pspanpackage);
extern int coloredmethod;
void D_PolysetDrawSpans8_C (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lpbuf;
	byte	*lptex;
	int forg;	// leilei - fog
	int		lsfrac, ltfrac;
	
	int		llight;
	int		llightd;
	int		llightrgb[3];
	int		llightrgbd[3];
	int		colme[3];
	int  rgb;
	int		lzi;
	short	*lpz;
	unsigned char *pix24;	// leilei - colored lighting
	int trans[3];
	int	shad = 0; // leilei - shade offset
	float bhad = 1;
	int ta, te, tv;
	int	dith, det; // leilei - shading dither
	int	dodith, doshine;
	int s, t;
	int s2, t2;
//	if (coloredmethod == 1){
//			D_PolysetDrawSpans8_Low_C (pspanpackage);
//			return;
//	}
	dodith = r_shadedither->value;
	doshine = r_shinygrays->value;
	if (currententity->effects & EF_NODRAW || currententity->leifect)	
			return; // haha don't do it
	
		det = 0;
	shadeq = (int)r_shadequality->value;
	do
	{
		lcount = d_aspancount - pspanpackage->count;
		
		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;

		


			det++;	
		
			if (det > 3)
			det = 0;			
	//		llight = pspanpackage->light;
			if (dodith)
			llightd = pspanpackage->light;
			else
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			
	
         //   if (lsfrac<0x8000) {
         //    lptex-=1;
         //   }
			   
    
			if(coloredlights){

			if (dodith){
			llightrgbd[0] = pspanpackage->lightr;
			llightrgbd[1] = pspanpackage->lightg;
			llightrgbd[2] = pspanpackage->lightb;
			if (llightrgbd[0] > 16000) llightrgbd[0] = 16000;
			if (llightrgbd[1] > 16000) llightrgbd[1] = 16000;
			if (llightrgbd[2] > 16000) llightrgbd[2] = 16000;
			}	
			else
			{
			llightrgb[0] = pspanpackage->lightr;
			llightrgb[1] = pspanpackage->lightg;
			llightrgb[2] = pspanpackage->lightb;
			if (llightrgb[0] > 16000) llightrgb[0] = 16000;
			if (llightrgb[1] > 16000) llightrgb[1] = 16000;
			if (llightrgb[2] > 16000) llightrgb[2] = 16000;

			}
				
				
				
			
			} // leilei - fog FIXME- dumb math
			if (foguse){ forg = lzi / 1024 * -3 + 16384 * (2.6 ) / 1.5;	if (forg > 32762)	forg = 32762; if (forg < 0)	forg = 0; 


			}

			
			
		do
			{
			
		
						if ((lzi >> 16) >= *lpz)
					{
							         
					
							

							if (dodith){
								det++;
								if (det > 3) det = 0;
						llight = smoothtable[llightd][det]; // leilei - dithered shading	
						if (coloredlights){
							llightrgb[0] = smoothtable[llightrgbd[0]][det];
							llightrgb[1] = smoothtable[llightrgbd[1]][det];
							llightrgb[2] = smoothtable[llightrgbd[2]][det];
						
			
						}

					// metal shine...
			// Shine crap

						if ((doshine == 2)&& *lptex < 16){
							int ti;


							llightrgb[0] &= 0xAF00;
							llightrgb[1] &= 0xAF00;
							llightrgb[2] &= 0xAF00;
							llight		 &= 0xAF00;
							
							



						}
               
					  
					//	*lpdest = palmap2[trans[0]][trans[1]][trans[2]];
			//			*lpdest = *(lptex + idiths + iditht);
						//    *lpz = lzi >> 16;
						
						
							}
			
			//	-------------------------------------------------
			// Big FAT BLENDING ROUTINES Begin
			//	-------------------------------------------------
						
							

				// Blending 
					if (gamemode == GAME_KUROK || *lptex != 255)	{
	
					if (currententity->effects & EF_ADDITIVE)
					*lpdest = addTable[((byte *)acolormap)[*lptex + (llight + shad &  0xFF00)]][*lpdest];
					else{
						if (coloredlights){



				if (*lptex < host_fullbrights || currententity->effects & EF_FULLBRIGHT)
						{ 	// Preserve the fullbrights
								pix24 = (unsigned char *)&d_8to24table[((byte *)acolormap)[*lptex + (8192 & 0xF000)]];
								// TODO FIXME: colored light vectors similar to dp105/q3
							{
								
							trans[0] = (pix24[0] * (16384 - shad - llightrgb[0])) >> 15;
							trans[1] = (pix24[1] * (16384 - shad - llightrgb[1])) >> 15;
						 	trans[2] = (pix24[2] * (16384 - shad - llightrgb[2])) >> 15;

							
							//if (trans[0] < 0) trans[0] = 0;	if (trans[1] < 0) trans[1] = 0;	if (trans[2] < 0) trans[2] = 0;
							if (trans[0] & ~63) trans[0] = 63; if (trans[1] & ~63) trans[1] = 63;	if (trans[2] & ~63) trans[2] = 63;
								// 18-Bit lighting - Alpha Blending
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[palmap2[trans[0]][trans[1]][trans[2]]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][palmap2[trans[0]][trans[1]][trans[2]]];
							}
							else
								// 18-Bit lighting - Vanilla Blending
							*lpdest = palmap2[trans[0]][trans[1]][trans[2]];
														}
													
						}
						else
							// 15-Bit lighting - Alpha Blending - Fullbright colors
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex]];
							}
							else
								// 15-Bit lighting - Vanilla Blending - Fullbright colors
						
		if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
					else
						*lpdest = ((byte *)acolormap)[*lptex];
						}
						else

							// 8-Bit lighting - Alpha Blending
							if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex + (llight  + shad & 0xFF00)]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex + (llight  + shad & 0xFF00)]];
							}
							else
							// 8-Bit lighting - Vanilla stuff
						//	shad = menumap[*lptex][0] - (menumap[*lptex][0]/2) * temp2->value;	// leilei - psuedo bump mapping experiment
						//	bhad = menumap[*lptex][0] + temp2->value + 1;
							
							if (currententity->effects & EF_FULLBRIGHT)
								*lpdest = ((byte *)acolormap)[*lptex];
							else if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
							else
								*lpdest = ((byte *)acolormap)[*lptex + (llight & 0xFF00)];
							

								if (foguse)
								 *lpdest = host_fogmap[*lpdest + (forg >> 2 & 0xFF00)];						
				
							
								*lpz = lzi >> 16;
							}
					// Stupid experiment - metal shine


					if (doshine == 1 && *lptex < 16 )
							*lpdest= ((byte *)acolormap)[*lpdest + (llight & 0xAF00)];
					
					}
			//	-------------------------------------------------
			// Big FAT BLENDING ROUTINES End
			//	-------------------------------------------------
	
				
				}
						
				lpdest++;
				
				lzi += r_zistepx;
				lpz++;
				
				

				if (dodith)
				llightd += r_lstepx;
				else
				llight += r_lstepx;
	
				//	llight += r_lstepx;
			//	if (dodith)
			//	llight = smoothtable[llightd][0];
				if(coloredlights){

				if (dodith){
				llightrgbd[0] += r_lrstepx;
				llightrgbd[1] += r_lgstepx;
				llightrgbd[2] += r_lbstepx;
				}
				else
				{
				llightrgb[0] += r_lrstepx;
				llightrgb[1] += r_lgstepx;
				llightrgb[2] += r_lbstepx;

				}
				}

				
				
				lptex += a_ststepxwhole;
				
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
					
				}
				
			} while (--lcount);

		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

// leilei - lookup colored lighting version (slimmer maybe...)
void D_PolysetDrawSpans8_Low_C (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lpbuf;
	byte	*lptex;
	int forg;	// leilei - fog
	int		lsfrac, ltfrac;
	
	int		llight;
	int		llightd;
	int		llightrgb[3];
	int		llightrgbd[3];
	int		colme[3];
	int  rgb;
	int		lzi;
	short	*lpz;
	unsigned char *pix24;	// leilei - colored lighting
	int trans[3];
	int	shad = 0; // leilei - shade offset
	float bhad = 1;
	int ta, te, tv;
	int	dith, det; // leilei - shading dither
	int	dodith;
	int s, t;
	dodith = r_shadedither->value;
	
	if (currententity->effects & EF_NODRAW || currententity->leifect)	
			return; // haha don't do it
		det = 0;
	shadeq = (int)r_shadequality->value;
	do
	{
		lcount = d_aspancount - pspanpackage->count;
		
		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;

			s = lsfrac;
			t = ltfrac;
			det++;	
		
			if (det > 3)
			det = 0;			
	//		llight = pspanpackage->light;
			if (dodith)
			llightd = pspanpackage->light;
			else
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			
	
         //   if (lsfrac<0x8000) {
         //    lptex-=1;
         //   }
			   
    
			if(coloredlights){

			if (dodith){
			llightrgbd[0] = pspanpackage->lightr * -1 + 16384;
			llightrgbd[1] = pspanpackage->lightg * -1 + 16384;
			llightrgbd[2] = pspanpackage->lightb * -1 + 16384;
			if (llightrgbd[0] > 16000) llightrgbd[0] = 16000;
			if (llightrgbd[1] > 16000) llightrgbd[1] = 16000;
			if (llightrgbd[2] > 16000) llightrgbd[2] = 16000;
			}	
			else
			{
			llightrgb[0] = 32768 - pspanpackage->lightr;
			llightrgb[1] = 32768 -pspanpackage->lightg;
			llightrgb[2] = 32768 -pspanpackage->lightb;
			if (llightrgb[0] > 16000) llightrgb[0] = 16000;
			if (llightrgb[1] > 16000) llightrgb[1] = 16000;
			if (llightrgb[2] > 16000) llightrgb[2] = 16000;

			}
				
				
				
			
			} // leilei - fog FIXME- dumb math
			if (foguse){ forg = lzi / 1024 * -3 + 16384 * (2.6 ) / 1.5;	if (forg > 32762)	forg = 32762; if (forg < 0)	forg = 0; 


			}

			
			
		do
			{
			
		
						if ((lzi >> 16) >= *lpz)
					{
							         
					
							if (dodith){
								det++;
								if (det > 3) det = 0;
						llight = smoothtable[llightd][det]; // leilei - dithered shading	
						if (coloredlights){
							llightrgb[0] = smoothtable[llightrgbd[0]][det];
							llightrgb[1] = smoothtable[llightrgbd[1]][det];
							llightrgb[2] = smoothtable[llightrgbd[2]][det];
						
					
						}
               
               /* 
					//	*lpdest = palmap2[trans[0]][trans[1]][trans[2]];
						*lpdest = *(lptex + idiths + iditht);
						    *lpz = lzi >> 16;
						*/
						
							}
			//	-------------------------------------------------
			// Big FAT BLENDING ROUTINES Begin
			//	-------------------------------------------------
	
							

				// Blending 
					if (gamemode == GAME_KUROK || *lptex != 255)	{
	
					if (currententity->effects & EF_ADDITIVE)
					*lpdest = addTable[((byte *)acolormap)[*lptex + (llight + shad &  0xFF00)]][*lpdest];
					else{
						if (coloredlights){



				if (*lptex < host_fullbrights || currententity->effects & EF_FULLBRIGHT)
						{ 	// Preserve the fullbrights
								pix24 = (unsigned char *)&d_8to24table[((byte *)acolormap)[*lptex + (8192 & 0xF000)]];
								// TODO FIXME: colored light vectors similar to dp105/q3
							{
								
								
							
							
								// 18-Bit lighting - Alpha Blending
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[palmap2[trans[0]][trans[1]][trans[2]]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][palmap2[trans[0]][trans[1]][trans[2]]];
							}
							else
								// 18-Bit lighting - Vanilla Blending
							*lpdest = palmap
							[host_colormap_red		[(llightrgb[0] & 0xFF00)  + *lptex]]
							[host_colormap_green	[(llightrgb[1] & 0xFF00) + *lptex]]
							[host_colormap_blue		[(llightrgb[2] & 0xFF00)+ *lptex]];
														}
													
						}
						else
							// 15-Bit lighting - Alpha Blending - Fullbright colors
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex]];
							}
							else
								// 15-Bit lighting - Vanilla Blending - Fullbright colors
						
		if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
					else
						*lpdest = ((byte *)acolormap)[*lptex];
						}
						else

							// 8-Bit lighting - Alpha Blending
							if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex + (llight  + shad & 0xFF00)]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex + (llight  + shad & 0xFF00)]];
							}
							else
							// 8-Bit lighting - Vanilla stuff
						//	shad = menumap[*lptex][0] - (menumap[*lptex][0]/2) * temp2->value;	// leilei - psuedo bump mapping experiment
						//	bhad = menumap[*lptex][0] + temp2->value + 1;
							
							if (currententity->effects & EF_FULLBRIGHT)
								*lpdest = ((byte *)acolormap)[*lptex];
							else if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
							else
								*lpdest = ((byte *)acolormap)[*lptex + (llight & 0xFF00)];
							

								if (foguse)
								 *lpdest = host_fogmap[*lpdest + (forg >> 2 & 0xFF00)];						
				
							
								*lpz = lzi >> 16;
							}
							
					}
			//	-------------------------------------------------
			// Big FAT BLENDING ROUTINES End
			//	-------------------------------------------------
	
				
				}
						
				lpdest++;
				
				lzi += r_zistepx;
				lpz++;
				
				

				if (dodith)
				llightd += r_lstepx;
				else
				llight += r_lstepx;
	
				//	llight += r_lstepx;
			//	if (dodith)
			//	llight = smoothtable[llightd][0];
				if(coloredlights){

				if (dodith){
				llightrgbd[0] += r_lrstepx;
				llightrgbd[1] += r_lgstepx;
				llightrgbd[2] += r_lbstepx;
				}
				else
				{
				llightrgb[0] += r_lrstepx;
				llightrgb[1] += r_lgstepx;
				llightrgb[2] += r_lbstepx;

				}
				}

				
				
				lptex += a_ststepxwhole;
				
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
					
				}
				
			} while (--lcount);

		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}



void D_PolysetDrawSpans8_C_Dither (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lpbuf;
	byte	*lptex;

	int		lsfrac, ltfrac;
	int		llight;
	int		llightrgb[3];
	int		colme[3];
	int  rgb;
	int		lzi;
	short	*lpz;
	unsigned char *pix24;	// leilei - colored lighting
	int trans[3];
if (currententity->effects & EF_NODRAW || currententity->leifect)	
			return; // haha don't do it
	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;
			if(coloredlights){
			llightrgb[0] = pspanpackage->lightr;
			llightrgb[1] = pspanpackage->lightg;
			llightrgb[2] = pspanpackage->lightb;
			}
				
		do
			{
				if ((lzi >> 16) >= *lpz)
				{
				// Blending 
					if (gamemode == GAME_KUROK || *lptex != 255)	{
	
					if (currententity->effects & EF_ADDITIVE)
					*lpdest = addTable[((byte *)acolormap)[*lptex + (llight & 0xFF00)]][*lpdest];
					else{
						if (coloredlights){



				if (*lptex < host_fullbrights || currententity->effects & EF_FULLBRIGHT)
						{ 	// Preserve the fullbrights

								pix24 = (unsigned char *)&d_8to24table[((byte *)acolormap)[*lptex + (8192 & 0xFF00)]];
								// TODO FIXME: colored light vectors similar to dp105/q3
								if (hqlite){
							trans[0] = (pix24[0] * (16384 - llightrgb[0])) >> 15;
							trans[1] = (pix24[1] * (16384 - llightrgb[1])) >> 15;
						 	trans[2] = (pix24[2] * (16384 - llightrgb[2])) >> 15;
							
							if (trans[0] < 0) trans[0] = 0;	if (trans[1] < 0) trans[1] = 0;	if (trans[2] < 0) trans[2] = 0;
							if (trans[0] > 63) trans[0] = 63; if (trans[1] > 63) trans[1] = 63;	if (trans[2] > 63) trans[2] = 63;
								// 18-Bit lighting - Alpha Blending
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[palmap2[trans[0]][trans[1]][trans[2]]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][palmap2[trans[0]][trans[1]][trans[2]]];
							}
							else
								// 18-Bit lighting - Vanilla Blending
							*lpdest = palmap2[trans[0]][trans[1]][trans[2]];
														}
								else
								{
							trans[0] = (pix24[0] * (16384 - llightrgb[0])) >> 16;
							trans[1] = (pix24[1] * (16384 - llightrgb[1])) >> 16;
						 	trans[2] = (pix24[2] * (16384 - llightrgb[2])) >> 16;
						
							if (trans[0] < 0) trans[0] = 0;	if (trans[1] < 0) trans[1] = 0;	if (trans[2] < 0) trans[2] = 0;
							if (trans[0] > 31) trans[0] = 31; if (trans[1] > 31) trans[1] = 31;	if (trans[2] > 31) trans[2] = 31;
							// 15-Bit lighting - Alpha Blending
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[palmap[trans[0]][trans[1]][trans[2]]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][palmap[trans[0]][trans[1]][trans[2]]];
							}
							else
								// 15-Bit lighting - Vanilla Blending
							*lpdest = palmap[trans[0]][trans[1]][trans[2]];
								}

						
						}
						else
							// 15-Bit lighting - Alpha Blending - Fullbright colors
								if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex]];
							}
							else
								// 15-Bit lighting - Vanilla Blending - Fullbright colors
						
		if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
					else
						*lpdest = ((byte *)acolormap)[*lptex];
						}
						else

							// 8-Bit lighting - Alpha Blending
							if (currententity->alpha){
								if (currententity->alpha < 0.33)
								*lpdest = transTable[((byte *)acolormap)[*lptex + (llight & 0xFF00)]][*lpdest];
								else if (currententity->alpha < 0.88)
								*lpdest = transTable[*lpdest][((byte *)acolormap)[*lptex + (llight & 0xFF00)]];
							}
							else
							// 8-Bit lighting - Vanilla stuff
							if (currententity->effects & EF_FULLBRIGHT)
								*lpdest = ((byte *)acolormap)[*lptex];
							else if (*lptex > host_fullbrights && currententity->muzzlehack)
							*lpdest = addTable[((byte *)acolormap)[*lptex]][*lpdest];
								else
								*lpdest = ((byte *)acolormap)[*lptex + (llight & 0xFF00)];

								
								*lpz = lzi >> 16;
							}
					}
				
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				if(coloredlights){
				llightrgb[0] += r_lrstepx;
				llightrgb[1] += r_lgstepx;
				llightrgb[2] += r_lbstepx;
				}
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);

		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}



int karnel[2][2][2] =
{
        {
                {16384,0},
                {49152,16384}
        }
        ,
        {
                {32768,49152},
                {0,32768}
   }
};


extern pixel_t ditherTable[262144][4];

//extern pixel_t ditherTable[32768][4];
void D_PolysetDrawSpans8_C_Filter (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte	*lpdest;
	byte	*lpbuf;
	byte	*lptex;
	
	
	int		lsfrac, ltfrac;
	int		llight;
	int		llightrgb[3];
	int		colme[3];
	int  rgb;
	int n;
	int		lzi;
	short	*lpz;
	unsigned char *pix24;	// leilei - colored lighting
	unsigned char *dix24;	// leilei - colored dithering
	int trans[3];
	if (currententity->effects & EF_NODRAW || currententity->leifect)	
			return; // haha don't do it
	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;
			if(coloredlights){
			llightrgb[0] = pspanpackage->lightr;
			llightrgb[1] = pspanpackage->lightg;
			llightrgb[2] = pspanpackage->lightb;
			}
			do
			{
				if ((lzi >> 16) >= *lpz)
				{
							/*		int idiths = lsfrac;
			{
				int idiths = s;
				int iditht = t;

				int X = (pspan->u + spancount) & 1;
				int Y = (pspan->v)&1;

				idiths += kernel[X][Y][0];
				iditht += kernel[X][Y][1];

				idiths = idiths >> 16;
				idiths = idiths ? idiths -1 : idiths;
										

				iditht = iditht >> 16;
				iditht = iditht ? iditht -1 : iditht;
				*pdest++ = *(pbase + idiths + iditht * cachewidth);


*/
					{ 	
						int idiths = lsfrac; int iditht = lsfrac;
						int X = (lsfrac + d_aspancount) & 1;
						int Y = (ltfrac)&1;

						idiths += kernel[X][Y][0];
						iditht += kernel[X][Y][1];

						idiths = idiths >> 16;
						idiths = idiths ? idiths -1 : idiths;
												

						iditht = iditht >> 16;
						iditht = iditht ? iditht -1 : iditht;

				    
							pix24 = (unsigned char *)&d_8to24table[((byte *)acolormap)[*lptex + + idiths + iditht + (8192 & 0xFF00)]];
								// TODO FIXME: colored light vectors similar to dp105/q3

							trans[0] = (pix24[0] * (16384 - llightrgb[0])) >> 15;
							trans[1] = (pix24[1] * (16384 - llightrgb[1])) >> 15;
						 	trans[2] = (pix24[2] * (16384 - llightrgb[2])) >> 15;
							
							if (trans[0] < 0) trans[0] = 0;	if (trans[1] < 0) trans[1] = 0;	if (trans[2] < 0) trans[2] = 0;
							if (trans[0] > 63) trans[0] = 63; if (trans[1] > 63) trans[1] = 63;	if (trans[2] > 63) trans[2] = 63;

								// 18-Bit lighting - Vanilla Blending
							//*lpdest = *(lptex + idiths + iditht);
							*lpdest = palmap2[trans[0]][trans[1]][trans[2]];
							*lpz = lzi >> 16;
							
					}
					/*
				// Blending 
				//	if (gamemode == GAME_KUROK || *lptex != 255)	{
	
			//		if (currententity->effects & EF_ADDITIVE)
				//	*lpdest = addTable[((byte *)acolormap)[*lptex + (llight & 0xFF00)]][*lpdest];
			//		else{
						if (*lptex < host_fullbrights || currententity->effects & EF_FULLBRIGHT)
						{ 	// Preserve the fullbrights

								pix24 = (unsigned char *)&d_8to24table[((byte *)acolormap)[*lptex + (8192 & 0xFF00)]];
								// TODO FIXME: colored light vectors similar to dp105/q3
								{
							trans[0] = (pix24[0] * (16384 - llightrgb[0])) >> 15;
							trans[1] = (pix24[1] * (16384 - llightrgb[1])) >> 15;
						 	trans[2] = (pix24[2] * (16384 - llightrgb[2])) >> 15;

							if (trans[0] < 0) trans[0] = 0;	if (trans[1] < 0) trans[1] = 0;	if (trans[2] < 0) trans[2] = 0;
							if (trans[0] > 63) trans[0] = 63; if (trans[1] > 63) trans[1] = 63;	if (trans[2] > 63) trans[2] = 63;
							

								 n = ((lzi & 1) << 1) | (lzi & 1);
								{
								
								trans[0] <<= 10;
								trans[1] <<= 5;
								trans[2] <<= 0;
								*lpdest = ditherTable[*trans & 0x7FFF][n];
								n ^= 1;
								}
								 
						
						}
							//	else
							//	{
							//	*lpdest = *lptex;
							//	}
				
						*lpz = lzi >> 16;
						}
				*/	
				
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				if(coloredlights){
				llightrgb[0] += r_lrstepx;
				llightrgb[1] += r_lgstepx;
				llightrgb[2] += r_lbstepx;
				}
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

/*
================
D_PolysetFillSpans8
================
*/
void D_PolysetFillSpans8 (spanpackage_t *pspanpackage)
{
	int				color;

// FIXME: do z buffering

	color = d_aflatcolor++;

	while (1)
	{
		int		lcount;
		byte	*lpdest;

		lcount = pspanpackage->count;

		if (lcount == -1)
			return;

		if (lcount)
		{
			lpdest = pspanpackage->pdest;

			do
			{
				*lpdest++ = color;
			} while (--lcount);
		}

		pspanpackage++;
	}
}

/*
================
D_RasterizeAliasPolySmooth
================
*/
void D_RasterizeAliasPolySmooth (void)
{
	int				initialleftheight, initialrightheight;
	int				*plefttop, *prighttop, *pleftbottom, *prightbottom;
	int				working_lstepx, originalcount;
	int				working_lrstepx;
	int				working_lgstepx;
	int				working_lbstepx;

	plefttop = pedgetable->pleftedgevert0;
	prighttop = pedgetable->prightedgevert0;

	pleftbottom = pedgetable->pleftedgevert1;
	prightbottom = pedgetable->prightedgevert1;

	initialleftheight = pleftbottom[1] - plefttop[1];
	initialrightheight = prightbottom[1] - prighttop[1];
	
	
	
//
// set the s, t, and light gradients, which are consistent across the triangle
// because being a triangle, things are affine
//
	D_PolysetCalcGradients (r_affinetridesc.skinwidth);

//
// rasterize the polygon
//

//
// scan out the top (and possibly only) part of the left edge
//
	d_pedgespanpackage = a_spans;

	ystart = plefttop[1];
	d_aspancount = plefttop[0] - prighttop[0];

	d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> 16) +
			(plefttop[3] >> 16) * r_affinetridesc.skinwidth;
#if	id386broken
	d_sfrac = (plefttop[2] & 0xFFFF) << 16;
	d_tfrac = (plefttop[3] & 0xFFFF) << 16;
#else
	d_sfrac = plefttop[2] & 0xFFFF;
	d_tfrac = plefttop[3] & 0xFFFF;
#endif
	d_light = plefttop[4];
	d_zi = plefttop[5];
	d_lightrgb[0] = plefttop[6];
	d_lightrgb[1] = plefttop[7];
	d_lightrgb[2] = plefttop[8];
	d_pdest = (byte *)d_viewbuffer +
			ystart * screenwidth + plefttop[0];
	d_pz = d_pzbuffer + ystart * d_zwidth + plefttop[0];

	if (initialleftheight == 1)
	{
		d_pedgespanpackage->pdest = d_pdest;
		d_pedgespanpackage->pz = d_pz;
		d_pedgespanpackage->count = d_aspancount;
		d_pedgespanpackage->ptex = d_ptex;

		d_pedgespanpackage->sfrac = d_sfrac;
		d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?
		d_pedgespanpackage->light = d_light;

		d_pedgespanpackage->zi = d_zi;
// leilei - colored lights on models
		if (coloredlights){
			d_pedgespanpackage->lightr = d_lightrgb[0];
			d_pedgespanpackage->lightg = d_lightrgb[1];
			d_pedgespanpackage->lightb = d_lightrgb[2];
			}

		d_pedgespanpackage++;
	}
	else
	{
		D_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
							  pleftbottom[0], pleftbottom[1]);

	#if	id386broken
		d_pzbasestep = (d_zwidth + ubasestep) << 1;
		d_pzextrastep = d_pzbasestep + 2;
	#else
		d_pzbasestep = d_zwidth + ubasestep;
		d_pzextrastep = d_pzbasestep + 1;
	#endif

		d_pdestbasestep = screenwidth + ubasestep;
		d_pdestextrastep = d_pdestbasestep + 1;

	// TODO: can reuse partial expressions here

	// for negative steps in x along left edge, bias toward overflow rather than
	// underflow (sort of turning the floor () we did in the gradient calcs into
	// ceil (), but plus a little bit)
		if (ubasestep < 0){
			working_lstepx = r_lstepx - 1;
			working_lrstepx = r_lrstepx - 1;
			working_lgstepx = r_lgstepx - 1;
			working_lbstepx = r_lbstepx - 1;
		}
		else
		{
			working_lstepx = r_lstepx;
			working_lrstepx = r_lrstepx;
			working_lgstepx = r_lgstepx;
			working_lbstepx = r_lbstepx;


		}

		d_countextrastep = ubasestep + 1;
		d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
				((r_tstepy + r_tstepx * ubasestep) >> 16) *
				r_affinetridesc.skinwidth;
	#if	id386broken
		d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) << 16;
		d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) << 16;
	#else
		d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
		d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
	#endif
		d_lightbasestep = r_lstepy + working_lstepx * ubasestep;

		d_zibasestep = r_zistepy + r_zistepx * ubasestep;

		d_lightbasestepr = r_lrstepy + working_lrstepx * ubasestep;
		d_lightbasestepg = r_lgstepy + working_lgstepx * ubasestep;
		d_lightbasestepb = r_lbstepy + working_lbstepx * ubasestep;



		d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
				((r_tstepy + r_tstepx * d_countextrastep) >> 16) *
				r_affinetridesc.skinwidth;
	#if	id386broken
		d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) << 16;
		d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) << 16;
	#else
		d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) & 0xFFFF;
		d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) & 0xFFFF;
	#endif
		d_lightextrastep = d_lightbasestep + working_lstepx;

		d_ziextrastep = d_zibasestep + r_zistepx;

		d_lightextrastepr = d_lightbasestepr + working_lrstepx;
		d_lightextrastepg = d_lightbasestepg + working_lgstepx;
		d_lightextrastepb = d_lightbasestepb + working_lbstepx;

		D_PolysetScanLeftEdge (initialleftheight);
	}

//
// scan out the bottom part of the left edge, if it exists
//
	if (pedgetable->numleftedges == 2)
	{
		int		height;

		plefttop = pleftbottom;
		pleftbottom = pedgetable->pleftedgevert2;

		height = pleftbottom[1] - plefttop[1];

// TODO: make this a function; modularize this function in general

		ystart = plefttop[1];
		d_aspancount = plefttop[0] - prighttop[0];
		d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> 16) +
				(plefttop[3] >> 16) * r_affinetridesc.skinwidth;
		d_sfrac = 0;
		d_tfrac = 0;
		d_light = plefttop[4];
		d_zi = plefttop[5];
		d_lightrgb[0] = plefttop[6];
		d_lightrgb[1] = plefttop[7];
		d_lightrgb[2] = plefttop[8];
		d_pdest = (byte *)d_viewbuffer + ystart * screenwidth + plefttop[0];
		d_pz = d_pzbuffer + ystart * d_zwidth + plefttop[0];

		if (height == 1)
		{
			d_pedgespanpackage->pdest = d_pdest;
			d_pedgespanpackage->pz = d_pz;
			d_pedgespanpackage->count = d_aspancount;
			d_pedgespanpackage->ptex = d_ptex;

			d_pedgespanpackage->sfrac = d_sfrac;
			d_pedgespanpackage->tfrac = d_tfrac;

		// FIXME: need to clamp l, s, t, at both ends?


			d_pedgespanpackage->light = d_light;
			d_pedgespanpackage->zi = d_zi;
// leilei - colored lights on models
		if (coloredlights){
			d_pedgespanpackage->lightr = d_lightrgb[0];
			d_pedgespanpackage->lightg = d_lightrgb[1];
			d_pedgespanpackage->lightb = d_lightrgb[2];
			}



			d_pedgespanpackage++;
		}
		else
		{
			D_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
								  pleftbottom[0], pleftbottom[1]);

			d_pdestbasestep = screenwidth + ubasestep;
			d_pdestextrastep = d_pdestbasestep + 1;

	#if	id386broken
			d_pzbasestep = (d_zwidth + ubasestep) << 1;
			d_pzextrastep = d_pzbasestep + 2;
	#else
			d_pzbasestep = d_zwidth + ubasestep;
			d_pzextrastep = d_pzbasestep + 1;
	#endif

			if (ubasestep < 0){
				working_lstepx = r_lstepx - 1;
				working_lrstepx = r_lrstepx - 1;
				working_lgstepx = r_lgstepx - 1;
				working_lbstepx = r_lbstepx - 1;
			}
			else{

				working_lstepx = r_lstepx;
				working_lrstepx = r_lrstepx;
				working_lgstepx = r_lgstepx;
				working_lbstepx = r_lbstepx;
			}

			d_countextrastep = ubasestep + 1;
			d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
					((r_tstepy + r_tstepx * ubasestep) >> 16) *
					r_affinetridesc.skinwidth;
	#if	id386broken
			d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) << 16;
			d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) << 16;
	#else
			d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
			d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
	#endif
			d_lightbasestep = r_lstepy + working_lstepx * ubasestep;

			d_lightbasestepr = r_lrstepy + working_lrstepx * ubasestep;
			d_lightbasestepg = r_lgstepy + working_lgstepx * ubasestep;
			d_lightbasestepb = r_lbstepy + working_lbstepx * ubasestep;

			d_zibasestep = r_zistepy + r_zistepx * ubasestep;

			d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
					((r_tstepy + r_tstepx * d_countextrastep) >> 16) *
					r_affinetridesc.skinwidth;
	#if	id386broken
			d_sfracextrastep = ((r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF)<<16;
			d_tfracextrastep = ((r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF)<<16;
	#else
			d_sfracextrastep = (r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF;
			d_tfracextrastep = (r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF;
	#endif
			d_lightextrastep = d_lightbasestep + working_lstepx;

			d_lightextrastepr = d_lightbasestepr + working_lrstepx;
			d_lightextrastepg = d_lightbasestepg + working_lgstepx;
			d_lightextrastepb = d_lightbasestepb + working_lbstepx;

			d_ziextrastep = d_zibasestep + r_zistepx;

			D_PolysetScanLeftEdge (height);
		}
	}

// scan out the top (and possibly only) part of the right edge, updating the
// count field
	d_pedgespanpackage = a_spans;

	D_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
						  prightbottom[0], prightbottom[1]);
	d_aspancount = 0;
	d_countextrastep = ubasestep + 1;
	originalcount = a_spans[initialrightheight].count;
	a_spans[initialrightheight].count = -999999; // mark end of the spanpackages
#ifdef dithermodelhack
if (coloredlights == 2)
	D_PolysetDrawSpans8_C (a_spans);
else
#endif

if (coloredlights)
if (r_filter->value)
	D_PolysetDrawSpans8_C (a_spans);
else
	D_PolysetDrawSpans8_C (a_spans);
else
	D_PolysetDrawSpans8_C (a_spans);
	//D_PolysetDrawSpans8 (a_spans);

// scan out the bottom part of the right edge, if it exists
	if (pedgetable->numrightedges == 2)
	{
		int				height;
		spanpackage_t	*pstart;

		pstart = a_spans + initialrightheight;
		pstart->count = originalcount;

		d_aspancount = prightbottom[0] - prighttop[0];

		prighttop = prightbottom;
		prightbottom = pedgetable->prightedgevert2;

		height = prightbottom[1] - prighttop[1];

		D_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
							  prightbottom[0], prightbottom[1]);

		d_countextrastep = ubasestep + 1;
		a_spans[initialrightheight + height].count = -999999;
											// mark end of the spanpackages
#ifdef dithermodelhack
if (coloredlights == 2)
		D_PolysetDrawSpans8_C (pstart);
else
#endif
if (coloredlights)
	if (r_filter->value)
		D_PolysetDrawSpans8_C (pstart);
		else
		D_PolysetDrawSpans8_C (pstart);

else
		D_PolysetDrawSpans8_C (pstart);

	}
}
/*
================
D_RasterizeAliasPolySmoothFilter
================
*/

void D_RasterizeAliasPolySmoothFilter (void)
{
	int				initialleftheight, initialrightheight;
	int				*plefttop, *prighttop, *pleftbottom, *prightbottom;
	int				working_lstepx, originalcount;
	int				working_lrstepx;
	int				working_lgstepx;
	int				working_lbstepx;

	plefttop = pedgetable->pleftedgevert0;
	prighttop = pedgetable->prightedgevert0;

	pleftbottom = pedgetable->pleftedgevert1;
	prightbottom = pedgetable->prightedgevert1;

	initialleftheight = pleftbottom[1] - plefttop[1];
	initialrightheight = prightbottom[1] - prighttop[1];

//
// set the s, t, and light gradients, which are consistent across the triangle
// because being a triangle, things are affine
//
	D_PolysetCalcGradients (r_affinetridesc.skinwidth);

//
// rasterize the polygon
//

//
// scan out the top (and possibly only) part of the left edge
//
	d_pedgespanpackage = a_spans;

	ystart = plefttop[1];
	d_aspancount = plefttop[0] - prighttop[0];

	d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> 16) + (plefttop[3] >> 16) * r_affinetridesc.skinwidth;
	d_sfrac = plefttop[2] & 0xFFFF;
	d_tfrac = plefttop[3] & 0xFFFF;
	d_light = plefttop[4];
	d_zi = plefttop[5];
	d_lightrgb[0] = plefttop[6];
	d_lightrgb[1] = plefttop[7];
	d_lightrgb[2] = plefttop[8];
	d_pdest = (byte *)d_viewbuffer +
			ystart * screenwidth + plefttop[0];
	d_pz = d_pzbuffer + ystart * d_zwidth + plefttop[0];

	if (initialleftheight == 1)
	{
		d_pedgespanpackage->pdest = d_pdest;
		d_pedgespanpackage->pz = d_pz;
		d_pedgespanpackage->count = d_aspancount;
		d_pedgespanpackage->ptex = d_ptex;

		d_pedgespanpackage->sfrac = d_sfrac;
		d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?
		d_pedgespanpackage->light = d_light;

		d_pedgespanpackage->zi = d_zi;
// leilei - colored lights on models
		if (coloredlights){
			d_pedgespanpackage->lightr = d_lightrgb[0];
			d_pedgespanpackage->lightg = d_lightrgb[1];
			d_pedgespanpackage->lightb = d_lightrgb[2];
			}

		d_pedgespanpackage++;
	}
	else
	{
		D_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
							  pleftbottom[0], pleftbottom[1]);

		d_pzbasestep = d_zwidth + ubasestep;
		d_pzextrastep = d_pzbasestep + 1;

		d_pdestbasestep = screenwidth + ubasestep;
		d_pdestextrastep = d_pdestbasestep + 1;

	// TODO: can reuse partial expressions here

	// for negative steps in x along left edge, bias toward overflow rather than
	// underflow (sort of turning the floor () we did in the gradient calcs into
	// ceil (), but plus a little bit)
		if (ubasestep < 0){
			working_lstepx = r_lstepx - 1;
			working_lrstepx = r_lrstepx - 1;
			working_lgstepx = r_lgstepx - 1;
			working_lbstepx = r_lbstepx - 1;
		}
		else
		{
			working_lstepx = r_lstepx;
			working_lrstepx = r_lrstepx;
			working_lgstepx = r_lgstepx;
			working_lbstepx = r_lbstepx;


		}

		d_countextrastep = ubasestep + 1;
		d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
				((r_tstepy + r_tstepx * ubasestep) >> 16) *
				r_affinetridesc.skinwidth;
		d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
		d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;

		d_lightbasestep = r_lstepy + working_lstepx * ubasestep;

		d_zibasestep = r_zistepy + r_zistepx * ubasestep;

		d_lightbasestepr = r_lrstepy + working_lrstepx * ubasestep;
		d_lightbasestepg = r_lgstepy + working_lgstepx * ubasestep;
		d_lightbasestepb = r_lbstepy + working_lbstepx * ubasestep;



		d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
				((r_tstepy + r_tstepx * d_countextrastep) >> 16) *
				r_affinetridesc.skinwidth;
		d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) & 0xFFFF;
		d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) & 0xFFFF;

		d_lightextrastep = d_lightbasestep + working_lstepx;

		d_ziextrastep = d_zibasestep + r_zistepx;

		d_lightextrastepr = d_lightbasestepr + working_lrstepx;
		d_lightextrastepg = d_lightbasestepg + working_lgstepx;
		d_lightextrastepb = d_lightbasestepb + working_lbstepx;

		D_PolysetScanLeftEdge (initialleftheight);
	}

//
// scan out the bottom part of the left edge, if it exists
//
	if (pedgetable->numleftedges == 2)
	{
		int		height;

		plefttop = pleftbottom;
		pleftbottom = pedgetable->pleftedgevert2;

		height = pleftbottom[1] - plefttop[1];

// TODO: make this a function; modularize this function in general

		ystart = plefttop[1];
		d_aspancount = plefttop[0] - prighttop[0];
		d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> 16) +
				(plefttop[3] >> 16) * r_affinetridesc.skinwidth;
		d_sfrac = 0;
		d_tfrac = 0;
		d_light = plefttop[4];
		d_zi = plefttop[5];
		d_lightrgb[0] = plefttop[6];
		d_lightrgb[1] = plefttop[7];
		d_lightrgb[2] = plefttop[8];
		d_pdest = (byte *)d_viewbuffer + ystart * screenwidth + plefttop[0];
		d_pz = d_pzbuffer + ystart * d_zwidth + plefttop[0];

		if (height == 1)
		{
			d_pedgespanpackage->pdest = d_pdest;
			d_pedgespanpackage->pz = d_pz;
			d_pedgespanpackage->count = d_aspancount;
			d_pedgespanpackage->ptex = d_ptex;

			d_pedgespanpackage->sfrac = d_sfrac;
			d_pedgespanpackage->tfrac = d_tfrac;

		// FIXME: need to clamp l, s, t, at both ends?


			d_pedgespanpackage->light = d_light;
			d_pedgespanpackage->zi = d_zi;
// leilei - colored lights on models
		if (coloredlights){
			d_pedgespanpackage->lightr = d_lightrgb[0];
			d_pedgespanpackage->lightg = d_lightrgb[1];
			d_pedgespanpackage->lightb = d_lightrgb[2];
			}



			d_pedgespanpackage++;
		}
		else
		{
			D_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
								  pleftbottom[0], pleftbottom[1]);

			d_pdestbasestep = screenwidth + ubasestep;
			d_pdestextrastep = d_pdestbasestep + 1;

			d_pzbasestep = d_zwidth + ubasestep;
			d_pzextrastep = d_pzbasestep + 1;


			if (ubasestep < 0){
				working_lstepx = r_lstepx - 1;
				working_lrstepx = r_lrstepx - 1;
				working_lgstepx = r_lgstepx - 1;
				working_lbstepx = r_lbstepx - 1;
			}
			else{

				working_lstepx = r_lstepx;
				working_lrstepx = r_lrstepx;
				working_lgstepx = r_lgstepx;
				working_lbstepx = r_lbstepx;
			}

			d_countextrastep = ubasestep + 1;
			d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
					((r_tstepy + r_tstepx * ubasestep) >> 16) *
					r_affinetridesc.skinwidth;
			d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
			d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;

			d_lightbasestep = r_lstepy + working_lstepx * ubasestep;

			d_lightbasestepr = r_lrstepy + working_lrstepx * ubasestep;
			d_lightbasestepg = r_lgstepy + working_lgstepx * ubasestep;
			d_lightbasestepb = r_lbstepy + working_lbstepx * ubasestep;

			d_zibasestep = r_zistepy + r_zistepx * ubasestep;

			d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
					((r_tstepy + r_tstepx * d_countextrastep) >> 16) *
					r_affinetridesc.skinwidth;
			d_sfracextrastep = (r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF;
			d_tfracextrastep = (r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF;
	
			d_lightextrastep = d_lightbasestep + working_lstepx;

			d_lightextrastepr = d_lightbasestepr + working_lrstepx;
			d_lightextrastepg = d_lightbasestepg + working_lgstepx;
			d_lightextrastepb = d_lightbasestepb + working_lbstepx;

			d_ziextrastep = d_zibasestep + r_zistepx;

			D_PolysetScanLeftEdge (height);
		}
	}

// scan out the top (and possibly only) part of the right edge, updating the
// count field
	d_pedgespanpackage = a_spans;

	D_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
						  prightbottom[0], prightbottom[1]);
	d_aspancount = 0;
	d_countextrastep = ubasestep + 1;
	originalcount = a_spans[initialrightheight].count;
	a_spans[initialrightheight].count = -999999; // mark end of the spanpackages
if (coloredlights)
		D_PolysetDrawSpans8_C (a_spans);
else
		D_PolysetDrawSpans8_C (a_spans);
	//D_PolysetDrawSpans8 (a_spans);

// scan out the bottom part of the right edge, if it exists
	if (pedgetable->numrightedges == 2)
	{
		int				height;
		spanpackage_t	*pstart;

		pstart = a_spans + initialrightheight;
		pstart->count = originalcount;

		d_aspancount = prightbottom[0] - prighttop[0];

		prighttop = prightbottom;
		prightbottom = pedgetable->prightedgevert2;

		height = prightbottom[1] - prighttop[1];

		D_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
							  prightbottom[0], prightbottom[1]);

		d_countextrastep = ubasestep + 1;
		a_spans[initialrightheight + height].count = -999999;
											// mark end of the spanpackages
if (coloredlights)
		D_PolysetDrawSpans8_C (pstart);

else
		D_PolysetDrawSpans8_C (pstart);

//		D_PolysetDrawSpans8_C (pstart);
//		D_PolysetDrawSpans8_C (pstart);
	}
}


/*
================
D_PolysetSetEdgeTable
================
*/
void D_PolysetSetEdgeTable (void)
{
	int			edgetableindex;

	edgetableindex = 0;	// assume the vertices are already in
						//  top to bottom order

//
// determine which edges are right & left, and the order in which
// to rasterize them
//
	if (r_p0[1] >= r_p1[1])
	{
		if (r_p0[1] == r_p1[1])
		{
			if (r_p0[1] < r_p2[1])
				pedgetable = &edgetables[2];
			else
				pedgetable = &edgetables[5];

			return;
		}
		else
		{
			edgetableindex = 1;
		}
	}

	if (r_p0[1] == r_p2[1])
	{
		if (edgetableindex)
			pedgetable = &edgetables[8];
		else
			pedgetable = &edgetables[9];

		return;
	}
	else if (r_p1[1] == r_p2[1])
	{
		if (edgetableindex)
			pedgetable = &edgetables[10];
		else
			pedgetable = &edgetables[11];

		return;
	}

	if (r_p0[1] > r_p2[1])
		edgetableindex += 2;

	if (r_p1[1] > r_p2[1])
		edgetableindex += 4;

	pedgetable = &edgetables[edgetableindex];
}



void D_PolysetRecursiveDrawLine (int *lp1, int *lp2)
{
	int		d;
	int		new[6];
	int 	ofs;

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;

	return;	// line is completed

split:
// split this edge
	new[0] = (lp1[0] + lp2[0]) >> 1;
	new[1] = (lp1[1] + lp2[1]) >> 1;
	new[5] = (lp1[5] + lp2[5]) >> 1;
	new[2] = (lp1[2] + lp2[2]) >> 1;
	new[3] = (lp1[3] + lp2[3]) >> 1;
	new[4] = (lp1[4] + lp2[4]) >> 1;

// draw the point
	ofs = d_scantable[new[1]] + new[0];
	if (new[5] > d_pzbuffer[ofs])
	{
		int		pix;

		d_pzbuffer[ofs] = new[5];
		pix = skintable[new[3]>>16][new[2]>>16];
//		pix = ((byte *)acolormap)[pix + (new[4] & 0xFF00)];
		d_viewbuffer[ofs] = pix;
	}

// recursively continue
	D_PolysetRecursiveDrawLine (lp1, new);
	D_PolysetRecursiveDrawLine (new, lp2);
}
/*
void D_PolysetRecursiveTriangle2 (int *lp1, int *lp2, int *lp3)
{
	int		d;
	int		new[4];

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;
	return;

split:
// split this edge
	new[0] = (lp1[0] + lp2[0]) >> 1;
	new[1] = (lp1[1] + lp2[1]) >> 1;
	new[5] = (lp1[5] + lp2[5]) >> 1;
	new[2] = (lp1[2] + lp2[2]) >> 1;
	new[3] = (lp1[3] + lp2[3]) >> 1;
	new[4] = (lp1[4] + lp2[4]) >> 1;



	D_PolysetRecursiveDrawLine (new, lp3);

// recursively continue
	D_PolysetRecursiveTriangle (lp1, new, lp3);
	D_PolysetRecursiveTriangle (new, lp2, lp3);
}
*/



