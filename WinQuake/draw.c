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

// draw.c -- this is the only file outside the refresh that touches the
// vid buffer

#include "quakedef.h"

// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  start
extern cvar_t	*con_alpha;

//pixel_t ditherTable[32768][4];
pixel_t ditherTable[262144][4];	// leilei - 18-bit dithering. holy crap, is this table huge

byte fademask[] =
{
	0x00,	// invisible
	0x14,	// 25%
	0x5a,	// 50%
	0xeb,	// 75%
	0xff,	// solid
};

#define CON_ALPHASTATES (sizeof(fademask) / sizeof(fademask[0]))
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  end

typedef struct {
	vrect_t	rect;
	int		width;
	int		height;
	byte	*ptexbytes;
	int		rowbytes;
} rectdesc_t;

static rectdesc_t	r_rectdesc;

byte		*draw_chars;				// 8*8 graphic characters
qpic_t		*draw_disc;
qpic_t		*draw_backtile;

//=============================================================================
/* Support Routines */

typedef struct cachepic_s
{
	char		name[MAX_QPATH];
	cache_user_t	cache;
} cachepic_t;

#define	MAX_CACHED_PICS		128
cachepic_t	menu_cachepics[MAX_CACHED_PICS];
int			menu_numcachepics;


qpic_t	*Draw_PicFromWad (char *name)
{
	return W_GetLumpName (name);
}





#ifdef _WIN32
extern qboolean WinNT;
#endif
/*
===============
BestColor
===============
*/
byte BestColor (int r, int g, int b, int start, int stop)
{
	int	i;
	int	dr, dg, db;
	int	bestdistortion, distortion;
	int	berstcolor;
	byte	*pal;

#ifdef _WIN32
	// lei - nt hack, so we never see illegal colors on Windows NT
	//if (WinNT){
//		if(start == 0) start = 1;
//		if(stop == 255) stop = 254;
//	}
#endif
//
// let any color go to 0 as a last resort
//
	bestdistortion = 256*256*4;
	berstcolor = 0;

	pal = host_basepal + start*3;
	for (i=start ; i<= stop ; i++)
	{
		dr = r - (int)pal[0];
		dg = g - (int)pal[1];
		db = b - (int)pal[2];
		pal += 3;
		distortion = dr*dr + dg*dg + db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			berstcolor = i;
		}
	}

	return berstcolor;
}

extern	byte		*host_otherpal;
/*
===============
BestColor
===============
*/
byte EGABestColor (int r, int g, int b)
{
	int	i;
	int	dr, dg, db;
	int	bestdistortion, distortion;
	int	berstcolor;
	byte	*pal;

//
// let any color go to 0 as a last resort
//
	bestdistortion = 256*256*4;
	berstcolor = 0;

	pal = host_otherpal + 0*3;
	for (i=0 ; i<= 254 ; i++)
	{
		dr = r - (int)pal[0];
		dg = g - (int)pal[1];
		db = b - (int)pal[2];
		pal += 3;
		distortion = dr*dr + dg*dg + db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			berstcolor = i;
		}
	}
	if (berstcolor > 256-host_fullbrights){

	pal = host_basepal + 7*3;
	for (i=8 ; i<= 15 ; i++)
	{
		dr = r - (int)pal[0];
		dg = g - (int)pal[1];
		db = b - (int)pal[2];
		pal += 3;
		distortion = dr*dr + dg*dg + db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			berstcolor = i;
		}
	}

	}
	else
	{
			pal = host_basepal + 0*3;
	for (i=0 ; i<= 8 ; i++)
	{
		dr = r - (int)pal[0];
		dg = g - (int)pal[1];
		db = b - (int)pal[2];
		pal += 3;
		distortion = dr*dr + dg*dg + db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			berstcolor = i;
		}
	}
	}
	return berstcolor;
}



int BestCol (int r, int g, int b)
{
	int	i;
	int	dr, dg, db;
	int	bestdistortion, distortion;
	int	berstcolor;
	byte	*pal;
	int start = 1;
	int stop = 254;
	bestdistortion = 256*256*4;
	berstcolor = 0;
	pal = host_basepal + start*3;
	for (i=start ; i<= stop ; i++)
	{
		dr = r - (int)pal[0];
		dg = g - (int)pal[1];
		db = b - (int)pal[2];
		pal += 3;
		distortion = dr*dr + dg*dg + db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			berstcolor = i;
		}
	}

	return berstcolor;
}


// COLOR Translation stuff
// Came straight out of image.c of Quake2 tools

byte	palmap[32][32][32];		// For FindColor's fast 15-bit lookup
byte	palmap2[64][64][64];		// Higher quality for lighting
unsigned char	palmap3[65535];		// for 888rgb...

byte	palmapnofb[32][32][32];		// for hl map conversion only

// this is just a lookup table version of the above

int FindColor (int r, int g, int b)
{
	int		bestcolor;
	if (r > 255)r = 255;if (r < 0)r = 0;
	if (g > 255)g = 255;if (g < 0)g = 0;
	if (b > 255)b = 255;if (b < 0)b = 0;
	bestcolor = palmap[r>>3][g>>3][b>>3];
	return bestcolor;
}

int FindColorNoFB (int r, int g, int b)
{
	int		bestcolor;
	if (r > 255)r = 255;if (r < 0)r = 0;
	if (g > 255)g = 255;if (g < 0)g = 0;
	if (b > 255)b = 255;if (b < 0)b = 0;
	bestcolor = palmapnofb[r>>3][g>>3][b>>3];
	return bestcolor;
}

int FindColor18 (int r, int g, int b)
{
	int		bestcolor;
	if (r > 255)r = 255;if (r < 0)r = 0;
	if (g > 255)g = 255;if (g < 0)g = 0;
	if (b > 255)b = 255;if (b < 0)b = 0;
	bestcolor = palmap2[r>>2][g>>2][b>>2];
	return bestcolor;
}

struct rgba_t		paleet[256];

/*
================
Draw_CachePic
================
*/

