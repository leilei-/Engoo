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
// d_scan.c
//
// Portable C scan-level rasterization code, all pixel depths.

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

unsigned char	*r_turb_pbase, *r_turb_pdest, *r_turb_prefst;;
fixed16_t		r_turb_s, r_turb_t, r_turb_sstep, r_turb_tstep;
int				*r_turb_turb;
int				r_turb_spancount;
// hexen II inheritance
//int				ZScanCount;
//#define	SCAN_SIZE		2048
//byte			scanList[SCAN_SIZE];
void D_DrawTurbulent8Span (void);
void D_DrawTurbulent8SpanT (void);	
extern int	fogcolr, fogcolg, fogcolb, fogthick, fogrange;	// leilei - fog
extern byte bumpmap[256];
extern pixel_t transTable[256][256];	 // for shrooms effect
pixel_t waterTable[256][256];
extern float oldwateralpha;
byte menumap[256][16];			// haha hack
extern pixel_t addTable[256][256];		// Additive blending effect
extern pixel_t mulTable[256][256];		// Multiply blending effect (for colormod)

extern pixel_t ditherTable[262144][4];
//extern pixel_t ditherTable[32768][4];

//	leilei - turning fogmap into a static = faster by an average of a few frames
//  so we do it here, we do it now, we do it forever! (or not)
static byte    *foggmap;
void SetFogMap (void)
{
	foggmap = (byte *)host_fogmap;
};
extern int		wootel[32][32][32];	
extern int reflectavailable;
int		gonnareflect;		// well are we gonna reflect or not?

extern cvar_t *r_tranquality;
extern cvar_t *r_wateralpha;
extern float fademap[256];				// Used in generation of certain alpha tables 
extern int oldwaterblend;
void WaterTablgeGet (void)
{
	float ay, ae;
	int		i, j, l, c, o;
	float  red, green, blue;
	int ooh;

	ooh = (int)r_tranquality->value;

	// Use these for now
	ae = 0.66;
	ay = 0.33;

	// or not.

	
	ay = r_wateralpha->value;	// water surface
	ae = 1 - ay;				// base pixels

//	newwateralpha = r_wateralpha->value; // why
	//if (oldwateralpha != newwateralpha)
	{
//	Con_Printf ("yep");


	
		for (l=0;l<255;l++)
	{
		for (c=0 ; c<255 ; c++)
		{
			if (oldwaterblend == 1){ // additive
			red = host_basepal[c*3]  + (host_basepal[l*3] *ay);
			green = host_basepal[c*3+1] + (host_basepal[l*3+1] *ay);
			blue = host_basepal[c*3+2]  + (host_basepal[l*3+2] *ay);
			}
			else if (oldwaterblend == 2){ // multiplicative
			red = host_basepal[c*3] *ae + ((host_basepal[c*3] * (host_basepal[l*3] * 0.05)) * ay);
			green = host_basepal[c*3+1] *ae + ((host_basepal[c*3+1]* (host_basepal[l*3+1] * 0.05)) * ay);
			blue = host_basepal[c*3+2] *ae + ((host_basepal[c*3+2] * (host_basepal[l*3+2] * 0.05)) * ay);
			}
			else if (oldwaterblend == 4){ // multiplicative

			red = host_basepal[c*3] *ae + ((host_basepal[l*3] * 0.1 * (host_basepal[c*3] * 0.5)) * ay);
			green = host_basepal[c*3+1] *ae + ((host_basepal[l*3+1] * 0.1 * (host_basepal[c*3+1] * 0.5)) * ay);
			blue = host_basepal[c*3+2] *ae + ((host_basepal[l*3+2] * 0.1 * (host_basepal[c*3+2] * 0.5)) * ay);
			}
			else if (oldwaterblend == 5){ // weird alpha thing
										  // how it works - goes through each color row from transparent(0) to opaque (16)
										  // this would also be used for the future (can you say decals?)
			red = host_basepal[c*3] *(fademap[l] * ae) + (host_basepal[l*3] * (fademap[l]+0.3 * ay));
			green = host_basepal[c*3+1] *(fademap[l] * ae) + (host_basepal[l*3+1] * (fademap[l]+0.3 * ay));
			blue = host_basepal[c*3+2] * (fademap[l] * ae) + (host_basepal[l*3+2] * (fademap[l]+0.3 * ay));
			}
			/*
			else if (oldwaterblend == 6){ // like above sort of
			red = host_basepal[c*3] *host_basepal[l*3] + (host_basepal[l*3] * host_basepal[c*3]) / 768;
			green = host_basepal[c*3+1] *host_basepal[l*3+1] + (host_basepal[l*3+1] * host_basepal[c*3+1]) / 768;
			blue = host_basepal[c*3+2] *host_basepal[l*3+2]+ (host_basepal[l*3+2] * host_basepal[c*3+2]) / 768;
			}
			else if (oldwaterblend == 13){ // weird ass blend, black is black, but is a mix of alpha!?

	
			red = host_basepal[c*3] *host_basepal[l*3] + (host_basepal[l*3] * host_basepal[c*3]) / 768;
			green = host_basepal[c*3+1] *host_basepal[l*3+1] + (host_basepal[l*3+1] * host_basepal[c*3+1]) / 768;
			blue = host_basepal[c*3+2] *host_basepal[l*3+2]+ (host_basepal[l*3+2] * host_basepal[c*3+2]) / 768;

			}
			*/
			else
			{
			red = host_basepal[c*3] *ae + (host_basepal[l*3] * ay);
			green = host_basepal[c*3+1] *ae + (host_basepal[l*3+1] * ay);
			blue = host_basepal[c*3+2] *ae + (host_basepal[l*3+2] * ay);
			}
			if (red > 255) red = 255;
			if (green > 255) green = 255;
			if (blue > 255) blue = 255;
			if (red < 0) red = 0;
			if (green < 0) green = 0;
			if (blue < 0) blue = 0;
	if (ooh)	waterTable[l][c] = BestColor(red,green,blue, 0, 255); // High quality color tables get best color
														
	else if (palmap2)	waterTable[l][c] = FindColor18(red,green,blue); 
	else waterTable[l][c] = FindColor(red,green,blue); // Since we do this live we must do this
		
														// fast! or i'll cry.
		}
		
	}
	//	if (!waterTable)
	//	Con_Printf (".... NOT!\n"); // how are we going to fail?
	//	else{
//		Con_Printf ("!\n");
			
	//	}

	}
}


/*
=============
D_AlphaShift

  This way is for like uh mods that use vid_cshift
  in ways intended for GLQuake use (i.e. contrasting a 
   centerprint visually)

  it's a hacky workaround to play prydon gate
=============
*/
extern int shiftshif;
extern int shiftalpha;


