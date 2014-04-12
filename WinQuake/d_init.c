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
// d_init.c: rasterization driver initialization

#include "quakedef.h"
#include "d_local.h"

#define NUM_MIPS	4

cvar_t	*d_subdiv16;
cvar_t	*r_filter;
cvar_t	*d_mipcap;
cvar_t	*d_mipscale;
cvar_t	*d_mipdetail;
cvar_t	*r_wateralpha;

float	oldwateralpha;
float	newwateralpha; // leilei - dirty hack to see if our values changed
						// and if so, make a new translucency table!

surfcache_t		*d_initial_rover;
qboolean		d_roverwrapped;
int				d_minmip;
float			d_scalemip[NUM_MIPS-1];

static float	basemip[NUM_MIPS-1] = {1.0, 0.5*0.8, 0.25*0.8};

extern int			d_aflatcolor;

void (*d_drawspans) (espan_t *pspan);
void (*d_fogspans) (espan_t *pspan);		// leilei - alternate fogging method (to get along with the asm spans etc)

/*
===============
D_Init
===============
*/
extern cvar_t	*engoo_lookuppalette;
void D_Init (void)
{

	r_skydirect = 1;

	d_subdiv16 = Cvar_Get ("d_subdiv16", "1", CVAR_ORIGINAL);
	d_mipcap = Cvar_Get ("d_mipcap", "0", CVAR_ORIGINAL);
	d_mipscale = Cvar_Get ("d_mipscale", "1", CVAR_ORIGINAL);
	engoo_lookuppalette = Cvar_Get ("engoo_lookuppalette", "0", CVAR_ORIGINAL);
	d_mipdetail = Cvar_Get ("d_mipdetail", "0", CVAR_ARCHIVE | CVAR_ORIGINAL); // leilei - mip detail cvar (combines mipcap and mipscale)
	r_wateralpha = Cvar_Get ("r_wateralpha", "1", CVAR_ARCHIVE | CVAR_ORIGINAL);	// leilei - water translucency
	r_drawpolys = false;
	r_worldpolysbacktofront = false;
	r_recursiveaffinetriangles = true;
	if (!r_pixbytes)
	r_pixbytes = 1;
	r_aliasuvscale = 1.0;

}


/*
===============
D_CopyRects
===============
*/
void D_CopyRects (vrect_t *prects, int transparent)
{

// this function is only required if the CPU doesn't have direct access to the
// back buffer, and there's some driver interface function that the driver
// doesn't support and requires Quake to do in software (such as drawing the
// console); Quake will then draw into wherever the driver points vid.buffer
// and will call this function before swapping buffers

	UNUSED(prects);
	UNUSED(transparent);
}


/*
===============
D_EnableBackBufferAccess
===============
*/
void D_EnableBackBufferAccess (void)
{
	VID_LockBuffer ();
}


/*
===============
D_TurnZOn
===============
*/
void D_TurnZOn (void)
{
// not needed for software version
}


/*
===============
D_DisableBackBufferAccess
===============
*/
void D_DisableBackBufferAccess (void)
{
	VID_UnlockBuffer ();
}


extern int ditheredrend;
/*
===============
D_SetupFrame
===============
*/
extern int reflectpass;