qpic_t	*Draw_CachePic (char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;
	
	for (pic=menu_cachepics, i=0 ; i<menu_numcachepics ; pic++, i++)
		if (!strcmp (path, pic->name))
			break;

	if (i == menu_numcachepics)
	{
		if (menu_numcachepics == MAX_CACHED_PICS)
			Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
		menu_numcachepics++;
		strcpy (pic->name, path);
	}

	dat = Cache_Check (&pic->cache);

	if (dat)
		return dat;

//
// load the pic from disk
//
	COM_LoadCacheFile (path, &pic->cache);

	dat = (qpic_t *)pic->cache.data;
	if (!dat)
	{
		Sys_Error ("Draw_CachePic: failed to load %s", path);
	}

	// leilei - quick palette translation
	if (rmap_ready){
		int bah;
		for (bah = 0; bah < dat->width*dat->height; bah++)
			dat->data[bah] = coltranslate[dat->data[bah]];
		}
	// leilei - quick palette translation

	SwapPic (dat);

	return dat;
}
extern byte	*host_colormap_nofb;
unsigned char d_15to8table[65536]; // 15to8
void MakeMy15to8(unsigned char *palette)
{
	byte	*pal;
	unsigned r,g,b;
	unsigned v;
	int		r1,g1,b1;
	int		j,k,l,m;
	unsigned short i;
	unsigned	*table;
	FILE *f;
	char s[255];
	
//	HWND hDlg, hProgress;
//	float gamma;
//	pal = host_basepal *3;
//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

//		v = (255<<24) + (r<<16) + (g<<8) + (b<<0);
//		v = (255<<0) + (r<<8) + (g<<16) + (b<<24);
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff;	// 255 is transparent


	table = d_8to24table;
	for (i=0 ; i<257 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
//	d_8to24table[255] &= 0xffffff;	// 255 is transparent

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	// FIXME: Precalculate this and cache to disk.
	for (i=0; i < (1<<15); i++) {
		/* Maps
			000000000000000
			000000000011111 = Red  = 0x1F
			000001111100000 = Blue = 0x03E0
			111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,l=10000*10000; v<256; v++,pal+=4) {
			r1 = r-pal[0];
			g1 = g-pal[1];
			b1 = b-pal[2];
			j = (r1*r1)+(g1*g1)+(b1*b1);
			if (j<l) {
				k=v;
				l=j;
			}
		}
		d_15to8table[i]=k;
	}
}



byte		*lmmap;
// are we done with lookup tables yet? I DONT THINK SO!

// despite the name these are actually gray
void IdnitColorColormaps (void) {
	int		levels, brights;
	int		l, c;
	float	frac;
	float red, green, blue;
	float	range;
	byte	*cropped, *lump_p;
	char	savename[1024];
	char	dest[1024];

	 range = 2;
	levels = 16;
	 brights = 1;	// ignore 255 (transparent)

	cropped = malloc((levels+256)*256);
	lump_p = cropped;
Con_Printf ("\n ohhhh ");
// shaded levels
	for (l=0;l<levels;l++)
	{
		frac = range - range*(float)l/(levels-1);
		for (c=0 ; c<256-brights ; c++)
		{
			red = 128;
			green = 128;
			blue = 128;

			red = (int)(red*frac+0.5);
			green = (int)(green*frac+0.5);
			blue = (int)(blue*frac+0.5);
		
			lump_p[l] = BestColor(red,green,blue, 0, 255);
			host_colormap[l] = lump_p[l];
		}
		Con_Printf ("shit");
	}
	

}

/*
==============
GrabColormap - from qbism- from qlumpy
modified by leilei to make it cvarable
with overbright clamp simulating glquake's bad looks

filename COLORMAP levels fullbrights
the first map is an identiy 0-255
the final map is all black except for the fullbrights
the remaining maps are evenly spread
fullbright colors start at the top of the palette.
==============
*/
void GrabColorMap (void)  //qbism - fixed, was a little screwy
{
    int		l, c, red, green, blue;
    float	frac, cscale, fracscaled;
    float   rscaled, gscaled, bscaled;
    byte *colmap;
	int RANGE = 2;
	int COLORLEVELS = 64;
	int PALBRIGHTS;
	
	RANGE = 2;

	if(!fullbrights) PALBRIGHTS = 0;
		else 
		PALBRIGHTS = 256 - host_fullbrights;


    colmap = host_colormap;

// shaded levels
    for (l=0; l<COLORLEVELS; l++)
    {
       // frac = (float)l/(COLORLEVELS-1);
		frac = RANGE - RANGE*(float)l/(COLORLEVELS-1);
		 if (!overbrights){	if (frac > 1)	frac = 1;}	// leilei - clamp it out for glsuck
        for (c=0 ; c<256-PALBRIGHTS ; c++)
        {
            red = (int)((float)host_basepal[c*3]*frac);
            green = (int)((float)host_basepal[c*3+1]*frac);
            blue = (int)((float)host_basepal[c*3+2]*frac); 

//
// note: 254 instead of 255 because 255 is the transparent color, and we
// don't want anything remapping to that
//
            *colmap++ = BestColor(red,green,blue, 0, 254);
        }
        for ( ; c<256 ; c++)
        {
            red = (int)host_basepal[c*3];
            green = (int)host_basepal[c*3+1];
            blue = (int)host_basepal[c*3+2];

            *colmap++ = BestColor(red,green,blue, 0, 254);
        }
    }
}

byte	additiveshade[16384];

void GrabColorMapNoFB (void)  //have no fullbrights or overbrights for a luminance colormap for additive blends only
{
    int		l, c, red, green, blue;
    float	frac, cscale, fracscaled;
    float   rscaled, gscaled, bscaled;
    byte *colmap;
	int RANGE = 2;
	int COLORLEVELS = 32;
	int PALBRIGHTS;
	
	RANGE = 2;

	 PALBRIGHTS = 2;
	host_colormap_nofb = malloc(16900);
    colmap = host_colormap_nofb;
	
// shaded levels
    for (l=0; l<COLORLEVELS; l++)
    {
       // frac = (float)l/(COLORLEVELS-1);
		frac = RANGE - RANGE*(float)l/(COLORLEVELS-1);
		 
        for (c=0 ; c<256 ; c++)
        {
            red = (int)((float)host_basepal[c*3]*frac);
            green = (int)((float)host_basepal[c*3+1]*frac);
            blue = (int)((float)host_basepal[c*3+2]*frac); 

//
// note: 254 instead of 255 because 255 is the transparent color, and we
// don't want anything remapping to that
//
            *colmap++ = BestColor(red,green,blue, 0, 254);
        }
   
    }
}


void TranslateColorMapEGA (void)  
{
    int		l, c, red, green, blue;
    float	frac, cscale, fracscaled;
    float   rscaled, gscaled, bscaled;
    byte *colmap;
	int RANGE = 2;
	int COLORLEVELS = 64;
	int PALBRIGHTS;
	float	sat = 2;
	float	s;
	RANGE = 6;

	if(!fullbrights) PALBRIGHTS = 0;
		else 
		PALBRIGHTS = 256 - host_fullbrights;


    colmap = host_colormap;

// shaded levels
    for (l=0; l<COLORLEVELS; l++)
    {
       // frac = (float)l/(COLORLEVELS-1);
		frac = RANGE - RANGE*(float)l/(COLORLEVELS-1);
		 if (!overbrights){	if (frac > 1)	frac = 1;}	// leilei - clamp it out for glsuck
        for (c=0 ; c<256-PALBRIGHTS ; c++)
        {
            red = (int)((float)host_otherpal[c*3]*frac);
            green = (int)((float)host_otherpal[c*3+1]*frac);
            blue = (int)((float)host_otherpal[c*3+2]*frac); 
			s = (red * 0.33333) + (green * 0.33333) + (blue * 0.33333);
			red = s + (red - s) * sat;
			green = s + (green - s) * sat;
			blue = s + (blue - s) * sat;
			if (red > 255) red = 255;
			if (green > 255) green = 255;
			if (blue > 255) blue = 255;
//
// note: 254 instead of 255 because 255 is the transparent color, and we
// don't want anything remapping to that
//
            *colmap++ = BestColor(red,green,blue, 0, 16);
        }
        for (; c<256 ; c++)
        {
            red = (int)host_otherpal[c*3];
            green = (int)host_otherpal[c*3+1];
            blue = (int)host_otherpal[c*3+2];
			s = (red * 0.33333) + (green * 0.33333) + (blue * 0.33333);
			red = s + (red - s) * sat;
			green = s + (green - s) * sat;
			blue = s + (blue - s) * sat;
			if (red > 255) red = 255;
			if (green > 255) green = 255;
			if (blue > 255) blue = 255;
            *colmap++ = BestColor(red,green,blue, 0, 16);
        }
    }
}

/*
==============
GrabColormapEGA 
==============
*/
void GrabColorMapEGA (void)  //qbism - fixed, was a little screwy
{
    int		l, c, red, green, blue;
    float	frac, cscale, fracscaled;
    float   rscaled, gscaled, bscaled;
    byte *colmap;
	int RANGE = 5; // extra range for ega
	int COLORLEVELS = 64;
	int PALBRIGHTS;

	if(!fullbrights) PALBRIGHTS = 0;
		else 
		PALBRIGHTS = 256 - 247; // leilei - try to only do fullbrights for the highlights.


    colmap = host_colormap;

// shaded levels
    for (l=0; l<COLORLEVELS; l++)
    {
       // frac = (float)l/(COLORLEVELS-1);
		frac = RANGE - RANGE*(float)l/(COLORLEVELS-1);
		 if (!overbrights){	if (frac > 1)	frac = 1;}	// leilei - clamp it out for glsuck
        for (c=0 ; c<256-PALBRIGHTS ; c++)
        {
            red = (int)((float)host_basepal[c*3]*frac);
            green = (int)((float)host_basepal[c*3+1]*frac);
            blue = (int)((float)host_basepal[c*3+2]*frac); 

            *colmap++ = BestColor(red,green,blue, 0, 16);
        }
        for ( ; c<16 ; c++)
        {
            red = (int)host_basepal[c*3];
            green = (int)host_basepal[c*3+1];
            blue = (int)host_basepal[c*3+2];

            *colmap++ = BestColor(red,green,blue, 0, 16);
        }
    }
}


extern cvar_t *temp2;
void GrabColorMapRGB (void)  // leilei - for the r g b components looking ups etcetera.
{
    int		l, c, red, green, blue;
    float	frac, cscale, fracscaled, frick;
    float   rscaled, gscaled, bscaled;
  //  byte *colmap;
	byte *colmapr;
	byte *colmapg;
	byte *colmapb;
	int RANGE = 2;
	int buh	  = 2;
	int COLORLEVELS = 64;
	int PALBRIGHTS;
	int	limit = 127;
	Con_Printf("YES!\nYES!!\nYES!!!\n");
	RANGE		= 2; // was 2.

	if(!fullbrights) PALBRIGHTS = 0;
		else 
		PALBRIGHTS = 256 - host_fullbrights;	 // gotta still have 'em fullbrights!

	PALBRIGHTS = 32;
  //  colmap = host_colormap;
	colmapr = host_colormap_red;
	colmapg = host_colormap_green;
	colmapb = host_colormap_blue;

// shaded levels
    for (l=0; l<COLORLEVELS; l++)
    {
     //   frac = (float)l/(COLORLEVELS-1);
		frick = (float)1/(COLORLEVELS-1);
		frac = RANGE*(float)l/(COLORLEVELS-1);
	//	frac = RANGE - RANGE*(float)l/(COLORLEVELS-1);
		 if (!overbrights){	if (frac > 1)	frac = 1;}	// leilei - clamp it out for glsuck
		 if (!overbrights){	if (frick > 1)	frick = 1;}	// leilei - clamp it out for glsuck
        for (c=0 ; c<256-PALBRIGHTS ; c++)
        {

            red = (int)((float)host_basepal[c*3]*frac);
            green = (int)((float)host_basepal[c*3+1]*frac);
            blue = (int)((float)host_basepal[c*3+2]*frac); 
		if (red > limit) red = limit;
		if (green > limit) green = limit;
		if (blue > limit) blue = limit;

	        *colmapr++ = red  >> buh;
			*colmapg++ = green >> buh;
			*colmapb++ = blue  >> buh;
        }
	    for ( ; c<256 ; c++)
        {
			
		if (red > 255) red = 255;
		if (green > 255) green = 255;
		if (blue > 255) blue = 255;
			
            red		=	host_basepal[c*3]  * 0.5;
            green	=   host_basepal[c*3+1] * 0.5;
            blue	=	host_basepal[c*3+2] * 0.5;

		if (red > limit) red = limit;
		if (green > limit) green = limit;
		if (blue > limit) blue = limit;
		
            *colmapr++ = red   >> buh;
			*colmapg++ = green >> buh;	// because fullbrights get added.
			*colmapb++ = blue  >> buh;	
			
        }
		
		
    }
}


// This version of the function tries to restrict to the 16x16 swatches
// so we don't have stuff like grays fading into blues or blues fading into grays
// and the overbrights won't turn into yellows. Stuff WILL look wrong.
// this is only a test. a test. a test. a test. a test. a test. a t
void GrabColorMapAlternative (void)  //qbism - fixed, was a little screwy
{
    int		l, c, g, red, green, blue;
    float	frac, cscale, fracscaled;
    float   rscaled, gscaled, bscaled;
    byte *colmap;
	int RANGE = 2;
	int COLORLEVELS = 64;
	int PALBRIGHTS;
	
	RANGE = 2;

	if(!fullbrights) PALBRIGHTS = 0;
		else 
		PALBRIGHTS = 256 - host_fullbrights;


    colmap = host_colormap;

// shaded levels
    for (l=0; l<COLORLEVELS; l++)
    {
       // frac = (float)l/(COLORLEVELS-1);
		frac = RANGE - RANGE*(float)l/(COLORLEVELS-1);
		 if (!overbrights){	if (frac > 1)	frac = 1;}	// leilei - clamp it out for glsuck
        for (c=0 ; c<256-PALBRIGHTS ; c+=16)
        {
			for (g=c ; g<c+16; g++){
            red = (int)((float)host_basepal[g*3]*frac);
            green = (int)((float)host_basepal[g*3+1]*frac);
            blue = (int)((float)host_basepal[g*3+2]*frac); 

//
// note: 254 instead of 255 because 255 is the transparent color, and we
// don't want anything remapping to that
//
            *colmap++ = BestColor(red,green,blue, c, c+16);
			}
        }
        for ( ; c<256 ; c++)
        {
            red = (int)host_basepal[c*3];
            green = (int)host_basepal[c*3+1];
            blue = (int)host_basepal[c*3+2];

            *colmap++ = BestColor(red,green,blue, 0, 254);
        }
    }
}


// Another alternative method where we try to saturate the darker colors so we can maintain the colors.
// a little less ugly than the top one because we still have access to all the colors
void GrabColorMapSaturation (void)  //qbism - fixed, was a little screwy
{
    int		l, c, red, green, blue;
	float	satr, satg, satb;
    float	frac, cscale, fracscaled;
    float   rscaled, gscaled, bscaled;
	float	satty;
    byte *colmap;
	int RANGE = 2;
	int COLORLEVELS = 64;
	int PALBRIGHTS;
	
	RANGE = 2;


	if(!fullbrights) PALBRIGHTS = 0;
		else 
		PALBRIGHTS = 256 - host_fullbrights;


    colmap = host_colormap;

// shaded levels
    for (l=0; l<COLORLEVELS; l++)
    {
       // frac = (float)l/(COLORLEVELS-1);
		frac = RANGE - RANGE*(float)l/(COLORLEVELS-1);
		satty = l  * 2.8 / 64;
		if (satty < 1)
			satty = 1; // clampity
		 if (!overbrights){	if (frac > 1)	frac = 1;}	// leilei - clamp it out for glsuck
        for (c=0 ; c<256-PALBRIGHTS ; c++)
        {
			

            red = (int)((float)host_basepal[c*3]*frac);
            green = (int)((float)host_basepal[c*3+1]*frac);
            blue = (int)((float)host_basepal[c*3+2]*frac); 

			
			satr = (red * 0.333) + (green * 0.333) + (blue * 0.333);
			red = satr + (red - satr)		* satty;
			green = satr + (green - satr)	* satty;
			blue = satr + (blue - satr)		* satty;

			if (red > 255) red = 255;
			if (green > 255) green = 255;
			if (blue > 255) blue = 255;
//
// note: 254 instead of 255 because 255 is the transparent color, and we
// don't want anything remapping to that
//
            *colmap++ = BestColor(red,green,blue, 0, 254);
        }
        for ( ; c<256 ; c++)
        {
            red = (int)host_basepal[c*3];
            green = (int)host_basepal[c*3+1];
            blue = (int)host_basepal[c*3+2];

            *colmap++ = BestColor(red,green,blue, 0, 254);
        }
    }
}


// 2001-09-18 New cvar system by Maddes (Init)  start
/*
===============
Draw_Init_Cvars
===============
*/
void Draw_Init_Cvars (void)
{
}
// 2001-09-18 New cvar system by Maddes (Init)  end

/*
===============
Draw_Init
===============
*/

//
// Our Friendly Neighborhood Lookup Tables
//
// TODO: Cache all to disk and load from disk when available
// gfx/addmap.lmp, gfx/mulmap.lmp, gfx/tinttab.lmp (hi hexen2)
// gfx/8to16.lmp, gfx/8to24.lmp, gfx/palmap.lmp, gfx/palmap2.lmp
// These are proposed filenames.
byte menumap[256][16];			// Used for menu backgrounds and simple colormod
byte gelmap[256];				// Unused id effect TO be used somehow. made redundant by menumap
byte remapmap[256];				// For translating an old palette to new on load
byte bumpmap[256];				// leilei experimental bumpmap feature

int		translate_bsp;
int		translate_mdl;
int		translate_gfx;
int		translate_spr;

float fademap[256];				// Used in generation of certain alpha tables 
byte coltranslate[256];			// TranslateToCustomPal - used for taking one palette to another without going through a whole 8to24to15to8 thing
pixel_t addTable[256][256];		// Additive blending effect
pixel_t mulTable[256][256];		// Multiply blending effect (for colormod)
pixel_t transTable[256][256];	// Alpha blending by 33% and 66%
//pixel_t alphaTable[256][256];	// Alpha blending by row 
pixel_t waterTable[256][256];	// Not cached to disk - for wateralpha only (THIS IS A STUPID but only sensible way to go for this)
int	smoothtable[32768][3];		// a table for smoothing out things.... :(
int		noisetable[512][5];		// a table for table

int		wootel[32][32][32];		// alternate kernel blend







unsigned d_8to24table[256];  // a super important table here


void InitGel (byte *palette)
{
	int		i;
	int		r;

	for (i=0 ; i<256 ; i++)
	{
//		r = (palette[i*3]>>4);
		r = (palette[i*3] + palette[i*3+1] + palette[i*3+2])/(16*3);
		gelmap[i] = /* 64 */ 0 + r;
	}
}

void InitBump (byte *palette)
{
	int		i;
	int		r;

	for (i=0 ; i<256 ; i++)
	{
		r = (palette[i*3] + palette[i*3+1] + palette[i*3+2]) / 3;
	//	r *= 3;
	//	if (r > 255) r = 255;
		bumpmap[i] = r;	
	}
}
// Gamepad notes:
// AUX29 - dpad up
// AUX32 - dpad left
// AUX30 - dpad right
// AUX31 - dpad down
extern	cvar_t	*temp2;
void InitWootel (void)
{
	int		i;
	int		r;
	int		ordered = 0;
	float ay, ae;
	int	result;
	int	range	= 32;
	float	spread	= 1024;
	//float	spread	= temp2->value;

	for (i=1 ; i<range; i++)
	{
		ae = i / range;
	for (r=0; r<32; r++){	
		ay = (RandomRange(0,spread));
		
		result = (float)ay;
		//if (result > 512) result = 512;
		//if (result < 0) result = 0;
		wootel[i][r][0] = result;
		}
	}

}

void InitNoise (void)
{
	int		i;
	int		r;
	float ay, ae;
	int	result;
	int	range	= 64;
	float	spread	= 1024;
	//float	spread	= temp2->value;

	for (i=1 ; i<range; i++)
	{
		ae = i / range;
	for (r=0; r<6; r++){	
		ay = (RandomRange(0,spread));
		
		result = (float)ay;
		//if (result > 512) result = 512;
		//if (result < 0) result = 0;
		noisetable[i][r] = result;
		
		//noisetable[i][r] = (float)(range * ay);
		}
	}
	
	for (r=0; r<6; r++){
	noisetable[0][r]	 = 0; //yeah.... 0
	//noisetable[64][r]	 = 64; //yeah...SIXTY FOUR
	}
}
/*
					t = (y & 1) << 1;
			if ((x & 3) != t)
				pbuf[x] = 0;
				*/
extern cvar_t *temp3;
void InitOrder (void)
{
	int		i;
	int		r;
	int		t;
	float ay, ae;
	int	result;
	int	range	= 64;
	float	spread	= 1024;
	//float	spread	= temp2->value;

	for (i=1 ; i<range; i++)
	{
		ae = i / range;
				t = (i & 1) << 1;
			for (r=0; r<6; r++){
				
					int	eep, erp;
				
					erp = (int)(range / (r + 1));
					for (eep=0; eep<1024; eep+=erp)
					{
				
						ay = 2048;
						if ((i & 1) != t)
							ay -= 768;

				}
			

		
		result = (float)ay;
		//if (result > 512) result = 512;
		//if (result < 0) result = 0;
		noisetable[i][r] = result;
		
		//noisetable[i][r] = (float)(range * ay);
		}
	}
	
	for (r=0; r<6; r++){
	noisetable[0][r]	 = 0; //yeah.... 0
	//noisetable[64][r]	 = 64; //yeah...SIXTY FOUR
	}
}


void InitSimple (void)
{
	int		i;
	int		r;
	float ay, ae;
	int	result;
	int	range	= 64;
	float	spread	= 1024;
	//float	spread	= temp2->value;

	for (i=1 ; i<range; i++)
	{
		ae = i / range;
	for (r=0; r<6; r++){	
		int ef;
		if (r == 0 || r == 2 || r == 4 || r == 6)
			ef = 1024;
		else
			ef = 512;
		ay = ef;
		
		result = (float)ay;
		//if (result > 512) result = 512;
		//if (result < 0) result = 0;
		noisetable[i][r] = result;
		
		//noisetable[i][r] = (float)(range * ay);
		}
	}
	
	for (r=0; r<6; r++){
	noisetable[0][r]	 = 0; //yeah.... 0
	//noisetable[64][r]	 = 64; //yeah...SIXTY FOUR
	}
}

// leilei - smooth table for some 'smooth' things... like model shading
void InitSmooth (void)
{
	int		i;
	int		r;
	int		k;

	float		noiz;
	float	noiiz;
	int		range = 64; // was 64
	InitNoise();
//	InitOrder();
//	InitSimple();
	for (i=0 ; i<32768; i+=range)
	{
		for (r=0; r<range; r++){
			for (k=0; k<6; k++){

					smoothtable[i+r][k] = MID(1024, i + 512 - (noisetable[r][k]),16000);
				
				
			}
		
		}
	}
	smoothtable[0][0] = 0;
	smoothtable[0][1] = 0;
	smoothtable[0][2] = 0;
	smoothtable[0][3] = 0;
	smoothtable[0][4] = 0;
}



extern byte		*host_origpal;		// QUAKE palette only!
// makes a little remap table for our new palette to use
void InitRemap (byte *palette)
{
	int		i;
	int		r, g, b;
	float sat = 3;
	float s;
	for (i=0 ; i<255 ; i++)
	{
			r = palette[i*3];
			g = palette[i*3+1];
			b = palette[i*3+2];
			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;
#ifdef EGAHACK
			s = (r * 0.33333) + (g * 0.33333) + (b * 0.33333);
			r = s + (r - s) * sat;
			g = s + (g - s) * sat;
			b = s + (b - s) * sat;
#endif
			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;
		coltranslate[i] = BestColor(r,g,b, 0, 254);
		remapmap	[i] = BestColor(r,g,b, 0, 256-host_fullbrights);	// no fullbrights...

	}
	coltranslate[256] = 256;
	coltranslate[255] = 255; // null is null
		
	rmap_ready = 1;
		
		
}




// makes a little remap table for our new palette to use
void InitRemapEGA (byte *palette)
{
	int		i;
	int		r;
	int		bah[3];

	for (i=0 ; i<256 ; i++)
	{
		bah[0] = d_8to24table[palette[i]];
		bah[1] = d_8to24table[palette[i+1]];
		bah[2] = d_8to24table[palette[i+2]];

		if (i > 256 - host_fullbrights){
		coltranslate[i] = BestColor(bah[0], bah[1], bah[2], 9, 15);
		remapmap[i] = BestColor(bah[0], bah[1], bah[2], 9, 15);
		}
		else
		{
		coltranslate[i] = BestColor(bah[0], bah[1], bah[2], 0, 8);
		remapmap[i] = BestColor(bah[0], bah[1], bah[2], 0, 8);
		}
		

	}

		rmap_ready = 1;
	
}

extern void PalmapStaticized ();
struct rgba_t								r_palette[256];
int	lookupcaching;
int nolookups;
// we make every lookup table in this function
// TODO: Caching to disk and loading if available
// TODO ALSO: Skipping of this by parameter
extern int inthedos;

void FogTableRefresh (void);
extern cvar_t *r_tranquality;
cvar_t	*engoo_lookuppalette;
#ifdef _WIN32
extern unsigned char	vid_curpal[256*3];
#endif

	int	erh;
int	fbs[256];
void MassiveLookupTablesInit (void)
{

	int		i, j, l, c;
	float  red, green, blue;
	int		r, g, b;
	int		beastcolor;
	int	ugly;
	
	unsigned char*	thepaltouse;
	InitSmooth();
	
	if (erh == 53)
#ifdef _WIN32
	thepaltouse = vid_curpal;		// leilei - silly experiment to try adjusted palettes as a basis
									// just for recalculating the big tables to see less color clashing
	else
#endif
	thepaltouse = host_basepal;
	InitBump(thepaltouse);	
//	InitFader();
//	if (COM_CheckParm ("-cache"))
//		lookupcaching = 1;		// try to cache it all for future loading

	
	if (inthedos)
	printf ("\nGenerating additive table - ");
		for (l=0;l<255;l++)
	{
		for (c=0 ; c<255 ; c++)
		{
			red = thepaltouse[c*3] + thepaltouse[l*3];
			green = thepaltouse[c*3+1] + thepaltouse[l*3+1];
			blue = thepaltouse[c*3+2] + thepaltouse[l*3+2];
			addTable[l][c] = BestColor(red,green,blue, 0, 254);
		}
		if (inthedos)	printf ("."); 		
	}

	
		// Make the Additive Lookup Table



	// Make the Alpha Transparency table
	if (inthedos)
	printf ("\nGenerating alpha table - ");
		for (l=0;l<255;l++)
	{
		for (c=0 ; c<255 ; c++)
		{
			red = thepaltouse[c*3] *0.66 + (thepaltouse[l*3] * 0.33);
			green = thepaltouse[c*3+1] *0.66 + (thepaltouse[l*3+1] * 0.33);
			blue = thepaltouse[c*3+2] *0.66 + (thepaltouse[l*3+2] * 0.33);
			transTable[l][c] = BestColor(red,green,blue, 0, 254);
		}
				
		if (inthedos)	printf ("."); 
	}

		



	if (COM_CheckParm ("-nolookups")){
		nolookups = 1;
if (inthedos)
	printf ("Color lookup tables are skipped\n");
	//	hqlite = 1;		// force bestcolor water
				return; // we don't need your lookupation
				}
	if (inthedos)
	printf ("\n---------------------\nGenerating all the lookup tables\n(If your computer is slow, this will take up to 30 seconds. Use the -nolookups \nparameter if you find this unsettling)\nYou can also skip the 18-bit generation by passing -no18\nAlternatively, you can pass -ugly for faster but ugly generation\n---------------------\n");

	if (COM_CheckParm ("-ugly")){
	printf ("UGLY GENERAION MODE ON!\n");
		ugly = 1;}
	else
		ugly = 0;
	// Do 24bit lookup

		VID_SetPalette2 (thepaltouse);

		{

						//printf ("da\n");

		if (inthedos)
	printf ("\nGenerating 15-bit lookup table - ");
	for (r=0 ; r<256 ; r+=8)
	{
		for (g=0 ; g<256 ; g+=8)
		{
			for (b=0 ; b<256 ; b+=8)
			{
				float lol = 3.6;
				beastcolor = BestColor (r + lol, g + lol, b + lol, 0, 254);
				palmap[r>>3][g>>3][b>>3] = beastcolor;
//				palmap3[r>>3>>10 + g>>3>>5 + b>>3] = beastcolor;
		         						// andrewj's crud here
								{ int k;
								  int pix = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);

								  int rr = r - 4;
								  int gg = g - 4;
								  int bb = b - 4;

								  int col[4];
								  int ity[4];
								  byte *found;
          
								  for (k = 0 ; k < 4 ; k++)
								  {
									col[k] = BestColor(rr + lol, gg + lol, bb + lol, 0, 254);  

									// apply error term
									found = thepaltouse + col[k] * 3;

									rr += (rr - (int)found[0]);
									gg += (gg - (int)found[1]);
									bb += (bb - (int)found[2]);

									ity[k] = found[0] + found[1] + found[2];
								  }

								  // sort colors by intensity
								  for (rr = 0 ; rr < 4 ; rr++)
								  for (k = 0 ; k < 3 ; k++)
								  {
									if (ity[k] < ity[k+1])
									{
									  gg = col[k] ; col[k] = col[k+1] ; col[k+1] = gg;
									  gg = ity[k] ; ity[k] = ity[k+1] ; ity[k+1] = gg;
									}
								  }

								  ditherTable[pix][0] = col[0];
								  ditherTable[pix][3] = col[1];
								  ditherTable[pix][2] = col[2];
								  ditherTable[pix][1] = col[3];
								
								}	


			}
		}
		if (inthedos)
	printf("."); // yep do the dot thing. it's a big 32k table so we have to
	}

			}

	if (!palmap)
		if (inthedos)
	printf ("FAILED!\n");
	else{
		if (inthedos)
	printf ("!\n");
				

	}


	// Make the 18-bit lookup table here
	// This is a HUGE 256kb table, the biggest there is here
	// TODO: Option to enable this 
//	if (r_lightquality->value){

	if (COM_CheckParm ("-no18")){
			if (inthedos)
	printf("\n18-bit lookup generation skipped - High quality light mode DISABLED!\n");
					}
	else
	{
//	if(!lookupcaching)	// when in -cache, skip the files loading for regeneration
//		fileinfo3 = COM_LoadHunkFile ("palmap2.lmp");
//			if (!fileinfo3)
			{
				
	if (inthedos)
	printf ("\nGenerating 18-bit lookup table - ");
	for (r=0 ; r<256 ; r+=4)
	{
		for (g=0 ; g<256 ; g+=4)
		{
			for (b=0 ; b<256 ; b+=4)
			{
				if (ugly) beastcolor = FindColor (r, g, b);
				else
				beastcolor = BestColor (r, g, b, 0, 254);
				palmap2[r>>2][g>>2][b>>2] = beastcolor;
				
			
		
			}
		}
		if (inthedos)	printf ("."); // yep do the dot thing. it's a big 256k table so we have to
	}
	
			}
	if (!palmap2)
		if (inthedos)
	printf ("FAILED!\n");
	else{
		if (inthedos)
	printf ("!\n");


	}
	}
//	}

	// what follows are 3 big 64kb lookup tables. pretty funny when you have this little 32kb
	// 15to8 table up there

//	printf ("\nGenerating rgb color surface clamping table - ");

	// Make the Additive Lookup Table
	if (inthedos)
	printf ("\nGenerating additive table - ");
		for (l=0;l<255;l++)
	{
		for (c=0 ; c<255 ; c++)
		{
			red = thepaltouse[c*3] + thepaltouse[l*3];
			green = thepaltouse[c*3+1] + thepaltouse[l*3+1];
			blue = thepaltouse[c*3+2] + thepaltouse[l*3+2];
			if (ugly) addTable[l][c] = FindColor(red,green,blue);
				else
			addTable[l][c] = BestColor(red,green,blue, 0, 254);
		}
		if (inthedos)	printf ("."); 		
	}
	if (!addTable)
		if (inthedos)
	printf ("FAILED!\n");
	else{
		if (inthedos)
	printf ("!\n");
		
		
	}
	// Make the Multiply Lookup Table which doesn't work
	if (inthedos)
	printf ("\nGenerating multiply blend table - ");
		for (l=0;l<255;l++)
	{
		for (c=0 ; c<255 ; c++)
		{
			float ee, er, erp;
			ee = 0.5;
			er = 0.5;
			erp = 0.03; // 0.1;
			red = ((thepaltouse[l*3] * erp * (thepaltouse[c*3] * ee)) * er);
			green =  ((thepaltouse[l*3+1] * erp * (thepaltouse[c*3+1] * ee)) * er);
			blue = ((thepaltouse[l*3+2] * erp * (thepaltouse[c*3+2] * ee)) * er);

			if (red>255) red=255; if (green>255) green=255; if (blue>255) blue=255;
			if (ugly) mulTable[l][c] = FindColor(red,green,blue);
				else
			mulTable[l][c] = BestColor(red,green,blue, 0, 223);
		}
		if (inthedos)	printf ("."); 
	}
		if (!mulTable)
		if (inthedos)
	printf ("FAILED!\n");
		else{
		if (inthedos)
	printf ("!\n");
		
		
		}

//		if(palmap2)
		PalmapStaticized();

		// Caching lookups to disk, possibly folder unfriendly.
/*	if (lookupcaching){
		
		if (palmap)
			COM_WriteFile ("palmap.lmp", palmap, 32768);
		if (palmap2)
			COM_WriteFile ("palmap2.lmp", palmap2, 262144);
		if (addTable)
			COM_WriteFile ("addmap.lmp", addTable, 65536);
		if (mulTable)
			COM_WriteFile ("mulmap.lmp", mulTable, 65536);
		if (transTable)
			COM_WriteFile ("tinttab.lmp", transTable, 65536);
		if (ditherTable)
			COM_WriteFile ("dithtab.lmp", ditherTable, 262144);	// good lord this is huge.
		
	}*/


		{
			int ahe;

			for (ahe=0; ahe<256; ahe++)
			{
				if (ahe < host_fullbrights)
					fbs[ahe] = 0;

				else
					fbs[ahe] = mulTable[ahe][6];
			}

		}

		GrabColorMapRGB();
}

// mangled a bit from grabcolormap
void SetFogMap (void);


void FogTableRefresh (void)
 {

	int ugly;
  int		l, c, red, green, blue;
	float	fogthik;
    float	frac;
	float	frac2;
	float	farc;
    byte *colmap;
	int RANGE = 1;
	int COLORLEVELS = 64;
	int PALBRIGHTS;
	
	RANGE = 1;
	ugly = (int)r_tranquality->value;

	// TODO: Feed 'ese
//	fogcolr = 0;
//	fogcolg = 0;
//	fogcolb = 0;
//	fogthick= 100; // thickness of fog (100 = opaque, 50 = half... ok well you get it)
	fogthik = fogthick * 0.01;
	 PALBRIGHTS = 0;
//	Con_Printf ("Fog generating with %f %f %f %f", fogthick, fogcolr, fogcolg, fogcolb);
    colmap = host_fogmap;

	for (l=COLORLEVELS; l>0; l--)
    {
		frac = 2 - 2*(float)l/(COLORLEVELS) ;
		frac2 = RANGE - RANGE/(float)l/(COLORLEVELS);
		farc = 1 - ((frac / 2 ) * fogthik);
		if (farc < 0)
				farc = 0;
        for (c=0 ; c<256-PALBRIGHTS ; c++)
        {
			red =	((int)((float)host_basepal[c*3]		* farc)	+	(fogcolr / 2 * frac* fogthik));
            green = ((int)((float)host_basepal[c*3+1]	* farc)	+	(fogcolg / 2 * frac* fogthik));
            blue =	((int)((float)host_basepal[c*3+2]	* farc)	+	(fogcolb / 2 * frac* fogthik)); 
			if (red>255)red=255;
			if (green>255)green=255;
			if (blue>255)blue=255;
			if (red<0)red=0;
			if (green<0)green=0;
			if (blue<0)blue=0;
			if (!ugly) *colmap++ = FindColor18(red,green,blue);
			else
            *colmap++ = BestColor(red,green,blue, 0, 254);
        }
    }
		SetFogMap(); // set the static.
};



// auto-regenning water tables
// compared to transtable, findcolor is super fast but actually 
// looks SLIGHTLY different to people with super eagle/sai eyes
// but not as much as one would lynch over
//
//
//
// This is also a good place to prototype new transparency lookup tables.
//
extern cvar_t *r_wateralpha;
extern int newwateralpha;
extern int oldwateralpha;
extern int oldwaterblend;

void WaterTableGet (void)
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



void RemapMapTableGet (void)
{
	int		i, j, l, c, o;
	float  red, green, blue;
	int ooh;


		for (l=0;l<255;l++)
	{
			remapmap[l] = 56;
	}
}


void Draw_Init (void)
{
	int		i, j, l, c, r;


	draw_chars = W_GetLumpName ("conchars");
	draw_disc = W_GetLumpName ("disc");
	draw_backtile = W_GetLumpName ("backtile");

	r_rectdesc.width = draw_backtile->width;
	r_rectdesc.height = draw_backtile->height;
	r_rectdesc.ptexbytes = draw_backtile->data;
	r_rectdesc.rowbytes = draw_backtile->width;
	

	
	// Make the menu background table
	// This has been extended to allow 16 others via r_menucolors
	for (i=0 ; i<256 ; i++)
	{
		r = (host_basepal[i*3] + host_basepal[i*3+1] + host_basepal[i*3+2])/(16*3);
		for (j=0; j<9; j++)
		menumap[i][j] = (j * 16) + r;
		for (j=14; j>7; j--)
		menumap[i][j] = (j * 16) + 15 - r;
		menumap[i][14] = 14*16 + r; // forward hack for the muzzleflash fire colors

		// and yes, color ramp #15 is left all black. any further is possibly reserved for slow
		// hexen 2 style menus which use a translucency table
	}

	// and no you can't skip it. i'll use it for colored text and other coolities




}


void RemapMenuMap (void)
{
	int		i, j, l, c, r;
// really for the 16 color mode

	for (i=0 ; i<256 ; i++)
	{
		for (j=0; j<15; j++)
		menumap[i][j] = coltranslate[menumap[i][j]];
		
	}


};







int lilchar = 1;





/*
================
Draw_Character

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Character (int x, int y, int num)
{
	byte			*dest;
	byte			*source;
	unsigned short	*pusdest;
	int				drawline;
	int				row, col;
	int ye; if(lilchar)	ye = 4;	else ye = 8;
	num &= 255;
	

	if (y <= -8)
		return;			// totally off screen
	
#ifdef PARANOID
	if (y > vid.height - 8 || x < 0 || x > vid.width - 8)
		Sys_Error ("Con_DrawCharacter: (%i, %i)", x, y);
	if (num < 0 || num > 255)
		Sys_Error ("Con_DrawCharacter: char %i", num);
#endif

	row = num>>4;
	col = num&15;
	source = draw_chars + (row<<10) + (col<<3);

	if (y < 0)
	{	// clipped
		drawline = 8 + y;
		source -= 128*y;
		y = 0;
	}
	else
		drawline = 8;


	if (r_pixbytes == 1)
	{
		dest = vid.conbuffer + y*vid.conrowbytes + x;

		while (drawline--)
		{
			if (source[0])
				dest[0] = source[0];
			if (source[1])
				dest[1] = source[1];
			if (source[2])
				dest[2] = source[2];
			if (source[3])
				dest[3] = source[3];
			if (source[4])
				dest[4] = source[4];
			if (source[5])
				dest[5] = source[5];
			if (source[6])
				dest[6] = source[6];
			if (source[7])
				dest[7] = source[7];
			source += 128;
			dest += vid.conrowbytes;
		}
	}
	else
	{
	// FIXME: pre-expand to native format?
		pusdest = (unsigned short *)
				((byte *)vid.conbuffer + y*vid.conrowbytes + (x<<1));

		while (drawline--)
		{
			if (source[0])
				pusdest[0] = d_8to16table[source[0]];
			if (source[1])
				pusdest[1] = d_8to16table[source[1]];
			if (source[2])
				pusdest[2] = d_8to16table[source[2]];
			if (source[3])
				pusdest[3] = d_8to16table[source[3]];
			if (source[4])
				pusdest[4] = d_8to16table[source[4]];
			if (source[5])
				pusdest[5] = d_8to16table[source[5]];
			if (source[6])
				pusdest[6] = d_8to16table[source[6]];
			if (source[7])
				pusdest[7] = d_8to16table[source[7]];

			source += 128;
			pusdest += (vid.conrowbytes >> 1);
		}
	}
}



void Draw_Character_Scaled (int x, int y, unsigned int num)
{
	byte		*source;
	int		drawline;
	int		u, v, row, col, s;

	int height, width;
	float vmax, umax;

	num &= 255;

	if (y <= -8)
		return;			// totally off screen

	if (y > vid.vconheight - 8 || x < 0 || x > vid.vconwidth - 8)
		return;

	row = num>>4;
	col = num&15;
	source = draw_chars + (row<<10) + (col<<3);

		if (y < 0)
	{	// clipped
		drawline = 8 + y;
		source -= 128*y;
		y = 0;
	}
	else
		drawline = 8;

//	if (y < 0)
//	{	// clipped
//		drawline = 8 + y;
//		source -= 256*y;
//		y = 0;
//	}
//	else
//		drawline = 8;

	height = drawline;
	width = 8;

	vmax = height * vid.height / (float)vid.vconheight;
	umax = width * vid.width / (float)vid.vconwidth;
	
	

	{
		byte *dest = vid.conbuffer + (y * vid.height / vid.vconheight) * vid.conrowbytes
			+ (x * vid.width / vid.vconwidth);

		for (v = 0; v < vmax; v++)
		{
			for (u = 0; u < umax; u++)
			{
				s = u * vid.vconwidth / vid.width
					+ (v * vid.vconheight / vid.height) * 128;
								
					if (source[s])
						dest[u] = source[s];
			
			}

			dest += vid.conrowbytes;
		}
	}

}



/*
================
Draw_String
================
*/
void Draw_String (int x, int y, char *str)
{
	while (*str)
	{

		Draw_Character (x, y, *str);
		str++;
		x += 8;
	}
}

void Draw_String_Scaled (int x, int y, char *str)
{
	while (*str)
	{

		Draw_Character_Scaled (x, y, *str);
		str++;
		x += 8;
	}
}
/*
================
Draw_DebugChar

Draws a single character directly to the upper right corner of the screen.
This is for debugging lockups by drawing different chars in different parts
of the code.
================
*/
void Draw_DebugChar (char num)
{
	byte			*dest;
	byte			*source;
	int				drawline;
	extern byte		*draw_chars;
	int				row, col;

	if (!vid.direct)
		return;		// don't have direct FB access, so no debugchars...

	drawline = 8;

	row = num>>4;
	col = num&15;
	source = draw_chars + (row<<10) + (col<<3);

	dest = vid.direct + 312;

	while (drawline--)
	{
		dest[0] = source[0];
		dest[1] = source[1];
		dest[2] = source[2];
		dest[3] = source[3];
		dest[4] = source[4];
		dest[5] = source[5];
		dest[6] = source[6];
		dest[7] = source[7];
		source += 128;
		dest += 320;
	}
}

/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	byte			*dest, *source;
	unsigned short	*pusdest;
	int				v, u;

	if ((x < 0) ||
		(x + pic->width > vid.width) ||
		(y < 0) ||
		(y + pic->height > vid.height))
	{
		Sys_Error ("Draw_Pic: bad coordinates");
	}

	source = pic->data;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y * vid.rowbytes + x;

		for (v=0 ; v<pic->height ; v++)
		{
			Q_memcpy (dest, source, pic->width);
			dest += vid.rowbytes;
			source += pic->width;
		}
	}
	else
	{
	// FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;

		for (v=0 ; v<pic->height ; v++)
		{
			for (u=0 ; u<pic->width ; u++)
			{
				pusdest[u] = d_8to16table[source[u]];
			}

			pusdest += vid.rowbytes >> 1;
			source += pic->width;
		}
	}
}