void D_AlphaShift (void)
{
	int			x,y;

	byte		*pbuf;

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
	if (shiftalpha < 20)
			return;

	for (y=0 ; y<vid.height ; y++)
	{
		int	t;

		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		t = (y & 1) << 1;

		for (x=0 ; x<vid.width ; x++)
		{
			if (shiftalpha < 120)
			pbuf[x] = transTable[shiftshif][pbuf[x]];	
			else if (shiftalpha < 220)
			pbuf[x] = transTable[pbuf[x]][shiftshif];	
			else if (shiftalpha > 220)
				pbuf[x] = (int)shiftshif;	
				
		}
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}

extern void *acolormap;	// FIXME: should go away


/*
=============
D_WarpScreen

// this performs a slight compression of the screen at the same time as
// the sine warp, to keep the edges from wrapping
=============
*/
void D_WarpScreen (void)
{
	int		w, h;
	int		u,v;
	byte	*dest;
	int		*turb;
	int		*col;
	byte	**row;
	byte	*rowptr[MAXHEIGHT+(AMP2*2)];
	int		column[MAXWIDTH+(AMP2*2)];
	float	wratio, hratio;

	w = r_refdef.vrect.width;
	h = r_refdef.vrect.height;

	wratio = w / (float)scr_vrect.width;
	hratio = h / (float)scr_vrect.height;

	for (v=0 ; v<scr_vrect.height+AMP2*2 ; v++)
	{
		rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
				 (screenwidth * (int)((float)v * hratio * h / (h + AMP2 * 2)));
	}
	for (u=0 ; u<scr_vrect.width+AMP2*2 ; u++)
	{
		column[u] = r_refdef.vrect.x +
				(int)((float)u * wratio * w / (w + AMP2 * 2));
	}

	turb = intsintable + ((int)(cl.time*SPEED)&(CYCLE-1));
	dest = vid.buffer + scr_vrect.y * vid.rowbytes + scr_vrect.x;

	for (v=0 ; v<scr_vrect.height ; v++, dest += vid.rowbytes)
	{
		col = &column[turb[v]];
		row = &rowptr[v];

		for (u=0 ; u<scr_vrect.width ; u+=4)
		{
			dest[u+0] = row[turb[u+0]][col[u+0]];
			dest[u+1] = row[turb[u+1]][col[u+1]];
			dest[u+2] = row[turb[u+2]][col[u+2]];
			dest[u+3] = row[turb[u+3]][col[u+3]];
		}
	}
}


/*
=============
D_CrapScreen

=============
*/
void D_CrapScreen (void)
{
	int		w, h;
	int		u,v;
	byte	*dest;
	int		*turb;
	int		*col;
	byte	**row;
	byte	*rowptr[MAXHEIGHT+(AMP2*2)];
	int		column[MAXWIDTH+(AMP2*2)];
	float	wratio, hratio;

	w = r_refdef.vrect.width;
	h = r_refdef.vrect.height;

	wratio = w / (float)scr_vrect.width;
	hratio = h / (float)scr_vrect.height;

	for (v=0 ; v<scr_vrect.height+AMP2*2 ; v++)
	{
		rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
				 (screenwidth * (int)((float)v * hratio * h / (h + AMP2 * 2)));
	}

	for (u=0 ; u<scr_vrect.width+AMP2*2 ; u++)
	{
		column[u] = r_refdef.vrect.x +
				(int)((float)u * wratio * w / (w + AMP2 * 2));
	}

	turb = atableofnothingtable;
	dest = vid.buffer + scr_vrect.y * vid.rowbytes + scr_vrect.x;

	for (v=0 ; v<scr_vrect.height ; v++, dest += vid.rowbytes)
	{
		col = &column[turb[v]];
		row = &rowptr[v];

		for (u=0 ; u<scr_vrect.width ; u+=4)
		{
			dest[u+0] = row[turb[u+0]][col[u+0]];
			dest[u+1] = row[turb[u+1]][col[u+1]];
			dest[u+2] = row[turb[u+2]][col[u+2]];
			dest[u+3] = row[turb[u+3]][col[u+3]];
		}
	}
}


void D_CrapScreenReflection (void)
{
	int		w, h;
	int		u,v;
	byte	*dest;
	int		*turb;
	byte	*rowptr[MAXHEIGHT+(AMP2*2)];
	int		column[MAXWIDTH+(AMP2*2)];
	float	wratio, hratio;

	w = r_refdef.vrect.width;
	h = r_refdef.vrect.height;

	wratio = w / (float)scr_vrect.width;
	hratio = h / (float)scr_vrect.height;

	for (v=0 ; v<scr_vrect.height+AMP2*2 ; v++)
	{
		rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
				 (screenwidth * (int)((float)v * hratio * h / (h + AMP2 * 2)));
	}

	for (u=0 ; u<scr_vrect.width+AMP2*2 ; u++)
	{
		column[u] = r_refdef.vrect.x +
				(int)((float)u * wratio * w / (w + AMP2 * 2));
	}

	turb = atableofnothingtable;

	dest = vid.buffer + scr_vrect.y * vid.rowbytes + scr_vrect.x;
	// Shrooms case
	if (r_doshrooms)
{
        for (v = 0; v < scr_vrect.height; v++, dest += vid.rowbytes)
        {
            byte *myrow = rowptr[v];
            int *mycol = column;
            byte *mydest = dest;

            for (u = 0; u < scr_vrect.width; u += 4, mycol += 4, mydest += 4)
            {
			// Shrooms effect test
				
				mydest[0] = transTable[myrow[mycol[0]]][mydest[0]];
				mydest[1] = transTable[myrow[mycol[1]]][mydest[1]];
				mydest[2] = transTable[myrow[mycol[2]]][mydest[2]];
				mydest[3] = transTable[myrow[mycol[3]]][mydest[3]];

            }
        }
    }
	// Normal case
	else
{
        for (v = 0; v < scr_vrect.height; v++, dest += vid.rowbytes)
        {
            byte *myrow = rowptr[v];
            int *mycol = column;
			int *mycolf = column;
            byte *mydest = dest;

            for (u = 0; u < scr_vrect.width; u += 4, mycol += 4, mydest += 4)
            {
		
                mydest[0] = myrow[mycol[0]];
                mydest[1] = myrow[mycol[1]];
                mydest[2] = myrow[mycol[2]];
               mydest[3] = myrow[mycol[3]];

            }
        }
    }
	
}

extern		cvar_t	*temp2;
// This is the MH version THANKS MH!!!
void D_CrapScreenMH (void)
{
		int		w, h;
	int		u,v;
	byte	*dest;
	int		*turb;
	byte	*rowptr[MAXHEIGHT+(AMP2*2)];
	int		column[MAXWIDTH+(AMP2*2)];
	float	wratio, hratio;

	w = r_refdef.vrect.width;
	h = r_refdef.vrect.height;

	wratio = w / (float)scr_vrect.width;
	hratio = h / (float)scr_vrect.height;

	for (v=0 ; v<scr_vrect.height+AMP2*2 ; v++)
	{
		rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
				 (screenwidth * (int)((float)v * hratio * h / (h + AMP2 * 2)));
	}

	for (u=0 ; u<scr_vrect.width+AMP2*2 ; u++)
	{
		column[u] = r_refdef.vrect.x +
				(int)((float)u * wratio * w / (w + AMP2 * 2));
	}

	turb = atableofnothingtable;

	dest = vid.buffer + scr_vrect.y * vid.rowbytes + scr_vrect.x;
	// Shrooms case
	if (r_doshrooms)
{
        for (v = 0; v < scr_vrect.height; v++, dest += vid.rowbytes)
        {
            byte *myrow = rowptr[v];
            int *mycol = column;
            byte *mydest = dest;

            for (u = 0; u < scr_vrect.width; u += 4, mycol += 4, mydest += 4)
            {
			// Shrooms effect test
				
				mydest[0] = transTable[myrow[mycol[0]]][mydest[0]];
				mydest[1] = transTable[myrow[mycol[1]]][mydest[1]];
				mydest[2] = transTable[myrow[mycol[2]]][mydest[2]];
				mydest[3] = transTable[myrow[mycol[3]]][mydest[3]];

            }
        }
    }
	// Normal case
	else
{
        for (v = 0; v < scr_vrect.height; v++, dest += vid.rowbytes)
        {
            byte *myrow = rowptr[v];
            int *mycol = column;
			int *mycolf = column;
            byte *mydest = dest;

            for (u = 0; u < scr_vrect.width; u += 4, mycol += 4, mydest += 4)
            {
		
                mydest[0] = myrow[mycol[0]];
                mydest[1] = myrow[mycol[1]];
                mydest[2] = myrow[mycol[2]];
               mydest[3] = myrow[mycol[3]];

            }
        }
    }
	
}

#if	!id386

/*
=============
D_DrawTurbulent8Span
=============
*/
void D_DrawTurbulent8Span (void)
{
	int		sturb, tturb;

	do
	{
		sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
		tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
		*r_turb_pdest++ = *(r_turb_pbase + (tturb<<6) + sturb);
		r_turb_s += r_turb_sstep;
		r_turb_t += r_turb_tstep;
	} while (--r_turb_spancount > 0);
}


		

		//
		// if (foguse)			// leilei - fogged case
		//		*pdest++ = (byte*)foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg >> 2 & 0xFF00)];
		//		else
		//		*pdest++ = pbase[(s >> 16) + (t >> 16) * cachewidth];
		//

#endif	// !id386
int			ferg;
void D_DrawTurbulent8Span_Fog (void)
{
	int		sturb, tturb;

	do
	{
		sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
		tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
		*r_turb_pdest++ = foggmap[(*(r_turb_pbase + (tturb<<6) + sturb)) + (ferg >> 2 & 0xFF00)];
		r_turb_s += r_turb_sstep;
		r_turb_t += r_turb_tstep;
	} while (--r_turb_spancount > 0);
}

int			gmcol;
int				izi, izistep, izistep2, sturb, tturb, teste; // mk transwater
short			*pz; // mk transwater

extern cvar_t *temp1;
extern cvar_t *r_waterquality;
extern cvar_t *r_wateralpha;
int depthen;
void D_DrawTurbulent8SpanAlphaReflections (void)
{
	int		sturb, tturb;
	int		refsturb, reftturb;
	int		refractsturb, refracttturb;
	unsigned char	temp, tempd2, tempd3, temp3, temp4;
	int		bleeh, bleeh2;
	float	wa;
	//int		pixsize = 2 * ( scr_vrect.width/ 640);
	int pixsize = 2;
	
	//wa = r_wateralpha->value;
//	wa = depthen / 8192;
	depthen = (depthen) >> 2;
	if (reflectpass == 1)
		return;	 // don't draw if we're doing water
	
					do
					{
						if (*pz <= (izi >> 16))
						{
//							depthen = ferg / 512;
	//						depthen = 44;
							sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
							tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
							if (r_waterquality->value > 1 && reflectavailable && !r_dowarp && !r_docrap){
							refsturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)] * 4)>>16)&16;
							reftturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)] * 4)>>16)&16;
							refractsturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)] * 4)>>16)&16;
							refracttturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)] * 4)>>16)&16;		
							tempd3 = *(r_turb_pdest + ((refracttturb) + refractsturb  >> pixsize));
							//tempd2 = *(r_turb_prefst + ((refsturb) + reftturb  >> 2));

							tempd2 = *(r_turb_prefst + ((refsturb) + reftturb  >> pixsize)) ;	// leilei - darken it based on wateralpha (for clarity etc) (+ ((int)(depthen) & 0xFF00))
								temp3 = addTable[tempd2][tempd3];
							}
							else{
							refractsturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)] * 4)>>16)&16;
							refracttturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)] * 4)>>16)&16;							
						//	if (!*(r_turb_pdest+9))
						//		tempd3 = *(r_turb_pdest);	// todo - edge check to prevent bleed
						//	else
								tempd3 = *(r_turb_pdest + ((refracttturb) + refractsturb  >> pixsize));
								
								
								temp3 = tempd3;
							}
						

							temp = *(r_turb_pbase + (tturb<<6) + sturb);

							if (foguse)
							*r_turb_pdest = foggmap[waterTable[temp][temp3]+ (ferg >> 2 & 0xFF00)];
							
							//temp = foggmap[(temp + (tturb<<6) + sturb) + (ferg >> 2 & 0xFF00)];
							else
							*r_turb_pdest = waterTable[temp][temp3];
						//	*r_turb_pdest = tempd2;

						}
						*r_turb_pdest++;
						*r_turb_prefst++;
						
						izi += izistep;
						pz++;
						r_turb_s += r_turb_sstep;
						r_turb_t += r_turb_tstep;
					} while (--r_turb_spancount > 0);
}