void D_SetupFrame (void)
{
	int		i;

	if (r_docrap == 1)
		d_viewbuffer = r_lowbuffer;
	else if (r_docrap > 1)
		d_viewbuffer = r_warpbuffer;
	else if (r_dowarp)
		d_viewbuffer = r_warpbuffer;
#ifdef WATERLOW
	else if (reflectpass)
		d_viewbuffer = r_reflectbuffer;
#else
	#ifdef WATERREFLECTIONS
	else if (reflectpass)
		d_viewbuffer = (void *)(byte *)vid.reflectbuffer;
#endif
#endif
	else
		d_viewbuffer = (void *)(byte *)vid.buffer;
	

//		d_fogbuffer = r_fogbuffer;
#ifdef WATERREFLECTIONS	
	if (!reflectpass && !inthedos)
	d_reflectbuffer = (void *)(byte *)vid.reflectbuffer;	// crapscreenreflect will write to this.
#endif
	

	if (r_docrap == 1)
		screenwidth = LOW_WIDTH;
	else if (r_docrap > 1)
		screenwidth = WARP_WIDTH;
	else if (r_dowarp)
		screenwidth = WARP_WIDTH;
#ifdef WATERLOW
	else if (reflectpass)
		screenwidth = WARP_WIDTH;	
#endif
	else
		screenwidth = vid.rowbytes;


	d_roverwrapped = false;
	d_initial_rover = sc_rover;

		// leilei - easy mip detail cvar controls override the quakes
	if (d_mipdetail->value == 64)
	{	// leilei - special exception - try to force half detail unless 32x32 and lower. no others!
			d_minmip = 1;
			for (i=0 ; i<(NUM_MIPS-1) ; i++)
				d_scalemip[i] = 0;
		
	}
	else
	if (d_mipdetail->value){
		d_minmip = d_mipdetail->value;
		if (d_minmip > 3)
			d_minmip = 3;
		else if (d_minmip < 0)
			d_minmip = 0;

	for (i=0 ; i<(NUM_MIPS-1) ; i++)
		d_scalemip[i] = basemip[i] * (d_mipdetail->value * 5);
	}
	else
	{
	d_minmip = d_mipcap->value;
	if (reflectpass)
		d_minmip = 2;	// leilei - force blurry textures for reflections
	if (d_minmip > 3)
		d_minmip = 3;
	else if (d_minmip < 0)
		d_minmip = 0;

	for (i=0 ; i<(NUM_MIPS-1) ; i++)
		d_scalemip[i] = basemip[i] * d_mipscale->value;
	}
#if	id386
		{
			if (r_filter->value == 1)	// Fabien Sanglard's Kernel Filtering
			{
				if (d_subdiv16->value)
					if (coloredlights == 2)
				if (foguse)
					d_drawspans = D_DrawSpans16_C_Dither;
					else
					d_drawspans = D_DrawSpans16_C_Dither_Filter;
					else
					d_drawspans = D_DrawSpans16_C_Filter;
				else
				if (coloredlights == 2)
					if (foguse)
					d_drawspans = D_DrawSpans16_C_Dither;
					else
					d_drawspans = D_DrawSpans16_C_Dither_Filter;
					else
					d_drawspans = D_DrawSpans8_C_Filter;
			}
			else
			if (r_filter->value == 2)	// Fabien Sanglard's Kernel Filtering tweaked to look slanted
			{
				if (d_subdiv16->value)
					if (coloredlights == 2)
				if (foguse)
					d_drawspans = D_DrawSpans16_C_Dither;
					else
					d_drawspans = D_DrawSpans16_C_Dither_Filter;
					else
					d_drawspans = D_DrawSpans16_C_Filter_64;
				else
				if (coloredlights == 2)
					if (foguse)
					d_drawspans = D_DrawSpans16_C_Dither;
					else
					d_drawspans = D_DrawSpans16_C_Dither_Filter;
					else
					d_drawspans = D_DrawSpans16_C_Filter_64;
			}
			else
			{
				if (d_subdiv16->value)
					if (coloredlights == 2)
					d_drawspans = D_DrawSpans16_C_Dither;
					else
						if (foguse)
#ifdef id386fog
							d_drawspans = D_DrawSpans16_C;
#else
						d_drawspans = D_DrawSpans16_C;
#endif
							else
					d_drawspans = D_DrawSpans16;
				else
					if (foguse)
					d_drawspans = D_DrawSpans16_C;
							else
					d_drawspans = D_DrawSpans8;
			}
		}
#else
				if (d_subdiv16->value == 1)
					d_drawspans = D_DrawSpans16_C;
		//		else if (d_subdiv16->value == 2)
		//			d_drawspans = D_DrawSpans32_C;
		//		else if (d_subdiv16->value == 3)
		//			d_drawspans = D_DrawSpans16_C_III;
				else
					d_drawspans = D_DrawSpans8;
#endif
	
	d_aflatcolor = 0;
}


#ifdef EXPREND

extern int shadowpass;
void D_DrawDeferredSpans8_C (espan_t *pspan);
void D_SetupFrameExperimental (void)
{
	int		i;

	if (shadowpass)
		d_viewbuffer = (void *)(byte *)vid.shadowbuffer;
	else
		d_viewbuffer = (void *)(byte *)vid.buffer;
	
		d_shadowbuffer = (void *)(byte *)vid.shadowbuffer;

		screenwidth = vid.rowbytes;


	d_roverwrapped = false;
	d_initial_rover = sc_rover;

		// leilei - easy mip detail cvar controls override the quakes
	if (d_mipdetail->value){
		d_minmip = d_mipdetail->value;
		if (d_minmip > 3)
			d_minmip = 3;
		else if (d_minmip < 0)
			d_minmip = 0;

	for (i=0 ; i<(NUM_MIPS-1) ; i++)
		d_scalemip[i] = basemip[i] * (d_mipdetail->value * 5);
	}

	else
	{
	d_minmip = d_mipcap->value;
	if (reflectpass)
		d_minmip = 2;	// leilei - force blurry textures for reflections
	if (d_minmip > 3)
		d_minmip = 3;
	else if (d_minmip < 0)
		d_minmip = 0;

	for (i=0 ; i<(NUM_MIPS-1) ; i++)
		d_scalemip[i] = basemip[i] * d_mipscale->value;
	}


	// we only support these spans for the time being.
	if (shadowpass)
		d_drawspans= D_DrawSpans8;
	else
		d_drawspans = D_DrawDeferredSpans8_C;
	
	d_aflatcolor = 0;
}

#endif
/*
===============
D_UpdateRects
===============
*/
void D_UpdateRects (vrect_t *prect)
{

// the software driver draws these directly to the vid buffer

	UNUSED(prect);
}