void Draw_Pic_Scaled (int x, int y, qpic_t *pic)
{
	byte		*source;
	int		v, u, s;
	float vmax, umax;


	if ((x < 0) ||
		(x + pic->width > vid.width) ||
		(y < 0) ||
		(y + pic->height > vid.height))
	{
		Sys_Error ("Draw_Pic: bad coordinates");
	}

	source = pic->data;

	umax = pic->width * vid.width / (float)vid.vconwidth;
	vmax = pic->height * vid.height / (float)vid.vconheight;

	{
		byte *dest = vid.buffer + (y*vid.height/vid.vconheight) * vid.rowbytes + (x*vid.width/vid.vconwidth);
		for (v = 0; v < vmax; v++)
		{
			for (u = 0; u < umax; u++) 
			{
				s = u * vid.vconwidth / vid.width + (v * vid.vconheight/vid.height) * pic->width;
				dest[u] = source[s];
			}
			dest += vid.rowbytes;
		}
	}

}
extern	int	col_toremap;
extern	int	col_light;
extern	int	col_toremap2;

// for recolorable status bars
void Draw_Pic_Scaled_Color (int x, int y, qpic_t *pic, int color)
{
	byte		*source;
	int		v, u, s;
	float vmax, umax;
	int		remapfrom, remapto, special;
	int		remapfrombrown1, remaptobrown1;
	int		remapfrombrown2, remaptobrown2;
	int		remapfromorange, remaptoorange;
	remapfrom = col_toremap * 16;
	remapto = col_toremap * 16 + 16;

	remapfrombrown1 = 10 * 16;
	remaptobrown1 = 10 * 16 + 16;

	remapfrombrown2 = 7 * 16;
	remaptobrown2 = 7 * 16 + 16;

	remapfromorange = col_toremap2 * 16;
	remaptoorange = col_toremap2 * 16 + 16;

	if (color == 16){
		Draw_Pic_Scaled(x, y, pic); //use normal function instead
		return;
	}

	

	if ((x < 0) ||
		(x + pic->width > vid.width) ||
		(y < 0) ||
		(y + pic->height > vid.height))
	{
		Sys_Error ("Draw_Pic: bad coordinates");
	}

	source = pic->data;

	umax = pic->width * vid.width / (float)vid.vconwidth;
	vmax = pic->height * vid.height / (float)vid.vconheight;
	if (col_toremap == 1)
		special = 1;
	{
		byte *dest = vid.buffer + (y*vid.height/vid.vconheight) * vid.rowbytes + (x*vid.width/vid.vconwidth);
		for (v = 0; v < vmax; v++)
		{
			for (u = 0; u < umax; u++) 
			{
				s = u * vid.vconwidth / vid.width + (v * vid.vconheight/vid.height) * pic->width;
				if (source[s] > remapfrom && source[s] < remapto || source[s] > remapfrombrown1 && source[s] < remaptobrown1 || source[s] > remapfrombrown2 && source[s] < remaptobrown2)
						dest[u] = menumap[source[s]][color];
				else 				if (source[s] > remapfromorange && source[s] < remaptoorange)
						dest[u] = menumap[source[s]][col_light];
				else
				if (source[s] > remapfrom && source[s] < remapto)
						dest[u] = menumap[source[s]][color];
				else
				dest[u] = source[s];
			}
			dest += vid.rowbytes;
		}
	}

}
//==========================================================================
//
// Draw_PicCropped
//
// Draws a qpic_t that is clipped at the bottom/top edges of the screen.
//
//==========================================================================