void D_DrawTurbulent8SpanAlphaRefractions (void)
{
	int		sturb, tturb;
	int		refsturb, reftturb;
	int		refractsturb, refracttturb;
	unsigned char	temp, tempd2, tempd3, temp3, temp4;
	int		bleeh, bleeh2;
	float	wa;
	//int		pixsize = 2 * ( scr_vrect.width/ 640);
	int pixsize = 2;
	
	//wa = r_wateralpha->value;
//	wa = depthen / 8192;
	depthen = (depthen) >> 2;
	if (reflectpass == 1)
		return;	 // don't draw if we're doing water
	
					do
					{
						if (*pz <= (izi >> 16))
						{
							sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
							tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
							refractsturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)] * 4)>>16)&16;
							refracttturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)] * 4)>>16)&16;							
							
						
							
							temp = *(r_turb_pbase + (tturb<<6) + sturb);
							tempd3 = *(r_turb_pdest + ((refracttturb) + refractsturb  >> pixsize));
						//	if ((*r_turb_pdest) < screenwidth)
								//screenwidth * pspan->v
						//	tempd3 = *(r_turb_pdest + (int)((bumpmap[temp] - bumpmap[temp] /2)* (float)temp2->value));
						//	else
						//	tempd3 = *(r_turb_pdest);
								
								
							temp3 = tempd3;

							if (foguse)
							*r_turb_pdest = foggmap[waterTable[temp][temp3]+ (ferg >> 2 & 0xFF00)];
														else
							*r_turb_pdest = waterTable[temp][temp3];

						}
						*r_turb_pdest++;
						
						
						izi += izistep;
						pz++;
						r_turb_s += r_turb_sstep;
						r_turb_t += r_turb_tstep;
					} while (--r_turb_spancount > 0);
}


void D_DrawTurbulent8SpanAlpha (void)
{
	int		sturb, tturb;
	unsigned char	temp;
					do
					{
						if (*pz <= (izi >> 16))
						{
							sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
							tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
							
							temp = *(r_turb_pbase + (tturb<<6) + sturb);
							if (foguse)
							*r_turb_pdest = foggmap[waterTable[temp][*r_turb_pdest]+ (ferg >> 2 & 0xFF00)];
							
							else
							*r_turb_pdest = waterTable[temp][*r_turb_pdest];
					
						}
						*r_turb_pdest++;
						izi += izistep;
						pz++;
						r_turb_s += r_turb_sstep;
						r_turb_t += r_turb_tstep;
					} while (--r_turb_spancount > 0);
}


// Leilei - this is very simple.
void D_DrawGelWaterSpan (void)
{


					do
					{
						if (*pz <= (izi >> 16))
						{
							if (foguse)
							*r_turb_pdest = foggmap[menumap[*r_turb_pdest][gmcol] + (ferg >> 2 & 0xFF00)];
							else
							*r_turb_pdest = menumap[*r_turb_pdest][gmcol];
						}
						*r_turb_pdest++;
						izi += izistep;
						pz++;
					} while (--r_turb_spancount > 0);
				
		
}

void D_DrawGelWaterSpanReflections (void)
{
	int		refsturb, reftturb;
	int		refractsturb, refracttturb;
	int	pixsize = 2;
	unsigned char	temp, tempd2, tempd3, temp3, temp4;
	depthen = (depthen) >> 2;

					do
					{
						if (*pz <= (izi >> 16))
						{
							if (r_waterquality->value > 1 && reflectavailable && !r_dowarp && !r_docrap){
							refsturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)] * 4)>>16)&16;
							reftturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)] * 4)>>16)&16;
							refractsturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)] * 4)>>16)&16;
							refracttturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)] * 4)>>16)&16;		

							tempd3 = *(r_turb_pdest + ((refracttturb) + refractsturb  >> 2));
							//tempd3 = transTable[*(r_turb_pdest + ((refracttturb) + refractsturb  >> 2))][*(r_turb_pdest - ((refracttturb) + refractsturb  >> 2))];
							//tempd3 = *(r_turb_pdest + ((refracttturb) + refractsturb  >> pixsize));
							//tempd2 = *(r_turb_prefst + ((refsturb) + reftturb  >> 2));

							tempd2 = *(r_turb_prefst + ((refsturb) + reftturb  >> pixsize)) ;	// leilei - darken it based on wateralpha (for clarity etc) (+ ((int)(depthen) & 0xFF00))
								//temp3 = addTable[tempd2][tempd3];
							temp3 = tempd2;
							}
							else{

							refractsturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)] * 4)>>16)&16;
							refracttturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)] * 4)>>16)&16;		
						
							//	tempd3 = *(r_turb_pdest + ((refracttturb) + refractsturb  >> 2));
							//tempd3 = transTable[*(r_turb_pdest + ((refracttturb) + refractsturb  >> 2))][*(r_turb_pdest + ((refractsturb) + refracttturb  >> 2))];

							tempd3 = *(r_turb_pdest + ((refracttturb) + refractsturb  >> 2));
								temp3 = tempd3;
							}
						


							
							
							
							
						

							if (foguse)
							*r_turb_pdest = foggmap[menumap[temp3][gmcol] + (ferg >> 2 & 0xFF00)];
							else
							*r_turb_pdest = menumap[temp3][gmcol];
						}
						*r_turb_pdest++;
						*r_turb_prefst++;
						
						izi += izistep;
	
						pz++;
						r_turb_s += r_turb_sstep;
					r_turb_t += r_turb_tstep;

					} while (--r_turb_spancount > 0);
				
		
}


