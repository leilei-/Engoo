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
// d_sprite.c: software top-level rasterization driver module for drawing
// spritesy

#include "quakedef.h"
#include "d_local.h"

static int		sprite_height;
static int		minindex, maxindex;
static sspan_t	*sprite_spans;
extern particle_t		*currentparticle;
extern flare_t		*currentflare;
extern byte	additiveshade[16384];
#if	!id386

// yeah,we sitll have the asm codes still used in there :)


#endif
extern byte	*host_colormap_nofb;
extern byte gelmap[256];
extern pixel_t addTable[256][256];		// Additive Blending
extern pixel_t mulTable[256][256];	 // Color Mod!
extern pixel_t transTable[256][256];	 
//extern pixel_t alphaTable[256][256];	 
extern cvar_t *r_filter;
extern byte menumap[256][16];			// haha hack
//	=!=!=!=!=!=!=!=
// ASMME (or extern the blending mode stuff to a new function)
//	=!=!=!=!=!=!=!=
float	alfer;
float	scaalx;
float	scaaly;
int		flared;	// draw through edges
int		flarespan;	// the span we need to know before we determine
int		pacolor;	
float	lensreflection = 1;	// hack

//#define		DITHERALPHATEST


/*
=====================
D_SpriteDrawSpans_C
=====================
*/
void D_SpriteDrawSpans_C (sspan_t *pspan)
{
	int			count, spancount, izistep;
	int			izi;
	byte		*pbase, *pdest;
	fixed16_t	s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	byte		btemp;
	short		*pz;
	int			forg;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (byte *)d_viewbuffer + (screenwidth * pspan->v) + pspan->u;

		pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin+ dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin+ dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
	// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
		// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}
			if (foguse){ forg = (float)z / 1024;	if (forg > 32762)	forg = 32762; } // leilei - fog
			do
			{
				btemp = *(pbase + (s >> 16) + (t >> 16)	* cachewidth);
				if (foguse)
					btemp = host_fogmap[btemp  + (forg >> fogrange & 0xFF00)];

				
				if (btemp != 255)
				{
					*pdest = btemp;
					
					*pz = izi >> 16;
					
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}
extern	cvar_t *temp2;
extern	cvar_t *temp3;
void D_SpriteDrawSpans_Blend_C (sspan_t *pspan, int blendmode)
{
	int			count, spancount, izistep;
	int			izi;
	byte		*pbase, *pdest;
	fixed16_t	s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	byte		btemp;

	short		*pz;
	int			addlpha, dalpha;
	int			forg;

#ifdef DITHERALPHATEST
	int			ehh, heh;	// leilei - dithered alpha fade
#endif
	dalpha = 8192 * (alfer * -1) + 16384;
	
	
	
	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);
#ifdef DITHERALPHATEST
	ehh = 256;
#endif
	do
	{
		pdest = (byte *)d_viewbuffer + (screenwidth * pspan->v) + pspan->u;

		pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin+ dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin+ dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
	// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
		// calculate s and t at the far end of the span
			if (count >= 8){
				spancount = 8;
			}
			else
				spancount = count;

			count -= spancount;
			

			if (count)
			{

			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
#ifdef DITHERALPHATEST
				ehh = 256;
#endif
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
#ifdef DITHERALPHATEST
				ehh = -256;
#endif
				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}
			if (foguse){ forg = (float)z / 1024;	if (forg > 32762)	forg = 32762; } // leilei - fog
			do
			{
				btemp = *(pbase + (s >> 16) + (t >> 16)	* cachewidth);
#ifdef DITHERALPHATEST
				ehh *= -1;

				addlpha = dalpha + ehh;
				if (addlpha < 0)
					addlpha = 0; // don't overgo it
#else
				addlpha = dalpha;
#endif

			//	-------------------------------------------------
			// Big FAT BLENDING ROUTINES Begin
			//	-------------------------------------------------

				if (btemp != 255)
				{
				if (blendmode == 3)
					btemp = pacolor;
				if (flared){
					btemp = mulTable[btemp][pacolor];
				//		btemp = menumap[btemp][3];
					if (reflectpass)	{
					if (alfer < 1)
								*pdest = addTable[host_colormap[btemp + (addlpha & 0xFF00)]][*pdest]; 
								else
								*pdest = addTable[btemp][*pdest];
											*pz = izi >> 16;
					}
					else{
						if (alfer < 1)
									*pdest = addTable[*pdest][host_colormap[btemp + (addlpha & 0xFF00)]]; 
									else
									*pdest = addTable[btemp][*pdest];
											*pz = izi >> 16;
					}
						
						
				}
				else	if (*pz <= (izi >> 16))
					{
						
						//*pdest = btemp;
						if (blendmode == 1){
							if (alfer < 1)
								*pdest = addTable[host_colormap[btemp + (addlpha & 0xFF00)]][*pdest]; 
								else
								*pdest = addTable[btemp][*pdest];
						}
						// Variable Alpha Blend
						else if (blendmode == 10){
							btemp = btemp;
							if (addlpha > 10900){
								// off.
							}
							else if (addlpha > 10000){
								
								*pdest = transTable[btemp][*pdest]; 
			
							}
							else if (addlpha > 8700){
								*pdest = transTable[*pdest][btemp]; 
								
							}
							else
								*pdest = btemp;
							
						}
						else if (blendmode == 8){
							btemp = mulTable[btemp][pacolor];
							if (alfer < 1)
								*pdest = addTable[host_colormap[btemp + (addlpha & 0xFF00)]][*pdest]; 
								else
								*pdest = addTable[btemp][*pdest];
						}
						else if (blendmode == 4){
							*pdest = menumap[*pdest][btemp];
						}

						else if (blendmode == 6){
												
							if (foguse)
								*pdest = host_fogmap[mulTable[btemp][*pdest] + (forg >> fogrange & 0xFF00)];
							else
								*pdest = mulTable[btemp][*pdest];
							
						}
						else
									
						{
							*pdest = btemp;

							*pz = izi >> 16;
						}
					}
			//	-------------------------------------------------
			// Big FAT BLENDING ROUTINES End
			//	-------------------------------------------------

				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;
	
	} while (pspan->count != DS_SPAN_LIST_END);
}


extern int kernel[2][2][2];

void D_SpriteDrawSpans_C_Filter (sspan_t *pspan)
{
	int			count, spancount, izistep;
	int			izi;
	byte		*pbase, *pdest;
	fixed16_t	s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	byte		btemp;
	short		*pz;
	int			addlpha;
	int			forg;
	addlpha = 8192 * (alfer * -1) + 16384;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (byte *)d_viewbuffer + (screenwidth * pspan->v) + pspan->u;

		pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin+ dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin+ dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
	// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
		// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}
			if (foguse){ forg = (float)z / 1024;	if (forg > 32762)	forg = 32762; } // leilei - fog
			do
			{
				int idiths = s;
				int iditht = t;

				int X = (pspan->u + spancount) & 1;
				int Y = (pspan->v)&1;

				//Using the kernel
					idiths += kernel[X][Y][0];
					iditht += kernel[X][Y][1];
					idiths = idiths >> 16;
					idiths = idiths ? idiths -1 : idiths;
					iditht = iditht >> 16;
					iditht = iditht ? iditht -1 : iditht;
				btemp = *(pbase + + idiths + iditht * cachewidth);
				if (*pz <= (izi >> 16))
				if (btemp != 255)
				{
				if (foguse)
					*pdest = host_fogmap[btemp  + (forg >> fogrange & 0xFF00)];
				else
					*pdest = btemp;
					*pz = izi >> 16;
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}



void D_SpriteDrawSpans_Blend_C_Filter (sspan_t *pspan, int blendmode)
{
	int			count, spancount, izistep;
	int			izi;
	byte		*pbase, *pdest;
	fixed16_t	s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	byte		btemp;
	byte		ctemp;	
	short		*pz;
	int			forg;
	int			addlpha;

	addlpha = 8192 * (alfer * -1) + 16384;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (byte *)d_viewbuffer + (screenwidth * pspan->v) + pspan->u;

		pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin+ dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin+ dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
	// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;
		if (foguse){ forg = (float)z / 1024;	if (forg > 32762)	forg = 32762; } // leilei - fog
		do
		{
		// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

			do
			{
				int idiths = s;
				int iditht = t;

				int X = (pspan->u + spancount) & 1;
				int Y = (pspan->v)&1;

				//Using the kernel
					idiths += kernel[X][Y][0];
					iditht += kernel[X][Y][1];
					idiths = idiths >> 16;
					idiths = idiths ? idiths -1 : idiths;
					iditht = iditht >> 16;
					iditht = iditht ? iditht -1 : iditht;
				btemp = *(pbase + + idiths + iditht * cachewidth);
			//	btemp = *(pbase + (s >> 16) + (t >> 16)	* cachewidth);

			//	-------------------------------------------------
			// Big FAT BLENDING ROUTINES Begin
			//	-------------------------------------------------
	
				if (btemp != 255)
				{
				if (blendmode == 3)
					btemp = pacolor;
				if (flared){
					btemp = mulTable[btemp][pacolor];
						//*btemp = menumap[*btemp][color];
					if (reflectpass)	{
					if (alfer < 1)
								*pdest = addTable[host_colormap[btemp + (addlpha & 0xFF00)]][*pdest]; 
								else
								*pdest = addTable[btemp][*pdest];
											*pz = izi >> 16;
					}
					else{
						if (alfer < 1)
									*pdest = addTable[host_colormap[btemp + (addlpha & 0xFF00)]][*pdest]; 
									else
									*pdest = addTable[btemp][*pdest];
											*pz = izi >> 16;
					}
						
						
				}
				else	if (*pz <= (izi >> 16))
					{
						
						//*pdest = btemp;
						if (blendmode == 1){
							if (alfer < 1)
								*pdest = addTable[host_colormap[btemp + (addlpha & 0xFF00)]][*pdest]; 
								else
								*pdest = addTable[btemp][*pdest];
						}
												// Variable Alpha Blend
						else if (blendmode == 10){
							btemp = btemp;
							if (addlpha > 10900){
								// off.
							}
							else if (addlpha > 10000){
								
								*pdest = transTable[btemp][*pdest]; 
			
							}
							else if (addlpha > 8700){
								*pdest = transTable[*pdest][btemp]; 
								
							}
							else
								*pdest = btemp;
							
						}
						else if (blendmode == 8){
							btemp = mulTable[btemp][pacolor];
							if (alfer < 1)
								*pdest = addTable[host_colormap[btemp + (addlpha & 0xFF00)]][*pdest]; 
								else
								*pdest = addTable[btemp][*pdest];
						}
						else if (blendmode == 4){
							*pdest = menumap[*pdest][btemp];
						}

						else if (blendmode == 6){
							
							if (foguse)
								*pdest = host_fogmap[mulTable[btemp][*pdest] + (forg >> fogrange & 0xFF00)];
							else
								*pdest = mulTable[btemp][*pdest];
						}
						else
									
						{
							*pdest = btemp;
							
							*pz = izi >> 16;
						}
					}
			//	-------------------------------------------------
			// Big FAT BLENDING ROUTINES End
			//	-------------------------------------------------

					
				}


				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}


void D_SpriteDrawSpans_Censor (sspan_t *pspan)
{
	int			count, spancount, izistep;
	int			izi;
	byte		*pbase, *pdest;
	fixed16_t	s, t, snext, tnext, sstep, tstep;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float		sdivz8stepu, tdivz8stepu, zi8stepu;
	byte		btemp;
	short		*pz;
	int			forg;
	int		censwidth = 8;
	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = (byte *)d_viewbuffer + (screenwidth * pspan->v) + pspan->u;

		pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

		if (count <= 0)
			goto NextSpan;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin+ dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin+ dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
	// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;

		do
		{
		// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}
			do
			{
				btemp = (pdest[spancount]);
				
				if (btemp != 255)
				{
					*pdest = btemp;
					
					*pz = izi >> 16;
					
				}

				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

NextSpan:
		pspan++;

	} while (pspan->count != DS_SPAN_LIST_END);
}

/*
=====================
D_SpriteScanLeftEdge
=====================
*/
void D_SpriteScanLeftEdge (void)
{
	int			i, v, itop, ibottom, lmaxindex;
	emitpoint_t	*pvert, *pnext;
	sspan_t		*pspan;
	float		du, dv, vtop, vbottom, slope;
	fixed16_t	u, u_step;

	pspan = sprite_spans;
	i = minindex;
	if (i == 0)
		i = r_spritedesc.nump;

	lmaxindex = maxindex;
	if (lmaxindex == 0)
		lmaxindex = r_spritedesc.nump;

	vtop = ceil (r_spritedesc.pverts[i].v);

	do
	{
		pvert = &r_spritedesc.pverts[i];
		pnext = pvert - 1;

		vbottom = ceil (pnext->v);

		if (vtop < vbottom)
		{
			du = pnext->u - pvert->u;
			dv = pnext->v - pvert->v;
			slope = du / dv;
			u_step = (int)(slope * 0x10000);
		// adjust u to ceil the integer portion
			u = (int)((pvert->u + (slope * (vtop - pvert->v))) * 0x10000) +
					(0x10000 - 1);
			itop = (int)vtop;
			ibottom = (int)vbottom;

			for (v=itop ; v<ibottom ; v++)
			{
				pspan->u = u >> 16;
				pspan->v = v;
				u += u_step;
				pspan++;
			}
		}

		vtop = vbottom;

		i--;
		if (i == 0)
			i = r_spritedesc.nump;

	} while (i != lmaxindex);
}


/*
=====================
D_SpriteScanRightEdge
=====================
*/
void D_SpriteScanRightEdge (void)
{
	int			i, v, itop, ibottom;
	emitpoint_t	*pvert, *pnext;
	sspan_t		*pspan;
	float		du, dv, vtop, vbottom, slope, uvert, unext, vvert, vnext;
	fixed16_t	u, u_step;

	pspan = sprite_spans;
	i = minindex;

	vvert = r_spritedesc.pverts[i].v;
	if (vvert < r_refdef.fvrecty_adj)
		vvert = r_refdef.fvrecty_adj;
	if (vvert > r_refdef.fvrectbottom_adj)
		vvert = r_refdef.fvrectbottom_adj;

	vtop = ceil (vvert);

	do
	{
		pvert = &r_spritedesc.pverts[i];
		pnext = pvert + 1;

		vnext = pnext->v;
		if (vnext < r_refdef.fvrecty_adj)
			vnext = r_refdef.fvrecty_adj;
		if (vnext > r_refdef.fvrectbottom_adj)
			vnext = r_refdef.fvrectbottom_adj;

		vbottom = ceil (vnext);

		if (vtop < vbottom)
		{
			uvert = pvert->u;
			if (uvert < r_refdef.fvrectx_adj)
				uvert = r_refdef.fvrectx_adj;
			if (uvert > r_refdef.fvrectright_adj)
				uvert = r_refdef.fvrectright_adj;

			unext = pnext->u;
			if (unext < r_refdef.fvrectx_adj)
				unext = r_refdef.fvrectx_adj;
			if (unext > r_refdef.fvrectright_adj)
				unext = r_refdef.fvrectright_adj;

			du = unext - uvert;
			dv = vnext - vvert;
			slope = du / dv;
			u_step = (int)(slope * 0x10000);
		// adjust u to ceil the integer portion
			u = (int)((uvert + (slope * (vtop - vvert))) * 0x10000) +
					(0x10000 - 1);
			itop = (int)vtop;
			ibottom = (int)vbottom;

			for (v=itop ; v<ibottom ; v++)
			{
				pspan->count = (u >> 16) - pspan->u;
				u += u_step;
				pspan++;
			}
		}

		vtop = vbottom;
		vvert = vnext;

		i++;
		if (i == r_spritedesc.nump)
			i = 0;

	} while (i != maxindex);

	pspan->count = DS_SPAN_LIST_END;	// mark the end of the span list
}
extern cvar_t *temp2;
extern cvar_t *temp3;

/*
=====================
D_SpriteCalculateGradients
=====================
*/
void D_SpriteCalculateGradients (void)
{
	vec3_t		p_normal, p_saxis, p_taxis, p_temp1, p_temp2;
	float		distinv;
	float	scel, scely;
	int	xcanter, ycanter;
	TransformVector (r_spritedesc.vpn, p_normal);
	TransformVector (r_spritedesc.vright, p_saxis);
	TransformVector (r_spritedesc.vup, p_taxis);
	VectorInverse (p_taxis);
	
	distinv = 1.0 / (-DotProduct (modelorg, r_spritedesc.vpn));
	//lensreflection = -2;
	// lame
	xcanter = vid.width - xcenter;
	ycanter = vid.height - ycenter;

	//if (currentparticle){
		scel = scaalx;
		scely = scaaly;
	//scel = currententity->scale;

	if (scaalx > 665)
		scel = scaalx -= 665; // leilei - flare hack

//	if (reflectpass)
//		scely *= 4;	// leilei - try to make the water flares long

	// Manoel Kasimier - QC Scale - begin



	p_saxis[0] /= scel;
	p_saxis[1] /= scel;
	p_saxis[2] /= scel;

	p_taxis[0] /= scely;
	p_taxis[1] /= scely;
	p_taxis[2] /= scely;


	

	d_sdivzstepu = p_saxis[0] * xscaleinv;
	d_tdivzstepu = p_taxis[0] * xscaleinv;

	d_sdivzstepv = -p_saxis[1] * yscaleinv;
	d_tdivzstepv = -p_taxis[1] * yscaleinv;

	d_zistepu = p_normal[0] * xscaleinv * distinv;
	d_zistepv = -p_normal[1] * yscaleinv * distinv;
	
	d_sdivzorigin = p_saxis[2] - xcenter * d_sdivzstepu -
			ycenter * d_sdivzstepv;
	d_tdivzorigin = p_taxis[2] - xcenter * d_tdivzstepu -
			ycenter * d_tdivzstepv;
	d_ziorigin = p_normal[2] * distinv - xcenter * d_zistepu -
			ycenter * d_zistepv;

	//d_sdivzorigin *= lensreflection;
	
	

	TransformVector (modelorg, p_temp1);
	
	// leilei - lens flare reflections (if it even works :/ )
	//p_temp1[0] *= lensreflection;
	//p_temp1[1] *= lensreflection;


	sadjust = ((fixed16_t)(DotProduct (p_temp1, p_saxis) * 0x10000 + 0.5)) -
			(-(cachewidth >> 1) << 16);
	tadjust = ((fixed16_t)(DotProduct (p_temp1, p_taxis) * 0x10000 + 0.5)) -
			(-(sprite_height >> 1) << 16);


		



// -1 (-epsilon) so we never wander off the edge of the texture
	bbextents = (cachewidth << 16) - 1;
	bbextentt = (sprite_height << 16) - 1;
}
extern cvar_t *temp2;

/*
=====================
D_DrawSprite
=====================
*/
void D_DrawSprite (int	isthisaparticle)
{
	int			i, nump;
	float		ymin, ymax;
	int isfading;	
	int	bypass;
	emitpoint_t	*pverts;
	sspan_t		spans[MAXHEIGHT+1];
	
	sprite_spans = spans;

	if (!isthisaparticle){
	//	scaalx = currententity->scale;
	//	scaaly = currententity->scale;
		scaalx = 1; // no qc scale yet
		scaaly = 1; // no qc scale yet
	}
	else if (isthisaparticle == 2) // it's a flare 
	{
		scaalx = currentflare->scale;
		scaaly = currentflare->scaley;
	}
	else			// ok it is a particle
	{
		scaalx = currentparticle->scale;
		scaaly = currentparticle->scaley;
		if (currentparticle->alphavel)
			isfading = 1;
		else
			isfading = 0;
		


	}




// find the top and bottom vertices, and make sure there's at least one scan to
// draw
	ymin = 999999.9;
	ymax = -999999.9;
	pverts = r_spritedesc.pverts;

	for (i=0 ; i<r_spritedesc.nump ; i++)
	{
		if (pverts->v < ymin)
		{
			ymin = pverts->v;
			minindex = i;
		}

		if (pverts->v > ymax)
		{
			ymax = pverts->v;
			maxindex = i;
		}

		pverts++;
	}

	ymin = ceil (ymin);
	ymax = ceil (ymax);

	if (ymin >= ymax)
		return;		// doesn't cross any scans at all

	cachewidth = r_spritedesc.pspriteframe->width;
	sprite_height = r_spritedesc.pspriteframe->height;
	cacheblock = (byte *)&r_spritedesc.pspriteframe->pixels[0];


// copy the first vertex to the last vertex, so we don't have to deal with
// wrapping
	nump = r_spritedesc.nump;
	pverts = r_spritedesc.pverts;
	pverts[nump] = pverts[0];
	
/*
	if(currentparticle->sprtype == 15){ flared = 1; currentparticle->sprtype = 5;			lensreflection = -3.5;}
	else	if(currentparticle->sprtype == 16){ flared = 1; currentparticle->sprtype = 5;	lensreflection = -2;}
	else	if(currentparticle->sprtype == 17){ flared = 1; currentparticle->sprtype = 5;	lensreflection = -1.5;}
	else	if(currentparticle->sprtype == 18){ flared = 1; currentparticle->sprtype = 5;	lensreflection = -1.1;}
	else	if(currentparticle->sprtype == 19){ flared = 1; currentparticle->sprtype = 5;	lensreflection = -0.7;}
	else	if(currentparticle->sprtype == 20){ flared = 1; currentparticle->sprtype = 5;	lensreflection = -0.3;}
	else	if(currentparticle->sprtype == 21){ flared = 1; currentparticle->sprtype = 5;	lensreflection = 1.2;}
	else	if(currentparticle->sprtype == 22){ flared = 1; currentparticle->sprtype = 5;	lensreflection = 1.7;}
	else	if(currentparticle->sprtype == 23){ flared = 1; currentparticle->sprtype = 5;	lensreflection = 2;}
	else
	*/
	lensreflection = 3;
//if (flared)
//	currentflare->lensreflection = lensreflection;




	D_SpriteCalculateGradients ();

	// :( 

	D_SpriteScanLeftEdge ();
	D_SpriteScanRightEdge ();
	

	if (isthisaparticle){
		if (isthisaparticle == 2){
					alfer = currentflare->alpha;	
					pacolor = currentflare->color;
		}
		else if (isthisaparticle){
						alfer = currentparticle->alpha;	
						pacolor = currentparticle->color;
		}
	if (flared && alfer < 0.08f)
		return;	// try not to draw totally faded flares.

				if (alfer < 0.01f && !isfading)
					alfer = 1;
				 if (alfer < 0.005f && isfading)
					return;
				
				if (isthisaparticle == 2){
					if (r_filter->value)
							D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 5);
						else
							D_SpriteDrawSpans_Blend_C		 (sprite_spans, 5);
						return;
					}
					// only do one type for flares
				
	if (r_filter->value){	
	if (currentparticle->blend == 1)	// additeev
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 1);
		
	else if (currentparticle->blend == 4)	// gelmapped
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 4);
	else if (currentparticle->blend == 5)	// additive glow flare
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 5);
	else if (currentparticle->blend == 6)	// multiplicatev
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 6);
	else if (currentparticle->blend == 8)	// additive bt colorable like flare
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 8);
	else if (currentparticle->blend == 9)	// alpha bt colorable like flare
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 9);
	else if (currentparticle->blend == 10)	// alpha
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 10);
	else if (currentparticle->blend == 3)	// particle color only
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 3);
	else if (alfer < 1)
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 2);	
	else
	D_SpriteDrawSpans_C_Filter (sprite_spans);
	}
	else

	{
		

	if (currentparticle->blend == 1)
		D_SpriteDrawSpans_Blend_C (sprite_spans, 1);
	else if (currentparticle->blend == 4)	// gelmapped
		D_SpriteDrawSpans_Blend_C (sprite_spans, 4);
	else if (currentparticle->blend == 3)
		D_SpriteDrawSpans_Blend_C (sprite_spans, 3);
	else if (currentparticle->blend == 5)
		D_SpriteDrawSpans_Blend_C (sprite_spans, 5);
	else if (currentparticle->blend == 6)
		D_SpriteDrawSpans_Blend_C (sprite_spans, 6);
	else if (currentparticle->blend == 8)	// additive bt colorable like flare
		D_SpriteDrawSpans_Blend_C (sprite_spans, 8);
	else if (currentparticle->blend == 9)	// alpha bt colorable like flare
		D_SpriteDrawSpans_Blend_C (sprite_spans, 9);
	else if (currentparticle->blend == 10)	// alpha 
		D_SpriteDrawSpans_Blend_C (sprite_spans, 10);
	else if (alfer < 1)
		D_SpriteDrawSpans_Blend_C (sprite_spans, 2);	
	else
#if	!id386
		D_SpriteDrawSpans_C (sprite_spans);
#else
		D_SpriteDrawSpans (sprite_spans);
#endif

	}
	//return;
	}
	else
	
	
	{
		flared = 0;	// no it is not a flare. also these frys are not ok.
				alfer = currententity->alpha;	
				if (alfer < 0.01f)
					alfer = 1;
				else
					alfer = 0;
	if (r_filter->value){	
	
	if (currententity->effects & EF_ADDITIVE)
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 1);
	else if (alfer < 1)
		D_SpriteDrawSpans_Blend_C_Filter (sprite_spans, 1);	
	else
	D_SpriteDrawSpans_C_Filter (sprite_spans);
	}
	else

	{


	if (currententity->effects & EF_ADDITIVE)
		D_SpriteDrawSpans_Blend_C (sprite_spans, 1);
	else if (alfer < 1)
		D_SpriteDrawSpans_Blend_C (sprite_spans, 1);	
	else
#if	!id386
		D_SpriteDrawSpans_C (sprite_spans);
#else
		D_SpriteDrawSpans (sprite_spans);
#endif

	}
	}

}