void Draw_PicCropped (int x, int y, qpic_t *pic)
{
	byte		*source;
	int		v, u, height;

	if ((x < 0) || (x+pic->width > (int)vid.width))
	{
		Sys_Error("%s: bad coordinates");
	}

	if (y >= vid.height || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

	if (y+pic->height > vid.height)
	{
		height = vid.height-y;
	}
	else if (y < 0)
	{
		height = pic->height+y;
	}
	else
	{
		height = pic->height;
	}

	source = pic->data;
	if (y < 0) 
	{
		source += (pic->width * (-y));
		y = 0;
	}

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y*vid.rowbytes + x;

			for (v = 0; v < height; v++)
			{
				memcpy(dest, source, pic->width);
				dest += vid.rowbytes;
				source += pic->width;
			}
		
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes>>1) + x;
		// FIXME: transparency bits are missing
		for (v = 0; v < height; v++)
		{
			for (u = 0; u < pic->width; u++)
			{
				dest[u] = d_8to16table[source[u]];
			}
			dest += vid.rowbytes>>1;
			source += pic->width;
		}
	}
}

void Draw_PicCropped_Scaled (int x, int y, qpic_t *pic)
{
	byte		*source;
	int		v, u, height, s;
	float vmax, umax;

	if ((x < 0) || (x+pic->width > (int)vid.width))
		Sys_Error("%s: bad coordinates");

	if (y >= vid.height || y+pic->height < 0)
		return;

	if (y+pic->height > vid.vconheight)
		height = vid.vconheight-y;
	else if (y < 0)
		height = pic->height+y;
	else
		height = pic->height;

	source = pic->data;
	if (y < 0) 
	{
		source += (pic->width * (-y));
		y = 0;
	}

	vmax = height * vid.height / (float)vid.vconheight;
	umax = pic->width * vid.width / (float)vid.vconwidth;

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + (y * vid.height / vid.vconheight) * vid.rowbytes
			+ (x * vid.width / vid.vconwidth);
		for (v = 0; v < vmax; v++) 
		{
			for (u = 0; u < umax; u++) 
			{
				s = u*vid.vconwidth/vid.width + (v*vid.vconheight/vid.height) * pic->width;
				dest[u] = source[s];
			
			}
			dest += vid.rowbytes;
		}
	} 
	else /* r_pixbytes == 2 */ 
	{
		unsigned short *dest = (unsigned short *)vid.buffer 
			+ (y*vid.height/vid.vconheight) * (vid.rowbytes>>1) + (x*vid.width/vid.vconwidth);
		for (v = 0; v < vmax; v++) 
		{
			for (u = 0; u < umax; u++) 
			{
				s = u * vid.vconwidth / vid.width 
					+ (v * vid.vconheight / vid.height) * pic->width;
				dest[u] = d_8to16table[source[s]];
			}
			dest += vid.rowbytes>>1;
		}
	}
}