void D_DrawGelWaterSpanRefractions (void)
{
	int		refsturb, reftturb;
	int		refractsturb, refracttturb;
	int	pixsize = 2;
	unsigned char	temp, tempd2, tempd3, temp3, temp4;
	depthen = (depthen) >> 2;

					do
					{
						if (*pz <= (izi >> 16))
						{
							refractsturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)] * 4)>>16)&16;
							refracttturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)] * 4)>>16)&16;		
						
							
							tempd3 = transTable[*(r_turb_pdest + ((refracttturb) + refractsturb  >> 2))][*(r_turb_pdest + ((refractsturb) + refracttturb  >> 2))];
							temp3 = tempd3;
							
							


							
							
							
														
						

							if (foguse)
							*r_turb_pdest = foggmap[menumap[temp3][gmcol] + (ferg >> 2 & 0xFF00)];
							else
							*r_turb_pdest = menumap[temp3][gmcol];
						}
						*r_turb_pdest++;
						
						
						izi += izistep;
	
						pz++;
						r_turb_s += r_turb_sstep;
					r_turb_t += r_turb_tstep;

					} while (--r_turb_spancount > 0);
				
		
}



extern cvar_t *r_waterblend;
/*
=============
Turbulent8
=============
*/
extern int		waterinsight;	// leilei  - water pixel shader
extern vec3_t		reflectorg;	// leilei  - water reflection
extern cvar_t *r_wateralpha;
void Turbulent8 (espan_t *pspan)
{
	int				count;
	fixed16_t		snext, tnext;

	
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz16stepu, tdivz16stepu, zi16stepu;
	int			wb;
	int			gelmode;
	int			watqual;
	float		zer;
	wb = (int)r_waterblend->value;
	if (wb == 3 || r_wateralpha->value < 1) 
			gelmode = 1; // leilei - skip the turb crap when in gelmode
	watqual = (int)r_waterquality->value;
	
	r_turb_turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));

	r_turb_sstep = 0;	// keep compiler happy
	r_turb_tstep = 0;	// ditto
	
	r_turb_pbase = (unsigned char *)cacheblock;
	
	sdivz16stepu = d_sdivzstepu * 16;
	tdivz16stepu = d_tdivzstepu * 16;
	zi16stepu = d_zistepu * 16;
	
	// mk transwater - begin
// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);
	izistep2 = izistep*2;
	// mk transwater - end
	
	// leilei - this is for the simple water
	// reuses our menu background tables to blend it
	// finds the nearest 'average' color from the color index
	if (wb == 3){ 
		// old values: 57 (breaks e3m1 slime/teleport), 
		//		
							gmcol = r_turb_pbase[2836]; // sample a pixel (TODO: Average a pixel on load)
							
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
			}
	do
	{

		r_turb_pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u; // mk - transwater
		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

	// we count on FP exceptions being turned off to avoid range problems // mk transwoter
		izi = (int)(zi * 0x8000 * 0x10000); // mk transwarter



		r_turb_s = (int)(sdivz * z) + sadjust;
		if (r_turb_s > bbextents)
			r_turb_s = bbextents;
		else if (r_turb_s < 0)
			r_turb_s = 0;

		r_turb_t = (int)(tdivz * z) + tadjust;
		if (r_turb_t > bbextentt)
			r_turb_t = bbextentt;
		else if (r_turb_t < 0)
			r_turb_t = 0;
		
		do
		{
			
		// calculate s and t at the far end of the span
			if (count >= 16)
				r_turb_spancount = 16;
			else
				r_turb_spancount = count;

			count -= r_turb_spancount;
			
			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivz16stepu;
				tdivz += tdivz16stepu;
				zi += zi16stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;	// guard against round-off error on <0 steps

				r_turb_sstep = (snext - r_turb_s) >> 4;
				r_turb_tstep = (tnext - r_turb_t) >> 4;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(r_turb_spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;	// guard against round-off error on <0 steps

				if (r_turb_spancount > 1)
				{
					r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
					r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
				}
			}

			r_turb_s = r_turb_s & ((CYCLE<<16)-1);
			r_turb_t = r_turb_t & ((CYCLE<<16)-1);

			if (foguse){ ferg = (int)(z / 1024);	if (ferg > 32762)	ferg = 32762; } // leilei - fog
			depthen = (int)z / 1024;	
// skip_turb:
			
			// mk transwater begin
			// leilei modified this to accomodate our waterTable
			// TODO: for hlbsp, use the rendertype/renderamt stuff
			// and add/transtable instead of watertable

			 
			 
			if (r_wateralpha->value < 1){
			if (r_drawwater){
				if (watqual){
				if (wb == 3){
						D_DrawGelWaterSpanRefractions ();
				}
				else{
				D_DrawTurbulent8SpanAlphaRefractions ();
				}
				}
				else
				{
				if (wb == 3){
						D_DrawGelWaterSpan ();
				}
				else{
				D_DrawTurbulent8SpanAlpha ();
				}

				}
			}
			}
			else
			{
				if (foguse){
					D_DrawTurbulent8Span_Fog ();
				} else {
					D_DrawTurbulent8Span ();
				 }
			}
			
			r_turb_s = snext;
			r_turb_t = tnext;
			

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

// has extra stuff for reflections

void Turbulent8Reflect (espan_t *pspan)
{
	int				count;
	fixed16_t		snext, tnext;

	
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz16stepu, tdivz16stepu, zi16stepu;
	int			wb;
	int			gelmode;
	int			watqual;
	float		zer;
	#ifdef WATERREFLECTIONS
	wb = (int)r_waterblend->value;
	if (wb == 3 || r_wateralpha->value < 1) 
			gelmode = 1; // leilei - skip the turb crap when in gelmode
	watqual = (int)r_waterquality->value;
	
	r_turb_turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));

	r_turb_sstep = 0;	// keep compiler happy
	r_turb_tstep = 0;	// ditto
	
	r_turb_pbase = (unsigned char *)cacheblock;
	
	sdivz16stepu = d_sdivzstepu * 16;
	tdivz16stepu = d_tdivzstepu * 16;
	zi16stepu = d_zistepu * 16;
	
	// mk transwater - begin
// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);
	izistep2 = izistep*2;
	// mk transwater - end
	
	// leilei - this is for the simple water
	// reuses our menu background tables to blend it
	// finds the nearest 'average' color from the color index
	if (wb == 3){ 
		// old values: 57 (breaks e3m1 slime/teleport), 
		//		
							gmcol = r_turb_pbase[2836]; // sample a pixel (TODO: Average a pixel on load)
							
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
			}
	do
	{

		r_turb_pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		// stupid hack to get reflections to align properly with the status bar
		if (reflectavailable && gonnareflect && !r_dowarp && !r_docrap){
		if (cl_sbar->value)
		r_turb_prefst = (unsigned char *)((byte *)vid.reflectbuffer + (vid.width * (vid.height - sb_what_lines)) -
				(screenwidth * pspan->v) + pspan->u);
		else
		r_turb_prefst = (unsigned char *)((byte *)vid.reflectbuffer + (vid.width * vid.height) -
				(screenwidth * pspan->v) + pspan->u);
		}
		pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u; // mk - transwater
		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

	// we count on FP exceptions being turned off to avoid range problems // mk transwoter
		izi = (int)(zi * 0x8000 * 0x10000); // mk transwarter



		r_turb_s = (int)(sdivz * z) + sadjust;
		if (r_turb_s > bbextents)
			r_turb_s = bbextents;
		else if (r_turb_s < 0)
			r_turb_s = 0;

		r_turb_t = (int)(tdivz * z) + tadjust;
		if (r_turb_t > bbextentt)
			r_turb_t = bbextentt;
		else if (r_turb_t < 0)
			r_turb_t = 0;
		
		do
		{
			
		// calculate s and t at the far end of the span
			if (count >= 16)
				r_turb_spancount = 16;
			else
				r_turb_spancount = count;

			count -= r_turb_spancount;
			
			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivz16stepu;
				tdivz += tdivz16stepu;
				zi += zi16stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;	// guard against round-off error on <0 steps

				r_turb_sstep = (snext - r_turb_s) >> 4;
				r_turb_tstep = (tnext - r_turb_t) >> 4;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(r_turb_spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;	// guard against round-off error on <0 steps

				if (r_turb_spancount > 1)
				{
					r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
					r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
				}
			}

			r_turb_s = r_turb_s & ((CYCLE<<16)-1);
			r_turb_t = r_turb_t & ((CYCLE<<16)-1);

			if (foguse){ ferg = (int)(z / 1024);	if (ferg > 32762)	ferg = 32762; } // leilei - fog
			depthen = (int)z / 1024;	
// skip_turb:
			
			// mk transwater begin
			// leilei modified this to accomodate our waterTable
			// TODO: for hlbsp, use the rendertype/renderamt stuff
			// and add/transtable instead of watertable

			 
			 
			if (r_wateralpha->value < 1){
			if (r_drawwater){
				if (watqual && gonnareflect){
				if (wb == 3){
						D_DrawGelWaterSpanReflections ();
				}
				else{
				D_DrawTurbulent8SpanAlphaReflections ();
				}
				}
				else
				{
				if (wb == 3){
						D_DrawGelWaterSpan ();
				}
				else{
				D_DrawTurbulent8SpanAlpha ();
				}

				}
			}
			}
			else
			{
				if (foguse){
					D_DrawTurbulent8Span_Fog ();
				} else {
					D_DrawTurbulent8Span ();
				 }
			}
			
			r_turb_s = snext;
			r_turb_t = tnext;
			

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
#endif
}



#if	!id386

/*
=============
D_DrawSpans8
=============
*/
void D_DrawSpans8 (espan_t *pspan)
{
	int				count, spancount;
	unsigned char	*pbase, *pdest;
	fixed16_t		s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (unsigned char *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
		pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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

			pdest += spancount;

         switch (spancount)
         {
         case 8: pdest[-8] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 7: pdest[-7] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 6: pdest[-6] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 5: pdest[-5] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 4: pdest[-4] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 3: pdest[-3] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 2: pdest[-2] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 1: pdest[-1] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         }

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

#endif

/*
=============
D_DrawSpans
qbism/mh
=============
*/

void D_DrawSpans16_C (espan_t *pspan) //qbism up it from 8 to 16.  This + unroll = big speed gain!
{
   int            count, spancount;
   unsigned char   *pbase, *pdest;
   fixed16_t      s, t, snext, tnext, sstep, tstep;
   float         sdivz, tdivz, zi, z, du, dv, spancountminus1;
   float         sdivzstepu, tdivzstepu, zistepu;
   int			forg= 0;			// leilei - fog
  // float		zf;			// leilei - fog
//   int			fogcount, fogcount2;	// leilei - fog
	int beep = 512;
	int boop = 512;
	int feg = 0;
   sstep = 0;   // keep compiler happy
   tstep = 0;   // ditto

   pbase = (unsigned char *)cacheblock;

   sdivzstepu = d_sdivzstepu * 16;
   tdivzstepu = d_tdivzstepu * 16;
   zistepu = d_zistepu * 16;

   do
   {
      pdest = (unsigned char *)((byte *)d_viewbuffer +
            (screenwidth * pspan->v) + pspan->u);

      count = pspan->count;
	 // fogcount2 = count;
   // calculate the initial s/z, t/z, 1/z, s, and t and clamp
      du = (float)pspan->u;
      dv = (float)pspan->v;

      sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
      tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
      zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;

	


      z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point
	  

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
		spancount = count > 15 ? 16 : count; // mh
         count -= spancount;
			
         if (count)
         {
         // calculate s/z, t/z, zi->fixed s and t at far end of span,
         // calculate s and t steps across span by shifting
            sdivz += sdivzstepu;
            tdivz += tdivzstepu;
            zi += zistepu;
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext <= 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture
			feg++;
           tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            sstep = (snext - s) >> 4;
            tstep = (tnext - t) >> 4;
			feg++;

				 if (foguse){ int te;
			forg = (int)(z / 1024);

			if (forg > 32762)	forg = 32762; 	} // leilei - fog

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
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point
            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext < 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            if (spancount > 1)
            {
               sstep = (snext - s) / (spancount - 1);
               tstep = (tnext - t) / (spancount - 1);
            }
			feg++;

         }

		 if (foguse){ int te;
			forg = (int)(z / 1024);
			
			if (forg > 32762)	forg = 32762; 
			
			


		} // leilei - fog

//qbism- Duff's Device loop unroll per mh.
         pdest += spancount;
		 if (foguse){
		

	
		 switch (spancount)
         {
         case 16: pdest[-16] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg  >> 2 & 0xFF00)]; s += sstep; t += tstep; 
         case 15: pdest[-15] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg + 512 >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 14: pdest[-14] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg  >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 13: pdest[-13] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg + 512  >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 12: pdest[-12] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 11: pdest[-11] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg + 512 >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 10: pdest[-10] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg  >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 9: pdest[-9] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg + 512  >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 8: pdest[-8] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 7: pdest[-7] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg + 512 >> 2 & 0xFF00)]; s += sstep; t += tstep;;
         case 6: pdest[-6] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg  >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 5: pdest[-5] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg + 512 >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 4: pdest[-4] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 3: pdest[-3] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg + 512  >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 2: pdest[-2] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg >> 2 & 0xFF00)]; s += sstep; t += tstep;
         case 1: pdest[-1] = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg + 512  >> 2 & 0xFF00)]; s += sstep; t += tstep;
         }
			 
		 }
		 else
		 {
         switch (spancount)
         {
         case 16: pdest[-16] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 15: pdest[-15] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 14: pdest[-14] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 13: pdest[-13] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 12: pdest[-12] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 11: pdest[-11] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 10: pdest[-10] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 9: pdest[-9] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 8: pdest[-8] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 7: pdest[-7] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 6: pdest[-6] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 5: pdest[-5] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 4: pdest[-4] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 3: pdest[-3] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 2: pdest[-2] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 1: pdest[-1] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         }
		 }
         s = snext;
         t = tnext;
		 if(beep){ boop =512;  beep = 0; }
		 else if(boop){ beep =512;  boop = 0; }
      } while (count > 0);

   } while ((pspan = pspan->pnext) != NULL);
} 






void D_DrawSpansBlank (espan_t *pspan) //Blank version
{
   int            count, spancount;
   unsigned char   *pbase, *pdest;
   fixed16_t      s, t, snext, tnext, sstep, tstep;
   float         sdivz, tdivz, zi, z, du, dv, spancountminus1;
   float         sdivzstepu, tdivzstepu, zistepu;

   sstep = 0;   // keep compiler happy
   tstep = 0;   // ditto

   pbase = (unsigned char *)cacheblock;

   sdivzstepu = d_sdivzstepu * 16;
   tdivzstepu = d_tdivzstepu * 16;
   zistepu = d_zistepu * 16;

   do
   {
      pdest = (unsigned char *)((byte *)d_viewbuffer +
            (screenwidth * pspan->v) + pspan->u);

      count = pspan->count;

   // calculate the initial s/z, t/z, 1/z, s, and t and clamp
      du = (float)pspan->u;
      dv = (float)pspan->v;

      sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
      tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
      zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
      z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

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
		spancount = count > 15 ? 16 : count; // mh
         count -= spancount;

         if (count)
         {
         // calculate s/z, t/z, zi->fixed s and t at far end of span,
         // calculate s and t steps across span by shifting
            sdivz += sdivzstepu;
            tdivz += tdivzstepu;
            zi += zistepu;
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext <= 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            sstep = (snext - s) >> 4;
            tstep = (tnext - t) >> 4;
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
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point
            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext < 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            if (spancount > 1)
            {
               sstep = (snext - s) / (spancount - 1);
               tstep = (tnext - t) / (spancount - 1);
            }
         }

//qbism- Duff's Device loop unroll per mh.
         pdest += spancount;
         switch (spancount)
         {
         case 16: pdest[-16] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 15: pdest[-15] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 14: pdest[-14] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 13: pdest[-13] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 12: pdest[-12] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 11: pdest[-11] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 10: pdest[-10] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 9: pdest[-9] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 8: pdest[-8] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 7: pdest[-7] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 6: pdest[-6] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 5: pdest[-5] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 4: pdest[-4] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 3: pdest[-3] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 2: pdest[-2] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         case 1: pdest[-1] = pbase[(s >> 16) + (t >> 16) * cachewidth]; s += sstep; t += tstep;
         }

         s = snext;
         t = tnext;

      } while (count > 0);

   } while ((pspan = pspan->pnext) != NULL);
} 