/*
=============
Draw_TransPic
=============
*/
void Draw_TransPic (int x, int y, qpic_t *pic)
{
	byte	*dest, *source, tbyte;
	unsigned short	*pusdest;
	int				v, u;

	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 ||
		 (unsigned)(y + pic->height) > vid.height)
	{
		Sys_Error ("Draw_TransPic: bad coordinates");
	}

	source = pic->data;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y * vid.rowbytes + x;

		if (pic->width & 7)
		{	// general
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u++)
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = tbyte;

				dest += vid.rowbytes;
				source += pic->width;
			}
		}
		else
		{	// unwound
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u+=8)
				{
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = tbyte;
					if ( (tbyte=source[u+1]) != TRANSPARENT_COLOR)
						dest[u+1] = tbyte;
					if ( (tbyte=source[u+2]) != TRANSPARENT_COLOR)
						dest[u+2] = tbyte;
					if ( (tbyte=source[u+3]) != TRANSPARENT_COLOR)
						dest[u+3] = tbyte;
					if ( (tbyte=source[u+4]) != TRANSPARENT_COLOR)
						dest[u+4] = tbyte;
					if ( (tbyte=source[u+5]) != TRANSPARENT_COLOR)
						dest[u+5] = tbyte;
					if ( (tbyte=source[u+6]) != TRANSPARENT_COLOR)
						dest[u+6] = tbyte;
					if ( (tbyte=source[u+7]) != TRANSPARENT_COLOR)
						dest[u+7] = tbyte;
				}
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
	}
	else
	{
	// FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;

		for (v=0 ; v<pic->height ; v++)
		{
			for (u=0 ; u<pic->width ; u++)
			{
				tbyte = source[u];

				if (tbyte != TRANSPARENT_COLOR)
				{
					pusdest[u] = d_8to16table[tbyte];
				}
			}

			pusdest += vid.rowbytes >> 1;
			source += pic->width;
		}
	}
}