/*
=============
D_DrawSpans
qbism/mh
=============
*/


/*
=============
D_DrawSpans8_C_Filter

  Code adapted from Fabien Sanglard's Quake2 port
  adds filtering from https://github.com/fabiensanglard/Quake-2/tree/master/ref_soft
=============
*/


int kernel[2][2][2] =
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

int gernel[12][12][8] =
{
        {
                {16384,			0},
				{16384,			32},
				{16384,			64},
				{16384,			128},
				{16384,			256},
				{49152,			512},
				{49152,			1024},
				{49152,			2048},
				{49152,			4096},
				{49152,			8192},
                {49152,			16384}
        }
        ,
        {
                {32768,			49152},
                {0,				32768}
		}
};

void D_DrawSpans8_C_Filter (espan_t *pspan)
{
	int				count, spancount;
	unsigned char	*pbase, *pdest;
	fixed16_t		s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;
	int			forg;			// leilei - fog
	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (unsigned char *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
		pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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
		spancount = count > 7 ? 8 : count; // mh

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
			if (foguse){ forg = (int)(z / 1024);	if (forg > 32762)	forg = 32762; } // leilei - fog
			do
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


				
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}


// leilei experimental rendering
#ifdef EXPREND
void D_DrawDeferredSpans8_C (espan_t *pspan)
{
	int				count, spancount;
	unsigned char	*pbase, *pdest, *pshadow;
	fixed16_t		s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;
	int			forg;			// leilei - fog
	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (unsigned char *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
	//	pdest = (unsigned char *)((byte *)d_viewbuffer +
	//			(screenwidth * pspan->v) + pspan->u);
		pdest = (unsigned char *)((byte *)d_shadowbuffer +
				(screenwidth * pspan->v) + pspan->u);


		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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
		spancount = count > 7 ? 8 : count; // mh

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
			if (foguse){ forg = (int)(z / 1024);	if (forg > 32762)	forg = 32762; } // leilei - fog
			do
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


				
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}
#endif




// leilei experiment
void D_DrawSpans8_C_FilterAlter (espan_t *pspan)
{
	int				count, spancount;
	unsigned char	*pbase, *pdest;
	fixed16_t		s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;
	int			forg;			// leilei - fog
	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (unsigned char *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
		pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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
		spancount = count > 7 ? 8 : count; // mh

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
			if (foguse){ forg = (int)(z / 1024);	if (forg > 32762)	forg = 32762; } // leilei - fog
			do
			{
				int idiths = s;
				int iditht = t;
				int	there;
				int	teem = temp2->value;

				int X = (pspan->u + spancount) & 1;
				int Y = (pspan->v)&1;

				idiths += kernel[X][Y][0];
				iditht += kernel[X][Y][1];
				//	idiths += noisetable[X>>teem][Y];
				//	iditht += noisetable[Y<<teem][X];

				idiths = idiths >> 16;
			//	idiths = idiths ? idiths -teem : idiths;
						
//				there = teem >> 16;

				iditht = iditht >> 16;
			//	iditht = iditht ? iditht -teem : iditht;
				*pdest++ = *(pbase + 0 * cachewidth);


				
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}



extern qboolean r_dowarp;
extern int r_docrap;

extern byte		*r_warpbuffer;
#ifdef WATERREFLECTIONS
void D_DrawSpans8_Mirror_C_Filter (espan_t *pspan)
{
	int				count, spancount;
	unsigned char	*pbase, *pdest, *pbest;
	fixed16_t		s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;
	int			forg;			// leilei - fog
	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (unsigned char *)cacheblock;

	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
		pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);
		if (r_dowarp || r_docrap > 1)
			pbest = (unsigned char *)(r_warpbuffer +
				(screenwidth * pspan->v) + pspan->u);
			else
		pbest = (unsigned char *)((byte *)vid.reflectbuffer +
				(screenwidth * pspan->v) + pspan->u);


		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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
		spancount = count > 7 ? 8 : count; // mh

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
			if (foguse){ forg = (int)(z / 1024);	if (forg > 32762)	forg = 32762; } // leilei - fog
			do
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
//				*pdest++ = transTable[*pbest++][*pbase + idiths + iditht * cachewidth];
				*pdest++ = transTable[*(pbase + idiths + iditht * cachewidth)][*pbest];
//		*pdest++ = transTable[*(pbase + idiths + iditht * cachewidth)][*pbest];
				*pbest++;


				
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

#endif


/*
=============
D_DrawSpans16_C_Filter

  Code adapted from Fabien Sanglard's Quake2 port
  adds filtering from https://github.com/fabiensanglard/Quake-2/tree/master/ref_soft
=============
*/

// leilei - macrofying attempt......... 

#define SurfaceSpancount16(i) { switch (spancount) { case 16: i(16); case 15: i(15); case 14: i(14);  case 13: i(13);  case 12: i(12);  case 11: i(11);  case 10: i(10);  case 9: i(9);  case 8: i(8);  case 7: i(7);  case 6: i(6);  case 5: i(5);  case 4: i(4);  case 3: i(3);  case 2: i(2);  case 1: i(1); } }

void D_DrawSpans16_C_Filter (espan_t *pspan) //qbism- up it from 8 to 16
{
   int            count, spancount;
   unsigned char   *pbase, *pdest;
   fixed16_t      s, t, snext, tnext, sstep, tstep;
   float         sdivz, tdivz, zi, z, du, dv, spancountminus1;
   float         sdivzstepu, tdivzstepu, zistepu;
	int idiths, iditht, X, Y;	// filter stuff
	int			forg= 0;			// leilei - fog
	int		beep, boop, bap;
	int		fogra = 0;
   sstep = 0;   // keep compiler happy
   tstep = 0;   // ditto

   pbase = (unsigned char *)cacheblock;

   sdivzstepu = d_sdivzstepu * 16;
   tdivzstepu = d_tdivzstepu * 16;
   zistepu = d_zistepu * 16;
   bap = 1;

   do
   {
	 pdest = (unsigned char *)((byte *)d_viewbuffer +
            (screenwidth * pspan->v) + pspan->u);

      count = pspan->count;

   // calculate the initial s/z, t/z, 1/z, s, and t and clamp
      du = (float)pspan->u;
      dv = (float)pspan->v;

      sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
      tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
      zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
	  
      z =	  (float)0x10000 / zi;   // prescale to 16.16 fixed-point
	  

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
		spancount = count > 15 ? 16 : count; // mh

         count -= spancount;

         if (count)
         {
         // calculate s/z, t/z, zi->fixed s and t at far end of span,
         // calculate s and t steps across span by shifting
            sdivz += sdivzstepu;
            tdivz += tdivzstepu;
            zi += zistepu;
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

            
            


            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext <= 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            sstep = (snext - s) >> 4;
            tstep = (tnext - t) >> 4;

			bap *= -1;
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
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

            
            

            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext < 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            if (spancount > 1)
            {
               sstep = (snext - s) / (spancount - 1);
               tstep = (tnext - t) / (spancount - 1);
            }

			bap *= -1;
         }

		 if (foguse){ forg = (int)((float)0x10000 / zi / 1024);	if (forg > 32762)	forg = 32762; }
			
		beep = 0;
		boop = 256 * bap;
		

         // if spans are far apart, filter.
		 pdest += spancount;
		 
		 #define SUBDIV16FILTERFOG(i)	{  idiths = s; iditht = t; X = (pspan->u + i) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16; idiths = idiths ? idiths -1: idiths;	iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; pdest[-i] = foggmap[*(pbase + idiths + iditht * cachewidth) + (forg + boop  >> 2 & 0xFF00)];	 s += sstep;	t += tstep; boop *= -1; }
		 #define SUBDIV16FILTERFOGDITHER(i)	{ idiths = s; iditht = t; X = (pspan->u + i) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16; idiths = idiths ? idiths -1: idiths;	iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; pdest[-i] = foggmap[*(pbase + idiths + iditht * cachewidth) + (smoothtable[forg][i] >> 2 & 0xFF00)];	 s += sstep;	t += tstep;  }
		 #define SUBDIV16FILTERNOFOG(i)	{  idiths = s; iditht = t; X = (pspan->u + i) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16; idiths = idiths ? idiths -1: idiths;	iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; pdest[-i] = *(pbase + idiths + iditht * cachewidth);	s += sstep;	t += tstep; }

		 if (foguse == 2){
			 	SurfaceSpancount16(SUBDIV16FILTERFOGDITHER);
		 }
		 else if (foguse){
			 	SurfaceSpancount16(SUBDIV16FILTERFOG);
		 } else{
				SurfaceSpancount16(SUBDIV16FILTERNOFOG);
		 }

         s = snext;
         t = tnext;
	//	 if(beep){ boop =256;  beep = 0; }
	//	 else if(boop){ beep =256;  boop = 0; }
      } while (count > 0);

   } while ((pspan = pspan->pnext) != NULL);
}


void D_DrawSpans16_C_Filter_64 (espan_t *pspan) //qbism- up it from 8 to 16
{
   int            count, spancount;
   unsigned char   *pbase, *pdest;
   fixed16_t      s, t, snext, tnext, sstep, tstep;
   float         sdivz, tdivz, zi, z, du, dv, spancountminus1;
   float         sdivzstepu, tdivzstepu, zistepu;
	int idiths, iditht, X, Y;	// filter stuff
	int			forg= 0;			// leilei - fog
	int		beep, boop, bap;
	int		fogra = 0;
   sstep = 0;   // keep compiler happy
   tstep = 0;   // ditto

   pbase = (unsigned char *)cacheblock;

   sdivzstepu = d_sdivzstepu * 16;
   tdivzstepu = d_tdivzstepu * 16;
   zistepu = d_zistepu * 16;
   bap = 1;

   do
   {
	 pdest = (unsigned char *)((byte *)d_viewbuffer +
            (screenwidth * pspan->v) + pspan->u);

      count = pspan->count;

   // calculate the initial s/z, t/z, 1/z, s, and t and clamp
      du = (float)pspan->u;
      dv = (float)pspan->v;

      sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
      tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
      zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
	  
      z =	  (float)0x10000 / zi;   // prescale to 16.16 fixed-point
	  

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
		spancount = count > 15 ? 16 : count; // mh

         count -= spancount;

         if (count)
         {
         // calculate s/z, t/z, zi->fixed s and t at far end of span,
         // calculate s and t steps across span by shifting
            sdivz += sdivzstepu;
            tdivz += tdivzstepu;
            zi += zistepu;
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

            
            


            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext <= 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            sstep = (snext - s) >> 4;
            tstep = (tnext - t) >> 4;

			bap *= -1;
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
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

            
            

            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext < 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            if (spancount > 1)
            {
               sstep = (snext - s) / (spancount - 1);
               tstep = (tnext - t) / (spancount - 1);
            }

			bap *= -1;
         }

		 if (foguse){ forg = (int)((float)0x10000 / zi / 1024);	if (forg > 32762)	forg = 32762; }
			
		beep = 0;
		boop = 256 * bap;
		

         // if spans are far apart, filter.
		 pdest += spancount;
		 
		 #define SUBDIV1664FILTERFOG(i)	{  idiths = s; iditht = t; X = (pspan->u + i) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][0];	idiths = idiths >> 16; idiths = idiths ? idiths -1: idiths;	iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; pdest[-i] = foggmap[*(pbase + idiths + iditht * cachewidth) + (forg + boop  >> 2 & 0xFF00)];	 s += sstep;	t += tstep; boop *= -1; }
		 #define SUBDIV1664FILTERFOGDITHER(i)	{ idiths = s; iditht = t; X = (pspan->u + i) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][0];	idiths = idiths >> 16; idiths = idiths ? idiths -1: idiths;	iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; pdest[-i] = foggmap[*(pbase + idiths + iditht * cachewidth) + (smoothtable[forg][i] >> 2 & 0xFF00)];	 s += sstep;	t += tstep;  }
		 #define SUBDIV1664FILTERNOFOG(i)	{  idiths = s; iditht = t; X = (pspan->u + i) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][0];	idiths = idiths >> 16; idiths = idiths ? idiths -1: idiths;	iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; pdest[-i] = *(pbase + idiths + iditht * cachewidth);	s += sstep;	t += tstep; }

		 if (foguse == 2){
			 	SurfaceSpancount16(SUBDIV1664FILTERFOGDITHER);
		 }
		 else if (foguse){
			 	SurfaceSpancount16(SUBDIV1664FILTERFOG);
		 } else{
				SurfaceSpancount16(SUBDIV1664FILTERNOFOG);
		 }

         s = snext;
         t = tnext;
	//	 if(beep){ boop =256;  beep = 0; }
	//	 else if(boop){ beep =256;  boop = 0; }
      } while (count > 0);

   } while ((pspan = pspan->pnext) != NULL);
}


void D_DrawSpans16_C_Dither (espan_t *pspan) //qbism up it from 8 to 16.  This + unroll = big speed gain!
{
   int            count, spancount;
   unsigned char   *pbase, *pdest;
   fixed16_t      s, t, snext, tnext, sstep, tstep;
   float         sdivz, tdivz, zi, z, du, dv, spancountminus1;
   float         sdivzstepu, tdivzstepu, zistepu;
   int n;
   int			forg= 0;			// leilei - fog
   //float		forgf= 0;			// leilei - alpha'ed fog
   unsigned char			fogg[3];
//	float		fogthic;
	unsigned short *src;
//	fogthic = fogthick * 0.01;
	


   sstep = 0;   // keep compiler happy
   tstep = 0;   // ditto
   
   pbase = (unsigned char *)cacheblock;

   sdivzstepu = d_sdivzstepu * 16;
   tdivzstepu = d_tdivzstepu * 16;
   zistepu = d_zistepu * 16;

   do
   {
      pdest = (unsigned char *)((byte *)d_viewbuffer +
            (screenwidth * pspan->v) + pspan->u);

      count = pspan->count;

   // calculate the initial s/z, t/z, 1/z, s, and t and clamp
      du = (float)pspan->u;
      dv = (float)pspan->v;

      sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
      tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
      zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
      z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

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
     //    if (count >= 16)
   //         spancount = 16;
   //      else
    //        spancount = count;
		spancount = count > 15 ? 16 : count;
         count -= spancount;

         if (count)
         {
         // calculate s/z, t/z, zi->fixed s and t at far end of span,
         // calculate s and t steps across span by shifting
            sdivz += sdivzstepu;
            tdivz += tdivzstepu;
            zi += zistepu;
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext <= 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            sstep = (snext - s) >> 4;
            tstep = (tnext - t) >> 4;
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
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point
            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext < 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            if (spancount > 1)
            {
               sstep = (snext - s) / (spancount - 1);
               tstep = (tnext - t) / (spancount - 1);
            }
         }


         n = ((pspan->v & 1) << 1) | (pspan->u & 1);

		pdest += spancount;
		if (foguse){ forg = (int)(z / 1024);	if (forg > 32762)	forg = 32762; 
			//forgf = forg / 32768; // get down to the 1.0's
			
		} // leilei - fog
			
		if(foguse){
			


		  switch (spancount)
         {
		 case 16: src = (unsigned short *)pbase;  src += (s >> 16) + (t >> 16) * cachewidth;  pdest[-16] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 15: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-15] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg  >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 14: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-14] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 13: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-13] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg   >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 12: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-12] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 11: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-11] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg   >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 10: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-10] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 9: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-9] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg  >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 8: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-8] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 7: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-7] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg   >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 6: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-6] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 5: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-5] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg   >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 4: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-4] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 3: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-3] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg   >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 2: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-2] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 case 1: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-1] = foggmap[ditherTable[*src & 0x7FFF][n] + (forg  >> 2 & 0xFF00)]; n ^= 1; s += sstep; t += tstep;
		 
		 }
		 
		}
		else

		{
         switch (spancount)
         {
			 #ifdef THIRTYTWOBITHACK
		 case 16: src = (unsigned short *)pbase;  src += (s >> 16) + (t >> 16) * cachewidth;  pdest[-16] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 15: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-15] = *src & 0x7F00; n ^= 1; s += sstep; t += tstep;
		 case 14: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-14] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 13: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-13] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 12: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-12] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 11: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-11] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 10: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-10] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 9: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-9] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 8: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-8] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 7: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-7] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 6: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-6] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 5: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-5] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 4: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-4] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 3: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-3] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 2: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-2] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;
		 case 1: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-1] = *src & 0x7FFF; n ^= 1; s += sstep; t += tstep;


#else
		 case 16: src = (unsigned short *)pbase;  src += (s >> 16) + (t >> 16) * cachewidth;  pdest[-16] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 15: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-15] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 14: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-14] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 13: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-13] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 12: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-12] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 11: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-11] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 10: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-10] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 9: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-9] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 8: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-8] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 7: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-7] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 6: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-6] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 5: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-5] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 4: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-4] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 3: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-3] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 2: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-2] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 1: src = (unsigned short *)pbase; src += (s >> 16) + (t >> 16) * cachewidth; pdest[-1] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
#endif
         }
		}
         s = snext;
         t = tnext;

      } while (count > 0);

   } while ((pspan = pspan->pnext) != NULL);
} 



void D_DrawSpans16_C_Dither_Filter (espan_t *pspan) //the slowest spans on earth.
{
   int            count, spancount;
   unsigned char   *pbase, *pdest;
   unsigned short *src;
   fixed16_t      s, t, snext, tnext, sstep, tstep;
   float         sdivz, tdivz, zi, z, du, dv, spancountminus1;
   float         sdivzstepu, tdivzstepu, zistepu;
   int n;
   int idiths, iditht, X, Y;	// filter stuff
   int			forg;			// leilei - fog
   sstep = 0;   // keep compiler happy
   tstep = 0;   // ditto
   
   pbase = (unsigned char *)cacheblock;

   sdivzstepu = d_sdivzstepu * 16;
   tdivzstepu = d_tdivzstepu * 16;
   zistepu = d_zistepu * 16;

   do
   {
      pdest = (unsigned char *)((byte *)d_viewbuffer +
            (screenwidth * pspan->v) + pspan->u);



      count = pspan->count;

   // calculate the initial s/z, t/z, 1/z, s, and t and clamp
      du = (float)pspan->u;
      dv = (float)pspan->v;

      sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
      tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
      zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
      z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

      s = (int)(sdivz * z) + sadjust;
		s = s > bbextents ? bbextents : (s < 0 ? 0 : s);

      t = (int)(tdivz * z) + tadjust;
		t = t > bbextentt ? bbextentt : (t < 0 ? 0 : t);

      do
      {
      // calculate s and t at the far end of the span
		spancount = count > 15 ? 16 : count;
         count -= spancount;

         if (count)
         {
         // calculate s/z, t/z, zi->fixed s and t at far end of span,
         // calculate s and t steps across span by shifting
            sdivz += sdivzstepu;
            tdivz += tdivzstepu;
            zi += zistepu;
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext <= 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            sstep = (snext - s) >> 4;
            tstep = (tnext - t) >> 4;
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
            z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point
            snext = (int)(sdivz * z) + sadjust;
            if (snext > bbextents)
               snext = bbextents;
            else if (snext < 16)
               snext = 16;   // prevent round-off error on <0 steps from
                        //  from causing overstepping & running off the
                        //  edge of the texture

            tnext = (int)(tdivz * z) + tadjust;
            if (tnext > bbextentt)
               tnext = bbextentt;
            else if (tnext < 16)
               tnext = 16;   // guard against round-off error on <0 steps

            if (spancount > 1)
            {
               sstep = (snext - s) / (spancount - 1);
               tstep = (tnext - t) / (spancount - 1);
            }
         }


         n = ((pspan->v & 1) << 1) | (pspan->u & 1);

		if (foguse){ forg = (int)(z / 1024);	if (forg > 32762)	forg = 32762; } // leilei - fog

	pdest += spancount;

         switch (spancount)
         {
		 case 16: 				idiths = s; iditht = t; X = (pspan->u + 16) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-16] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 15: 				idiths = s; iditht = t; X = (pspan->u + 15) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-15] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 14: 				idiths = s; iditht = t; X = (pspan->u + 14) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-14] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 13: 				idiths = s; iditht = t; X = (pspan->u + 13) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-13] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 12: 				idiths = s; iditht = t; X = (pspan->u + 12) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-12] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 11: 				idiths = s; iditht = t; X = (pspan->u + 11) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-11] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 10: 				idiths = s; iditht = t; X = (pspan->u + 10) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-10] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 9: 				idiths = s; iditht = t; X = (pspan->u + 9) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-9] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 8: 				idiths = s; iditht = t; X = (pspan->u + 8) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-8] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 7: 				idiths = s; iditht = t; X = (pspan->u + 7) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-7] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 6: 				idiths = s; iditht = t; X = (pspan->u + 6) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-6] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 5: 				idiths = s; iditht = t; X = (pspan->u + 5) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-5] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 4: 				idiths = s; iditht = t; X = (pspan->u + 4) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-4] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 3: 				idiths = s; iditht = t; X = (pspan->u + 3) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-3] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 2: 				idiths = s; iditht = t; X = (pspan->u + 2) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-2] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
		 case 1: 				idiths = s; iditht = t; X = (pspan->u + 1) & 1; Y = (pspan->v)& 1;	idiths += kernel[X][Y][0];	iditht += kernel[X][Y][1];	idiths = idiths >> 16;	idiths = idiths ? idiths -1: idiths;
				iditht = iditht >> 16;	iditht = iditht ? iditht -1 : iditht; src = (unsigned short *)pbase; src +=  idiths + iditht * cachewidth; pdest[-1] = ditherTable[*src & 0x7FFF][n]; n ^= 1; s += sstep; t += tstep;
         }
         s = snext;
         t = tnext;

      } while (count > 0);

   } while ((pspan = pspan->pnext) != NULL);
} 