/*

void Draw_TransPic_Scaled (int x, int y, qpic_t *pic)
{
	byte		*source, tbyte;
	int		v, u, s;

	float vmax, umax;

	if (x < 0 || (x + pic->width) > vid.vconwidth ||
	    y < 0 || (y + pic->height) > vid.vconheight)
	{
		Sys_Error("%s: bad coordinates");
	}

	source = pic->data;

	vmax = pic->height * vid.height / (float)vid.vconheight;
	umax = pic->width * vid.width / (float)vid.vconwidth;

	{
		byte *dest = vid.buffer + (y * vid.height / vid.vconheight) * vid.rowbytes
			+ (x * vid.width / vid.vconwidth);
		for (v = 0; v < vmax; v++)
		{
			for (u = 0; u < umax; u++)
			{
				s = u * vid.vconwidth / vid.width 
					+ (v * vid.vconheight / vid.height) * pic->width;
		
					if ((tbyte = source[s]) != TRANSPARENT_COLOR)
					{
						dest[u] = tbyte;
					}
		
				
			}

			dest += vid.rowbytes;
		}
	} 

}
*/
void Draw_TransPic_Scaled (int x, int y, qpic_t *pic)
{
	byte		*source, tbyte;
	int		v, u, s;
	float vmax, umax;

	if (x < 0 || (x + pic->width) > vid.vconwidth ||
	    y < 0 || (y + pic->height) > vid.vconheight)
	{
		Sys_Error("%s: bad coordinates");
	}

	source = pic->data;

	vmax = pic->height * vid.height / (float)vid.vconheight;
	umax = pic->width * vid.width / (float)vid.vconwidth;
	
	
	{
		byte *dest = vid.buffer + (y * vid.height / vid.vconheight) * vid.rowbytes
			+ (x * vid.width / vid.vconwidth);

		for (v = 0; v < vmax; v++)
		{
			for (u = 0; u < umax; u++)
			{
					s = u * vid.vconwidth / vid.width + (v * vid.vconheight / vid.height) * pic->width;
		
					if ((tbyte = source[s]) != TRANSPARENT_COLOR)
					{
						dest[u] = tbyte;
					}
		
				
			}

			dest += vid.rowbytes;
		}
	} 

}



void Draw_TransPic_Scaled_Two (int x, int y, qpic_t *pic, float scel)
{
	byte		*source, tbyte;
	int		v, u, s;
	float vmax, umax;
	float scal;
	int	  scol;
	if (x < 0 || (x + pic->width) > vid.vconwidth ||
	    y < 0 || (y + pic->height) > vid.vconheight)
	{
		Sys_Error("%s: bad coordinates");
	}

	source = pic->data;

	scel = 2;

	vmax = pic->height * vid.height / (float)vid.vconheight;
	umax = pic->width * (scel / (scel * 2)) * vid.width / (float)vid.vconwidth;
	
	
	{
		byte *dest = vid.buffer + (y * vid.height / vid.vconheight) * vid.rowbytes
			+ (x * vid.width / vid.vconwidth);

		for (v = 0; v < vmax; v+=scel)
		{
			
			for (u = 0; u < umax; u++)
			{
					s = u * scel * vid.vconwidth / vid.width + (v * vid.vconheight / vid.height) * pic->width;
		
					if ((tbyte = source[s]) != TRANSPARENT_COLOR)
					{
						dest[u] = tbyte;
					}
		
				
			}

			dest += vid.rowbytes;
		}
	} 

}
//==========================================================================
//
// Draw_TransPicCropped
//
// Draws a holey qpic_t that is clipped at the bottom edge of the screen.
//
//==========================================================================

void Draw_TransPicCropped (int x, int y, qpic_t *pic)
{
	byte		*source, tbyte;
	int		v, u, height;

	if ((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("%s: bad coordinates");
	}

	if (y >= vid.height || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

	if (y+pic->height > vid.height)
	{
		height = vid.height-y;
	}
	else if (y < 0)
	{
		height = pic->height+y;
	}
	else
	{
		height = pic->height;
	}

	source = pic->data;
	if (y < 0)
	{
		source += (pic->width * (-y));
		y = 0;
	}

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y * vid.rowbytes + x;
		if (pic->width & 7)
		{ // General
				for (v = 0; v < height; v++)
				{
					for (u = 0; u < pic->width; u++)
					{
						if ((tbyte = source[u]) != TRANSPARENT_COLOR)
						{
							dest[u] = tbyte;
						}
					}
					dest += vid.rowbytes;
					source += pic->width;
				}

		}
		else
		{ // Unwound
			for (v = 0; v < height; v++)
			{
				for (u = 0; u < pic->width; u += 8)
				{
					if ((tbyte = source[u]) != TRANSPARENT_COLOR)
							dest[u] = tbyte;
						if ((tbyte = source[u+1]) != TRANSPARENT_COLOR)
							dest[u+1] = tbyte;
						if ((tbyte = source[u+2]) != TRANSPARENT_COLOR)
							dest[u+2] = tbyte;
						if ((tbyte = source[u+3]) != TRANSPARENT_COLOR)
							dest[u+3] = tbyte;
						if ((tbyte = source[u+4]) != TRANSPARENT_COLOR)
							dest[u+4] = tbyte;
						if ((tbyte = source[u+5]) != TRANSPARENT_COLOR)
							dest[u+5] = tbyte;
						if ((tbyte = source[u+6]) != TRANSPARENT_COLOR)
							dest[u+6] = tbyte;
						if ((tbyte = source[u+7]) != TRANSPARENT_COLOR)
							dest[u+7] = tbyte;
				
				}
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes>>1) + x;
		// FIXME: transparency bits are missing
		for (v = 0; v < height; v++)
		{
			for (u = 0; u < pic->width; u++)
			{
				tbyte = source[u];
				if (tbyte != TRANSPARENT_COLOR)
				{
					dest[u] = d_8to16table[tbyte];
				}
			}
			dest += vid.rowbytes>>1;
			source += pic->width;
		}
	}
}

void Draw_TransPicCropped_Scaled (int x, int y, qpic_t *pic)
{
	byte		*source, tbyte;
	int		v, u, height, s;

	float vmax, umax;

	if ((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("%s: bad coordinates");
	}

	if (y >= vid.vconheight || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

	if (y+pic->height > vid.vconheight)
	{
		height = vid.vconheight-y;
	}
	else if (y < 0)
	{
		height = pic->height+y;
	}
	else
	{
		height = pic->height;
	}

	source = pic->data;
	if (y < 0)
	{
		source += (pic->width * (-y));
		y = 0;
	}

	vmax = height * vid.height / (float)vid.vconheight;
	umax = pic->width * vid.width / (float)vid.vconwidth;

	{
		byte *dest = vid.buffer + (y * vid.height/vid.vconheight) * vid.rowbytes
			+ (x * vid.width / vid.vconwidth);
		for (v = 0; v < vmax; v++)
		{
			for (u = 0; u < umax; u++)
			{
				s = u * vid.vconwidth / vid.width 
					+ (v * vid.vconheight / vid.height) * pic->width;
					if ((tbyte = source[s]) != TRANSPARENT_COLOR)
					{
						dest[u] = tbyte;
					}
		
			}

			dest += vid.rowbytes;
		}
	}

}


/*
=============
Draw_TransPicTranslate
=============
*/
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation)
{
	byte	*dest, *source, tbyte;
	unsigned short	*pusdest;
	int				v, u;

	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 ||
		 (unsigned)(y + pic->height) > vid.height)
	{
		Sys_Error ("Draw_TransPic: bad coordinates");
	}

	source = pic->data;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y * vid.rowbytes + x;

		if (pic->width & 7)
		{	// general
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u++)
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = translation[tbyte];

				dest += vid.rowbytes;
				source += pic->width;
			}
		}
		else
		{	// unwound
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u+=8)
				{
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = translation[tbyte];
					if ( (tbyte=source[u+1]) != TRANSPARENT_COLOR)
						dest[u+1] = translation[tbyte];
					if ( (tbyte=source[u+2]) != TRANSPARENT_COLOR)
						dest[u+2] = translation[tbyte];
					if ( (tbyte=source[u+3]) != TRANSPARENT_COLOR)
						dest[u+3] = translation[tbyte];
					if ( (tbyte=source[u+4]) != TRANSPARENT_COLOR)
						dest[u+4] = translation[tbyte];
					if ( (tbyte=source[u+5]) != TRANSPARENT_COLOR)
						dest[u+5] = translation[tbyte];
					if ( (tbyte=source[u+6]) != TRANSPARENT_COLOR)
						dest[u+6] = translation[tbyte];
					if ( (tbyte=source[u+7]) != TRANSPARENT_COLOR)
						dest[u+7] = translation[tbyte];
				}
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
	}
	else
	{
	// FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;

		for (v=0 ; v<pic->height ; v++)
		{
			for (u=0 ; u<pic->width ; u++)
			{
				tbyte = source[u];

				if (tbyte != TRANSPARENT_COLOR)
				{
					pusdest[u] = d_8to16table[tbyte];
				}
			}

			pusdest += vid.rowbytes >> 1;
			source += pic->width;
		}
	}
}