extern cvar_t *temp2;


//	Bilinear filtering from flquake

void D_DrawSpans8_C_Bilinear (espan_t *pspan)
{
	int				count, spancount;
	unsigned char	*pbase, *pdest;
	fixed16_t		s, t, snext, tnext, sstep, tstep;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz8stepu, tdivz8stepu, zi8stepu;
	int			forg	= 0;			// leilei - fog
	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (unsigned char *)cacheblock;


	
	sdivz8stepu = d_sdivzstepu * 8;
	tdivz8stepu = d_tdivzstepu * 8;
	zi8stepu = d_zistepu * 8;

	do
	{
		pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

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
		spancount = count > 7 ? 8 : count; // mh

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
			if (foguse){ forg = (int)(z / 1024);	if (forg > 32762)	forg = 32762; } // leilei - fog
			do
			{
				if (foguse)			// leilei - fogged case
				*pdest++ = foggmap[pbase[(s >> 16) + (t >> 16) * cachewidth]  + (forg >> 2 & 0xFF00)];
				else
				*pdest++ = pbase[(s >> 16) + (t >> 16) * cachewidth];
				s += sstep; t += tstep;

			} while (--spancount > 0);

			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}



#if	!id386


/*
=============
D_DrawZSpans
=============
*/
void D_DrawZSpans (espan_t *pspan)
{
	int				count, doublecount, izistep;
	int				izi;
	short			*pdest;
	unsigned		ltemp;
	double			zi;
	float			du, dv;

// FIXME: check for clamping/range problems
// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

	// calculate the initial 1/z
		du = (float)pspan->u;
		dv = (float)pspan->v;

		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
	// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		if ((long)pdest & 0x02)
		{
			*pdest++ = (short)(izi >> 16);
			
			izi += izistep;
			count--;
		}

		if ((doublecount = count >> 1) > 0)
		{
			do
			{
				ltemp = izi >> 16;
				izi += izistep;
				ltemp |= izi & 0xFFFF0000;
				izi += izistep;
				*(int *)pdest = ltemp;
				pdest += 2;
				
			} while (--doublecount > 0);
		}

		if (count & 1)
		
			*pdest = (short)(izi >> 16);

	} while ((pspan = pspan->pnext) != NULL);
}


#endif