/*
=============
Draw_TransPicTranslate_Scaled
=============
*/


void Draw_TransPicTranslate_Scaled (int x, int y, qpic_t *pic, byte *translation)
{
	byte		*source, tbyte;
	int		v, u, s;

	float vmax, umax;

	if (x < 0 || (x + pic->width) > vid.vconwidth ||
	    y < 0 || (y + pic->height) > vid.vconheight)
	{
		Sys_Error("%s: bad coordinates");
	}

	source = pic->data;

	vmax = pic->height * vid.height / (float)vid.vconheight;
	umax = pic->width * vid.width / (float)vid.vconwidth;

	{
		byte *dest = vid.buffer + (y * vid.height / vid.vconheight) * vid.rowbytes
			+ (x * vid.width / vid.vconwidth);
		for (v = 0; v < vmax; v++)
		{
			for (u = 0; u < umax; u++)
			{
				s = u * vid.vconwidth / vid.width 
					+ (v * vid.vconheight / vid.height) * pic->width;
		
					if ((tbyte = source[s]) != TRANSPARENT_COLOR)
					{
						dest[u] = translation[tbyte];
					}
		
				
			}

			dest += vid.rowbytes;
		}
	} 

}


void Draw_TransPicTranslatde_Scaled (int x, int y, qpic_t *pic, byte *translation)
{
	byte	*dest, *source, tbyte;
	unsigned short	*pusdest;
	int				v, u;

	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 ||
		 (unsigned)(y + pic->height) > vid.height)
	{
		Sys_Error ("Draw_TransPic: bad coordinates");
	}

	source = pic->data;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y * vid.rowbytes + x;

		if (pic->width & 7)
		{	// general
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u++)
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = translation[tbyte];

				dest += vid.rowbytes;
				source += pic->width;
			}
		}
		else
		{	// unwound
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u+=8)
				{
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = translation[tbyte];
					if ( (tbyte=source[u+1]) != TRANSPARENT_COLOR)
						dest[u+1] = translation[tbyte];
					if ( (tbyte=source[u+2]) != TRANSPARENT_COLOR)
						dest[u+2] = translation[tbyte];
					if ( (tbyte=source[u+3]) != TRANSPARENT_COLOR)
						dest[u+3] = translation[tbyte];
					if ( (tbyte=source[u+4]) != TRANSPARENT_COLOR)
						dest[u+4] = translation[tbyte];
					if ( (tbyte=source[u+5]) != TRANSPARENT_COLOR)
						dest[u+5] = translation[tbyte];
					if ( (tbyte=source[u+6]) != TRANSPARENT_COLOR)
						dest[u+6] = translation[tbyte];
					if ( (tbyte=source[u+7]) != TRANSPARENT_COLOR)
						dest[u+7] = translation[tbyte];
				}
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
	}
	else
	{
	// FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;

		for (v=0 ; v<pic->height ; v++)
		{
			for (u=0 ; u<pic->width ; u++)
			{
				tbyte = source[u];

				if (tbyte != TRANSPARENT_COLOR)
				{
					pusdest[u] = d_8to16table[tbyte];
				}
			}

			pusdest += vid.rowbytes >> 1;
			source += pic->width;
		}
	}
}


void Draw_CharToConback (int num, byte *dest)
{
	int		row, col;
	byte	*source;
	int		drawline;
	int		x;

	row = num>>4;
	col = num&15;
	source = draw_chars + (row<<10) + (col<<3);

	drawline = 8;

	while (drawline--)
	{
		for (x=0 ; x<8 ; x++)
			if (source[x])
				dest[x] = 0x60 + source[x];
		source += 128;
		dest += 320;
	}

}

/*
================
Draw_ConsoleBackground

================
*/
void Draw_ConsoleBackground (int lines)
{
	int				x, y, v;
	byte			*src, *dest;
	unsigned short	*pusdest;
	int				f, fstep;
	qpic_t			*conback;
	char			ver[100];

	conback = Draw_CachePic ("gfx/conback.lmp");

// hack the version number directly into the pic
#ifdef _WIN32
//	sprintf (ver, "(%s, WinQuake) %4.2f", QIP_VERSION, (float)VERSION);	// 2001-10-25 QIP version in the console background by Maddes

//	dest = conback->data + 320*186 + 320 - 11 - 8*strlen(ver);	// 2001-10-25 Version printing fix by Maddes
#elif defined(X11)
//	sprintf (ver, "(%s, X11 Quake %2.2f) %4.2f", QIP_VERSION, (float)X11_VERSION, (float)VERSION);	// 2001-10-25 QIP version in the console background by Maddes

//	dest = conback->data + 320*186 + 320 - 11 - 8*strlen(ver);	// 2001-10-25 Version printing fix by Maddes
#elif defined(__linux__)
//	sprintf (ver, "(%s, Linux Quake %2.2f) %4.2f", QIP_VERSION, (float)LINUX_VERSION, (float)VERSION);	// 2001-10-25 QIP version in the console background by Maddes

//	dest = conback->data + 320*186 + 320 - 11 - 8*strlen(ver);	// 2001-10-25 Version printing fix by Maddes
#else
//	sprintf (ver, "(%s) %4.2f", QIP_VERSION, VERSION);	// 2001-10-25 QIP version in the console background by Maddes

	//	dest = conback->data + 320 - 43 + 320*186;	// 2001-10-25 Version printing fix by Maddes
#endif
	sprintf (ver, "%4.2f", VERSION);	// 2001-10-25 QIP version in the console background by Maddes
	dest = conback->data + 320*186 + 320 - 11 - 8*strlen(ver);	// 2001-10-25 Version printing fix by Maddes

	for (x=0 ; x<strlen(ver) ; x++)
		Draw_CharToConback (ver[x], dest+(x<<3));

// draw the pic
	{
		dest = vid.conbuffer;

		for (y=0 ; y<lines ; y++, dest += vid.conrowbytes)
		{
			v = (vid.conheight - lines + y)*200/vid.conheight;
			src = conback->data + v*320;
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  start
				f = 0;
				fstep = 320*0x10000/vid.conwidth;

			if (con_forcedup)
			{
				for (x = 0; x < vid.conwidth; x++, f+=fstep )
					dest[x] = src[f >> 16];
			}
			else
			{
				int	t;

				t = con_alpha->value * (float)CON_ALPHASTATES;
				if (t >= CON_ALPHASTATES)
				{
					t = CON_ALPHASTATES - 1;
				}
				t = fademask[t]>>((y&1)<<2);

				for (x=0 ; x<vid.conwidth ; x+=4)
				{
					if (t&1) dest[x]   = src[f>>16]; f+=fstep;
					if (t&2) dest[x+1] = src[f>>16]; f+=fstep;
					if (t&4) dest[x+2] = src[f>>16]; f+=fstep;
					if (t&8) dest[x+3] = src[f>>16]; f+=fstep;
// 2000-08-04 "Transparent" console background for software renderer by Norberto Alfredo Bensa/Maddes  end
				}
			}
		}
	}

}


/*
==============
R_DrawRect8
==============
*/
void R_DrawRect8 (vrect_t *prect, int rowbytes, byte *psrc,
	int transparent)
{
	byte	t;
	int		i, j, srcdelta, destdelta;
	byte	*pdest;

	pdest = vid.buffer + (prect->y * vid.rowbytes) + prect->x;

	srcdelta = rowbytes - prect->width;
	destdelta = vid.rowbytes - prect->width;

	if (transparent)
	{
		for (i=0 ; i<prect->height ; i++)
		{
			for (j=0 ; j<prect->width ; j++)
			{
				t = *psrc;
				if (t != TRANSPARENT_COLOR)
				{
					*pdest = t;
				}

				psrc++;
				pdest++;
			}

			psrc += srcdelta;
			pdest += destdelta;
		}
	}
	else
	{
		for (i=0 ; i<prect->height ; i++)
		{
			memcpy (pdest, psrc, prect->width);
			psrc += rowbytes;
			pdest += vid.rowbytes;
		}
	}
}


/*
==============
R_DrawRect16
==============
*/
void R_DrawRect16 (vrect_t *prect, int rowbytes, byte *psrc,
	int transparent)
{
	byte			t;
	int				i, j, srcdelta, destdelta;
	unsigned short	*pdest;

// FIXME: would it be better to pre-expand native-format versions?

	pdest = (unsigned short *)vid.buffer +
			(prect->y * (vid.rowbytes >> 1)) + prect->x;

	srcdelta = rowbytes - prect->width;
	destdelta = (vid.rowbytes >> 1) - prect->width;

	if (transparent)
	{
		for (i=0 ; i<prect->height ; i++)
		{
			for (j=0 ; j<prect->width ; j++)
			{
				t = *psrc;
				if (t != TRANSPARENT_COLOR)
				{
					*pdest = d_8to16table[t];
				}

				psrc++;
				pdest++;
			}

			psrc += srcdelta;
			pdest += destdelta;
		}
	}
	else
	{
		for (i=0 ; i<prect->height ; i++)
		{
			for (j=0 ; j<prect->width ; j++)
			{
				*pdest = d_8to16table[*psrc];
				psrc++;
				pdest++;
			}

			psrc += srcdelta;
			pdest += destdelta;
		}
	}
}


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h)
{
	int				width, height, tileoffsetx, tileoffsety;
	byte			*psrc;
	vrect_t			vr;

	r_rectdesc.rect.x = x;
	r_rectdesc.rect.y = y;
	r_rectdesc.rect.width = w;
	r_rectdesc.rect.height = h;

	vr.y = r_rectdesc.rect.y;
	height = r_rectdesc.rect.height;

	tileoffsety = vr.y % r_rectdesc.height;

	while (height > 0)
	{
		vr.x = r_rectdesc.rect.x;
		width = r_rectdesc.rect.width;

		if (tileoffsety != 0)
			vr.height = r_rectdesc.height - tileoffsety;
		else
			vr.height = r_rectdesc.height;

		if (vr.height > height)
			vr.height = height;

		tileoffsetx = vr.x % r_rectdesc.width;

		while (width > 0)
		{
			if (tileoffsetx != 0)
				vr.width = r_rectdesc.width - tileoffsetx;
			else
				vr.width = r_rectdesc.width;

			if (vr.width > width)
				vr.width = width;

			psrc = r_rectdesc.ptexbytes +
					(tileoffsety * r_rectdesc.rowbytes) + tileoffsetx;

			if (r_pixbytes == 1)
			{
				R_DrawRect8 (&vr, r_rectdesc.rowbytes, psrc, 0);
			}
			else
			{
				R_DrawRect16 (&vr, r_rectdesc.rowbytes, psrc, 0);
			}

			vr.x += vr.width;
			width -= vr.width;
			tileoffsetx = 0;	// only the left tile can be left-clipped
		}

		vr.y += vr.height;
		height -= vr.height;
		tileoffsety = 0;		// only the top tile can be top-clipped
	}
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	byte			*dest;
	unsigned short	*pusdest;
	unsigned		uc;
	int				u, v;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y*vid.rowbytes + x;
		for (v=0 ; v<h ; v++, dest += vid.rowbytes)
			for (u=0 ; u<w ; u++)
				dest[u] = c;
	}
	else
	{
		uc = d_8to16table[c];

		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;
		for (v=0 ; v<h ; v++, pusdest += (vid.rowbytes >> 1))
			for (u=0 ; u<w ; u++)
				pusdest[u] = uc;
	}
}


void Draw_Fill_Scaled (int x, int y, int w, int h, int c)
{
	int		u, v;
	float umax, vmax;

	x = (x * vid.width) / vid.vconwidth;
	y = (y * vid.height) / vid.vconheight;

	if (x < 0 || x + w > vid.width ||
	    y < 0 || y + h > vid.height)
	{
		Con_Printf("Bad Draw_Fill SCALED(%d, %d, %d, %d, %c)\n", x, y, w, h, c);
		return;
	}

	vmax = h * vid.height / (float)vid.vconheight;
	umax = w * vid.width / (float)vid.vconwidth;

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y*vid.rowbytes + x;


			for (v = 0; v < vmax; v++, dest += vid.rowbytes)
			{
				for (u = 0; u < umax; u++)
				{
					dest[u] = c;
				}
			}
		
	}

}

//=============================================================================

extern cvar_t *r_menucolor;
extern cvar_t *r_tingecolor;


/*
================
Draw_FadeScreen

================
*/

	byte		*obuf;


void Draw_FadeScreen (void)
{
	int			x,y;
	byte		*pbuf;
	int		mycol;


	mycol = (int)r_menucolor->value;
	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();


	for (y=0 ; y<vid.height ; y++)
	{
		int	t;

		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		t = (y & 1) << 1;
		for (x=0 ; x<vid.width ; x++)
		{
			// Classic 0.8-1.06 look
			if (mycol < 15){
				pbuf[x] = menumap[pbuf[x]][mycol];	// new menu tint
			}
			else			if (mycol == 17){
				pbuf[x] = transTable[pbuf[x]][0];	// alpha blend a black
			}
			else			if (mycol == 18){
				pbuf[x] = transTable[42+rand()&5][ (pbuf[x])];
			}
			else			if (mycol == 19){
				pbuf[x] = transTable[pbuf[x+rand()&5]][ (pbuf[x+(rand()&(int)temp2->value)])];
			}

			else
			{
			// stupid v1.08 look:

			if ((x & 3) != t)
				pbuf[x] = 0;
			}
		}


		
	}
	 
/*
	// Weird blur experiment, which unfortunately, causes a crash in a higher resolution.
	{

	byte		*pbuf2[8];

		for (y=0 ; y<vid.height ; y+=8)
		{
			int	t, e;

			
			pbuf2[0] = (byte *)(vid.buffer + vid.rowbytes*(y));
			pbuf2[1] = (byte *)(vid.buffer + vid.rowbytes*(y+1));
			pbuf2[2] = (byte *)(vid.buffer + vid.rowbytes*(y+2));
			pbuf2[3] = (byte *)(vid.buffer + vid.rowbytes*(y+3));
			pbuf2[4] = (byte *)(vid.buffer + vid.rowbytes*(y+4));
			pbuf2[5] = (byte *)(vid.buffer + vid.rowbytes*(y+5));
			pbuf2[6] = (byte *)(vid.buffer + vid.rowbytes*(y+6));
			pbuf2[7] = (byte *)(vid.buffer + vid.rowbytes*(y+7));
			pbuf2[8] = (byte *)(vid.buffer + vid.rowbytes*(y+8));
			
		//	t = (y & 1) << 1;
			for (x=0 ; x<vid.width ; x+=8)
			{
			//	for (t=0;t<9; t++)
			//		for (e=0;e<9; e++)
			//			pbuf2[t][x+e] = pbuf2[t][e+x];
				//pbuf2[t][x+e] = transTable[pbuf2[t][x+8]][pbuf2[8-t][x]];
					
			}
		}
	}
*/
	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}


// 12/01/14 LEILEI
// Sort of a parody of current gen console shooters
// Blends the screen in brown, which is blended additively AND translucency
// 3 lookup tables working on a single pixel = slow, just like the consoles

void Draw_DudeScreen (void)
{
	int			x,y;
	byte		*pbuf;
	int		mycol;

	mycol = (int)r_tingecolor->value;
	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();

	for (y=0 ; y<vid.height ; y++)
	{
		int	t;

		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		t = (y & 1) << 1;

		for (x=0 ; x<vid.width ; x++)
		{
		
				pbuf[x] = addTable[menumap[pbuf[x]][mycol]][pbuf[x]];	// new menu tint
		}
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}

void Draw_DudeScreen66 (void)
{
	int			x,y;
	byte		*pbuf;
	int		mycol;

	mycol = (int)r_tingecolor->value;
	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();

	for (y=0 ; y<vid.height ; y++)
	{
		int	t;

		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		t = (y & 1) << 1;

		for (x=0 ; x<vid.width ; x++)
		{
		
			pbuf[x] = transTable[pbuf[x]][addTable[menumap[pbuf[x]][mycol]][pbuf[x]]];	// new menu tint
		}
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}

void Draw_DudeScreen33 (void)
{
	int			x,y;
	byte		*pbuf;
	int		mycol;

	mycol = (int)r_tingecolor->value;
	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();

	for (y=0 ; y<vid.height ; y++)
	{
		int	t;

		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		t = (y & 1) << 1;

		for (x=0 ; x<vid.width ; x++)
		{
		
				pbuf[x] = transTable[addTable[menumap[pbuf[x]][mycol]][pbuf[x]]][pbuf[x]];	// new menu tint
		}
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}


void Draw_DudeScreen66A (void)
{
	int			x,y;
	byte		*pbuf;
	int		mycol;

	mycol = (int)r_tingecolor->value;
	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();

	for (y=0 ; y<vid.height ; y++)
	{
		int	t;

		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		t = (y & 1) << 1;

		for (x=0 ; x<vid.width ; x++)
		{
		
			pbuf[x] = transTable[pbuf[x]][menumap[pbuf[x]][mycol]];	// new menu tint
		}
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}

void Draw_DudeScreen33A (void)
{
	int			x,y;
	byte		*pbuf;
	int		mycol;

	mycol = (int)r_tingecolor->value;
	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();

	for (y=0 ; y<vid.height ; y++)
	{
		int	t;

		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		t = (y & 1) << 1;

		for (x=0 ; x<vid.width ; x++)
		{
		
				pbuf[x] = transTable[menumap[pbuf[x]][mycol]][pbuf[x]];	// new menu tint
		}
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}


//	 low Detail
void Draw_Bloom (void)
{
	int			x,y;
	byte		*pbuf;
	int		mycol;
	int		weedth = 12;
	return;
//	mycol = (int)r_tingecolor->value;
//	VID_UnlockBuffer ();
//	S_ExtraUpdate ();
//	VID_LockBuffer ();

	for (y=0 ; y<r_refdef.vrect.height ; y++)
	{
		int	t, e;

		pbuf = (byte *)(vid.buffer + vid.rowbytes * y);

		for (x=0 ; x<vid.width ; x+=weedth)
		{
			for(e=0;e<weedth;e++){
				pbuf[x+e] = pbuf[x];
				
			}
		}
	}

//	VID_UnlockBuffer ();
//	S_ExtraUpdate ();
//	VID_LockBuffer ();
}


// Dunno what this'll be.
void Draw_Something (void)
{
	int			x,y;
	byte		*pbuf;
	byte		*pbaf;
	int		mycol;
	int phil = 4;
	int	philclampx;
	int	philclampy;
	float	splet;
	int	bloome;
	int ee;
	mycol = (int)r_tingecolor->value;
	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
	splet = vid.height;
	bloome = 16300;
	
//	for (ee=0; ee<vid.width*vid.height; ee++)
	//	pbaf[ee] = vid.buffer + ee;
	//pbaf = (byte *)(vid.buffer + vid.rowbytes*y);
	for (y=0 ; y<vid.height ; y++)
	{
		int	t;

		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		pbaf = (byte *)(vid.buffer + vid.rowbytes*y);
		
		t = (y & 1) << 1;

		for (phil=0 ; phil<12; phil++)
		for (x=0 ; x<vid.width ; x+=2)
		{
				
				//pbuf[x] = addTable[44][pbuf[x]];	 
			//	pbuf[x] =  transTable
			//		[transTable [pbuf[x - (vid.width * phil)]][pbuf[x - phil]]]
			//		[transTable[pbuf[x + (vid.width * phil)]][pbuf[x + phil]]];
			//	if (x > vid.width)
				philclampx = vid.width * phil;
				philclampy = vid.width * -phil;
				if (philclampx < 0) philclampx = 0;
				if (philclampx > vid.width * vid.height) philclampx = vid.width;
				if (philclampy < 0) philclampy = 0;
				if (philclampy > vid.width * vid.height) philclampy = vid.width;


				pbuf[x] = pbuf[x + (rand()&phil)];
				pbuf[x+1] = pbuf[x + 1 + (rand()&phil)];

				pbuf[x] = transTable[pbuf[x+1]][pbuf[x]];

			//	pbuf[x] = host_colormap[pbuf[x] + (14000 & 0xFF00)]; 
	/*			
				pbuf[x] =  addTable[transTable
					[transTable [fbs[pbuf[x + (philclampx)]]][	 fbs[pbuf[x + - phil]] ]]
					[transTable [fbs[pbuf[x + (philclampy)]]][fbs[pbuf[x + phil]] ]]
					][pbuf[x]];
			
				pbuf[x+1] =  addTable[transTable
					[transTable [fbs[pbuf[x + (philclampy)]]][fbs[pbuf[x + phil] ]]]
					[transTable [fbs[pbuf[x + (philclampx)]]][fbs[pbuf[x - phil] ]]]
					][pbuf[x+1]];
*/
		//		pbuf[x+1] =  transTable
		//			[transTable [pbuf[x + (philclampy)]][pbuf[x + phil]]]
			//		[transTable[pbuf[x + (philclampx)]][pbuf[x - phil]]];	
/*
				pbuf[x] =  transTable
					[transTable [pbuf[x + (philclampx)]][pbuf[x - phil]]]
					[transTable[pbuf[x + (philclampy)]][pbuf[x + phil]]];
				pbuf[x+1] =  transTable
					[transTable [pbuf[x + (philclampy)]][pbuf[x + phil]]]
					[transTable[pbuf[x + (philclampx)]][pbuf[x - phil]]];						
*/

			//		pbuf[x] = pbuf[x + (int)splet];
			//		pbuf[x+1] = pbuf[x - (int)splet];
				//	pbuf[x+2] = pbuf[x + (int)splet];
			//		pbuf[x+3] = pbuf[x - (int)splet];
				//	transTable [pbuf[x - (vid.width * 2)]][pbuf[x - 2]]

			//	transTable[pbuf[x + (vid.width * 2)]][pbuf[x + 2]]
					//[pbuf[x + (vid.width * 2)]];
					
		}
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}

//=============================================================================

/*
================
Draw_BeginDisc

Draws the little blue disc in the corner of the screen.
Call before beginning any disc IO.
================
*/
void Draw_BeginDisc (void)
{

	D_BeginDirectRect (vid.width - 24, 0, draw_disc->data, 24, 24);
}


/*
================
Draw_EndDisc

Erases the disc icon.
Call after completing any disc IO
================
*/
void Draw_EndDisc (void)
{

	D_EndDirectRect (vid.width - 24, 0, 24, 24);
}



