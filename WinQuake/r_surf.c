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
// r_surf.c: surface-related refresh code

#include "quakedef.h"
#include "r_local.h"
// hmm
drawsurf_t	r_drawsurf;

int				lightleft, sourcesstep, blocksize, sourcetstep;
int				lightdelta, lightdeltastep;
int				lightright, lightleftstep, lightrightstep, blockdivshift;


int				lightlefta[3];
int				sourcesstep, blocksize, sourcetstep;
int				lightrighta[3];
int				lightleftstepa[3], lightrightstepa[3], blockdivshift;


extern cvar_t *temp2;

unsigned		blockdivmask;
void			*prowdestbase;

unsigned char	*pbasesource;
unsigned int	*pbasesource24;
int				surfrowbytes;	// used by ASM files
unsigned int			dynlightenabled;	// r_dynamic


int				r_stepback;
int				r_lightwidth;
int				r_lightwidther;
int				r_numhblocks, r_numvblocks;
unsigned char	*r_source, *r_sourcemax;
unsigned long	*r_source24;
unsigned char	*r_sourcemax24;

int		*r_lightptr;


int		*r_lightptr_r;
int		*r_lightptr_g;
int		*r_lightptr_b;
int	coloredmethod;
extern byte	palmap[32][32][32];	
extern unsigned char	palmap3[65535];


static char	 *rgb_colormap_red;
static char	 *rgb_colormap_green;
static char	 *rgb_colormap_blue;


void PalmapStaticized ()
{
	int i, j, k;

	rgb_colormap_red = host_colormap_red;
	rgb_colormap_green = host_colormap_green;
	rgb_colormap_blue = host_colormap_blue;


}


// test

int		blocklights[18*18*3]; // LordHavoc: .lit support (*3 for RGB)


void R_DrawSurfaceBlock8_mip0 (void);
void R_DrawSurfaceBlock8_mip1 (void);
void R_DrawSurfaceBlock8_mip2 (void);
void R_DrawSurfaceBlock8_mip3 (void);

void R_DrawSurfaceBlock8Fast_mip0 (void);
void R_DrawSurfaceBlock8Fast_mip1 (void);
void R_DrawSurfaceBlock8Fast_mip2 (void);
void R_DrawSurfaceBlock8Fast_mip3 (void);




#ifdef id386rgb

void D_DrawSurfaceBlock8RGBASM_mip0(void);
void D_DrawSurfaceBlock8RGBASM_mip1(void);
void D_DrawSurfaceBlock8RGBASM_mip2(void);
void D_DrawSurfaceBlock8RGBASM_mip3(void);

#endif

/*

void D_DrawSurfaceBlock8RED_mip0(void);
void D_DrawSurfaceBlock8RED_mip1(void);
void D_DrawSurfaceBlock8RED_mip2(void);
void D_DrawSurfaceBlock8RED_mip3(void);

void D_DrawSurfaceBlock8GREEN_mip0(void);
void D_DrawSurfaceBlock8GREEN_mip1(void);
void D_DrawSurfaceBlock8GREEN_mip2(void);
void D_DrawSurfaceBlock8GREEN_mip3(void);

void D_DrawSurfaceBlock8BLUE_mip0(void);
void D_DrawSurfaceBlock8BLUE_mip1(void);
void D_DrawSurfaceBlock8BLUE_mip2(void);
void D_DrawSurfaceBlock8BLUE_mip3(void);
*/

//	8-bit target - 8-bit textures  - colored lighting - nearest

void D_DrawSurfaceBlock8RGBX_mip0(void);
void D_DrawSurfaceBlock8RGBX_mip1(void);
void D_DrawSurfaceBlock8RGBX_mip2(void);
void D_DrawSurfaceBlock8RGBX_mip3(void);


//	8-bit target - 8-bit textures  - colored lighting - nearest - lookup method

void D_DrawSurfaceBlock888RGBX_mip0(void);
void D_DrawSurfaceBlock888RGBX_mip1(void);
void D_DrawSurfaceBlock888RGBX_mip2(void);
void D_DrawSurfaceBlock888RGBX_mip3(void);


//	8-bit target - 8-bit textures  - colored lighting - dithered

void D_DrawSurfaceBlock8RGBXD_mip0(void);
void D_DrawSurfaceBlock8RGBXD_mip1(void);
void D_DrawSurfaceBlock8RGBXD_mip2(void);
void D_DrawSurfaceBlock8RGBXD_mip3(void);

//	8-bit target - 24-bit textures - colored lighting - dithered

void D_DrawSurfaceBlock824RGBXD_mip0(void);
void D_DrawSurfaceBlock824RGBXD_mip1(void);
void D_DrawSurfaceBlock824RGBXD_mip2(void);
void D_DrawSurfaceBlock824RGBXD_mip3(void);

//	8-bit target - 24-bit textures - colored lighting - nearest

void D_DrawSurfaceBlock824RGBX_mip0(void);
void D_DrawSurfaceBlock824RGBX_mip1(void);
void D_DrawSurfaceBlock824RGBX_mip2(void);
void D_DrawSurfaceBlock824RGBX_mip3(void);


static void	(*surfmiptable[4])(void) = {
	R_DrawSurfaceBlock8_mip0,
	R_DrawSurfaceBlock8_mip1,
	R_DrawSurfaceBlock8_mip2,
	R_DrawSurfaceBlock8_mip3
};

#ifdef EXPREND
static void	(*surfmiptableshadow[4])(void) = {
	R_DrawSurfaceBlock8S_mip0,
	R_DrawSurfaceBlock8S_mip1,
	R_DrawSurfaceBlock8S_mip2,
	R_DrawSurfaceBlock8S_mip3
};

static void	(*surfmiptablediffuse[4])(void) = {
	R_DrawSurfaceBlock8D_mip0,
	R_DrawSurfaceBlock8D_mip1,
	R_DrawSurfaceBlock8D_mip2,
	R_DrawSurfaceBlock8D_mip3
};

#endif

static void	(*surfmiptable8RGBX[4])(void) =
{
#if id386rgb
	R_DrawSurfaceBlock8RGBASM_mip0,
	R_DrawSurfaceBlock8RGBASM_mip1,
	R_DrawSurfaceBlock8RGBASM_mip2,
	R_DrawSurfaceBlock8RGBASM_mip3
#else
	R_DrawSurfaceBlock8RGBX_mip0,
	R_DrawSurfaceBlock8RGBX_mip1,
	R_DrawSurfaceBlock8RGBX_mip2,
	R_DrawSurfaceBlock8RGBX_mip3
#endif
};





static void	(*surfmiptable8RGBXD[4])(void) =
{

	R_DrawSurfaceBlock8RGBXD_mip0,
	R_DrawSurfaceBlock8RGBXD_mip1,
	R_DrawSurfaceBlock8RGBXD_mip2,
	R_DrawSurfaceBlock8RGBXD_mip3
};



static void	(*surfmiptable888RGB[4])(void) =
{

	R_DrawSurfaceBlock888RGB_mip0,
	R_DrawSurfaceBlock888RGB_mip1,
	R_DrawSurfaceBlock888RGB_mip2,
	R_DrawSurfaceBlock888RGB_mip3
};


#ifdef twentyfourbithack
static void	(*surfmiptable824RGBXD[4])(void) =
{

	R_DrawSurfaceBlock824RGBXD_mip0,
	R_DrawSurfaceBlock824RGBXD_mip1,
	R_DrawSurfaceBlock824RGBXD_mip2,
	R_DrawSurfaceBlock824RGBXD_mip3
};

static void	(*surfmiptable824RGBX[4])(void) =
{

	R_DrawSurfaceBlock824RGBX_mip0,
	R_DrawSurfaceBlock824RGBX_mip1,
	R_DrawSurfaceBlock824RGBX_mip2,
	R_DrawSurfaceBlock824RGBX_mip3
};
#endif
// leilei - fast versions of these
// are just the same, sans lightstepping
// which actually brings a cool speed boost
#if !id386
static void	(*surfmiptablefast[4])(void) = {
	R_DrawSurfaceBlock8_mip0,
	R_DrawSurfaceBlock8_mip1,
	R_DrawSurfaceBlock8_mip2,
	R_DrawSurfaceBlock8_mip3
};

// our c versions suck for the time being.
#else

static void	(*surfmiptablefast[4])(void) = {
	R_DrawSurfaceBlock8Fast_mip0,
	R_DrawSurfaceBlock8Fast_mip1,
	R_DrawSurfaceBlock8Fast_mip2,
	R_DrawSurfaceBlock8Fast_mip3
};

#endif



// MACROHARD!!!!!!!


// Macros for initiating the RGB light deltas.
#define MakeLightDelta() { light[0] =  lightrighta[0];	light[1] =  lightrighta[1];	light[2] =  lightrighta[2];};
#define PushLightDelta() { light[0] += lightdelta[0];	light[1] += lightdelta[1];	light[2] += lightdelta[2]; };
#define FinishLightDelta() { psource += sourcetstep; lightrighta[0] += lightrightstepa[0];lightlefta[0] += lightleftstepa[0];lightdelta[0] += lightdeltastep[0]; lightrighta[1] += lightrightstepa[1];lightlefta[1] += lightleftstepa[1];lightdelta[1] += lightdeltastep[1]; lightrighta[2] += lightrightstepa[2];lightlefta[2] += lightleftstepa[2];lightdelta[2] += lightdeltastep[2]; prowdest += surfrowbytes;}

// Low Colored Light Quality 
#define MIP888RGB(i) {prowdest[i] = palmap[rgb_colormap_red		[(light[0] & 0xFF00) + psource[i]]][rgb_colormap_green	[(light[1] & 0xFF00) + psource[i]]][rgb_colormap_blue		[(light[2] & 0xFF00) + psource[i]]]; }

// High Colored Light Quality 
#define MIP8RGBX(i) {  	if (psource[i] < host_fullbrights){ 	pix = psource[i]; pix24 = (unsigned char *)&d_8to24table[pix]; trans[0] = (pix24[0] * (light[0])) >> 17; trans[1] = (pix24[1] * (light[1])) >> 17; trans[2] = (pix24[2] * (light[2])) >> 17; if (trans[0] & ~63) trans[0] = 63; if (trans[1] & ~63) trans[1] = 63; if (trans[2] & ~63) trans[2] = 63; prowdest[i] = palmap2[trans[0]][trans[1]][trans[2]]; }	else prowdest[i] = psource[i];}

// Ultra Colored Light Quality (15bpp Span driver required)
#define MIP8RGBXD(i) { 				if (psource[i] < host_fullbrights) {  pix = psource[i]; pix24 = (unsigned char *)&d_8to24table[pix]; trans[0] = ((int)pix24[0] * light[0]) >> 18; trans[1] = ((int)pix24[1] * light[1]) >> 18;		trans[2] = ((int)pix24[2] * light[2]) >> 18; if (trans[0] & ~31) trans[0] = 31; if (trans[1] & ~31) trans[1] = 31; if (trans[2] & ~31) trans[2] = 31; prowdest[i] = (trans[0] << 10) | (trans[1] << 5) | trans[2];				}else{	pix = psource[i]; pix24 = (unsigned char *)&d_8to24table[pix]; trans[0] = ((int)pix24[0]) >> 3; trans[1] = ((int)pix24[1]) >> 3; trans[2] = ((int)pix24[2]) >> 3; prowdest[i] =  (trans[0] << 10) | (trans[1] << 5) | trans[2]; 	} }
#define MIP824RGBXD(i) { 		pix24 = (unsigned char *)&psource[i];	trans[0] = ((int)pix24[0] * light[0]) >> 18; trans[1] = ((int)pix24[1] * light[1]) >> 18; trans[2] = ((int)pix24[2] * light[2]) >> 18; if (trans[0] & ~31) trans[0] = 31; if (trans[1] & ~31) trans[1] = 31; if (trans[2] & ~31) trans[2] = 31; 	prowdest[i] = (trans[0] << 10) | (trans[1] << 5) | trans[2]; }

// yo dawg i heard you like macros so we put some macros in your macros while you macro

#define Mip0Stuff(i) { MakeLightDelta(); i(15); PushLightDelta(); i(14); PushLightDelta(); PushLightDelta(); i(13); PushLightDelta(); i(12); PushLightDelta(); i(11); PushLightDelta(); i(10); PushLightDelta(); i(9); PushLightDelta(); i(8); PushLightDelta(); i(7); PushLightDelta(); i(6); PushLightDelta(); i(5); PushLightDelta(); i(4); PushLightDelta(); i(3); PushLightDelta(); i(2); PushLightDelta(); i(1); PushLightDelta(); i(0);  FinishLightDelta();}
#define Mip1Stuff(i) { MakeLightDelta(); i(7); PushLightDelta(); i(6); PushLightDelta(); i(5); PushLightDelta(); i(4); PushLightDelta(); i(3); PushLightDelta(); i(2); PushLightDelta(); i(1); PushLightDelta(); i(0); FinishLightDelta();}
#define Mip2Stuff(i) { MakeLightDelta();i(3); PushLightDelta(); i(2); PushLightDelta(); i(1); PushLightDelta(); i(0); FinishLightDelta();}
#define Mip3Stuff(i) { MakeLightDelta(); i(1); PushLightDelta(); i(0); FinishLightDelta();}

/*
===============
R_AddDynamicLights
===============
*/
void R_AddDynamicLights (void)
{
	msurface_t *surf;
	int			lnum;
	int			sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local;
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;
	
	
	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		
		rad = cl_dlights[lnum].radius;
		if (cl_dlights[lnum].unmark)
			continue;		// don't add the one who doesn't want to be
	    
		VectorSubtract (cl_dlights[lnum].origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix
   //     dist = DotProduct (origin_for_ent, surf->plane->normal); // mankrip - dynamic lights on moving brush models fix - edited
		 dist = DotProduct (origin_for_ent, surf->plane->normal) - // mankrip - dynamic lights on moving brush models fix - edited
				surf->plane->dist;
		rad -= fabs(dist);

		minlight = cl_dlights[lnum].minlight;

		if (rad < minlight)
			continue;

		minlight = rad - minlight;

		impact[0] = origin_for_ent[0] - // mankrip - dynamic lights on moving brush models fix - edited
			surf->plane->normal[0]*dist;

			impact[1] = origin_for_ent[1] - // mankrip - dynamic lights on moving brush models fix - edited
					surf->plane->normal[1]*dist;
			impact[2] = origin_for_ent[2] - // mankrip - dynamic lights on moving brush models fix - edited
					surf->plane->normal[2]*dist;


		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];

		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)

				{
					unsigned temp;
					temp = (rad - dist)*256;
;
				
					i = t*smax + s;
					if (!cl_dlights[lnum].dark)
						blocklights[i] += temp;
					else
					{
						if (blocklights[i] > temp)
							blocklights[i] -= temp;
						else
							blocklights[i] = 0;
					}
				}

	


			}
		}
	}
}

void R_AddDynamicLightsRGB ()
{
	msurface_t *surf;

	int		lnum;
	int		sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local;
	int		s, t;
	int		i;
	int		smax, tmax;
	mtexinfo_t	*tex;
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix
	
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
	float		cred, cgreen, cblue, brightness;
	unsigned	*bl;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{

		

		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		rad = cl_dlights[lnum].radius;
		if (cl_dlights[lnum].unmark)
			continue;		// don't add the one who doesn't want to be

		VectorSubtract (cl_dlights[lnum].origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix

		dist = DotProduct (origin_for_ent, surf->plane->normal) - // mankrip - dynamic lights on moving brush models fix - edited
				surf->plane->dist;
		
		rad -= fabs(dist);
		minlight = cl_dlights[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;

		// mankrip - dynamic lights on moving brush models fix - edited
		impact[0] = origin_for_ent[0] - surf->plane->normal[0]*dist;
		impact[1] = origin_for_ent[1] - surf->plane->normal[1]*dist;
		impact[2] = origin_for_ent[2] - surf->plane->normal[2]*dist;



		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];

// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
		cred = cl_dlights[lnum].color[0] * 256.0f;
		cgreen = cl_dlights[lnum].color[1] * 256.0f;
		cblue = cl_dlights[lnum].color[2] * 256.0f;

		bl = blocklights;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end

		
					
		
		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)
				{
					brightness = rad - dist;
					bl[0] += (int) (brightness * cred);
					bl[1] += (int) (brightness * cgreen);
					bl[2] += (int) (brightness * cblue);
				}
				bl += 3;
			}
		}
		
	}

}











// leilei - shadowhack continued



// Crude Versions 

void R_AddDynamicLightsCrude (void)
{
	msurface_t *surf;
	int			lnum;
	int			sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local;
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;
	
	
	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		rad = cl_dlights[lnum].radius;
		if (cl_dlights[lnum].unmark)
			continue;		// don't add the one who doesn't want to be
	    
		VectorSubtract (cl_dlights[lnum].origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix
		 dist = DotProduct (origin_for_ent, surf->plane->normal) - // mankrip - dynamic lights on moving brush models fix - edited
				surf->plane->dist;
		rad -= fabs(dist);// + ((rad) * 0.2);

		minlight = cl_dlights[lnum].minlight;

		if (rad < minlight)
			continue;

		minlight = rad - minlight;

		impact[0] = origin_for_ent[0] - // mankrip - dynamic lights on moving brush models fix - edited
			surf->plane->normal[0]*dist;

			impact[1] = origin_for_ent[1] - // mankrip - dynamic lights on moving brush models fix - edited
					surf->plane->normal[1]*dist;
			impact[2] = origin_for_ent[2] - // mankrip - dynamic lights on moving brush models fix - edited
					surf->plane->normal[2]*dist;


		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];

		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)

				{
					unsigned temp;
					// leilei - approximate of the amiga port's "crude lighting"
					temp = rad * 66;

					i = t*smax + s;
					if (!cl_dlights[lnum].dark)
						blocklights[i] += temp;
					else
					{
						if (blocklights[i] > temp)
							blocklights[i] -= temp;
						else
							blocklights[i] = 0;
					}
				}

	


			}
		}
	}
}

void R_AddDynamicLightsRGBCrude ()
{
	msurface_t *surf;

	int		lnum;
	int		sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local;
	int		s, t;
	int		i;
	int		smax, tmax;
	mtexinfo_t	*tex;
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix
	
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
	float		cred, cgreen, cblue, brightness;
	unsigned	*bl;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		rad = cl_dlights[lnum].radius;
		if (cl_dlights[lnum].unmark)
			continue;		// don't add the one who doesn't want to be

		VectorSubtract (cl_dlights[lnum].origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix

		dist = DotProduct (origin_for_ent, surf->plane->normal) - // mankrip - dynamic lights on moving brush models fix - edited
				surf->plane->dist;
		
		rad -= fabs(dist);
		minlight = cl_dlights[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;

		// mankrip - dynamic lights on moving brush models fix - edited
		impact[0] = origin_for_ent[0] - surf->plane->normal[0]*dist;
		impact[1] = origin_for_ent[1] - surf->plane->normal[1]*dist;
		impact[2] = origin_for_ent[2] - surf->plane->normal[2]*dist;



		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];

// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
		cred = cl_dlights[lnum].color[0] * 256.0f;
		cgreen = cl_dlights[lnum].color[1] * 256.0f;
		cblue = cl_dlights[lnum].color[2] * 256.0f;

		bl = blocklights;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end



		
		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)
				{
					brightness = rad - dist;
					bl[0] += (int) (brightness * cred);
					bl[1] += (int) (brightness * cgreen);
					bl[2] += (int) (brightness * cblue);
				}
				bl += 3;
			}
		}
		
	}

}




/*
===============
R_AddShadows
===============
*/
void R_AddShadows (void)
{
	msurface_t *surf;
	int			lnum;
	int			sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local;
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix
	
	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<MAX_SHADOWS ; lnum++)
	{
		if ( !(surf->shadowbits & (1<<lnum) ) )
			continue;		// not lit by this light

		rad = cl_shadows[lnum].radius;
		 VectorSubtract (cl_shadows[lnum].origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix
		 	
		 dist = DotProduct (origin_for_ent, surf->plane->normal) - // mankrip - dynamic lights on moving brush models fix - edited
				surf->plane->dist;

		rad -= fabs(dist);
		minlight = cl_shadows[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;

		impact[0] = origin_for_ent[0] - 
			 			surf->plane->normal[0]*dist;
		impact[1] = origin_for_ent[1] - 
			 			surf->plane->normal[1]*dist;
		impact[2] = origin_for_ent[2] - 
			 			surf->plane->normal[2]*dist;


		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];

		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)

				{
					unsigned temp;
					temp = (rad - dist)*256;
					i = t*smax + s;
					//if (!cl_shadows[lnum].dark)
					//	blocklights[i] += temp;
		//			else
					{
						if (blocklights[i] > temp)
							blocklights[i] -= temp;
						else
							blocklights[i] = 0;
					}
				}




			}
		}
	}
}


void R_AddShadowsRGB ()
{
	msurface_t *surf;

	int		lnum;
	int		sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local;
	vec3_t      origin_for_ent; // mankrip - dynamic lights on moving brush models fix
	int		s, t;
	int		i;
	int		smax, tmax;
	mtexinfo_t	*tex;
	
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
	float		cred, cgreen, cblue, brightness;
	unsigned	*bl;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<MAX_SHADOWS ; lnum++)
	{
		if ( !(surf->shadowbits & (1<<lnum) ) )
			continue;		// not lit by this light

		rad = cl_shadows[lnum].radius;
		 VectorSubtract (cl_shadows[lnum].origin, currententity->origin, origin_for_ent); // mankrip - dynamic lights on moving brush models fix
		 	//	dist = DotProduct (cl_shadows[lnum].origin, surf->plane->normal) -
      dist = DotProduct (origin_for_ent, surf->plane->normal) - // mankrip - dynamic lights on moving brush models fix - edited
				surf->plane->dist;
		rad -= fabs(dist);
		minlight = cl_shadows[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;

		for (i=0 ; i<3 ; i++)
		{
		 impact[i] = origin_for_ent[i] - // mankrip - dynamic lights on moving brush models fix - edited
			 //impact[i] = cl_shadows[lnum].origin[i] -
					surf->plane->normal[i]*dist;
		}

		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];

// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
		cred = -256;
		cgreen = -256;
		cblue = -256;

		bl = blocklights;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end

		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  start
//					blocklights[t*smax + s] += (rad - dist)*256;
				{
					brightness = rad - dist;
					bl[0] += (int) (brightness * cred);
					bl[1] += (int) (brightness * cgreen);
					bl[2] += (int) (brightness * cblue);
				}
				bl += 3;
// 2001-09-11 Colored lightning by LordHavoc/Sarcazm/Maddes  end
			}
		}
	}
}




/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the 8.8 format in blocklights
===============
*/
extern cvar_t *r_lightquality;
void R_BuildLightMap (void)
{
	int			smax, tmax;
	int			t;
	int			i, size;
	byte		*lightmap;
	unsigned	scale;
	int			maps;
	int			crude;
	msurface_t	*surf;
	crude = r_lightquality->value;
	surf = r_drawsurf.surf;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;
	lightmap = surf->samples;

	if (r_fullbright->value || !cl.worldmodel->lightdata)
	{
		for (i=0 ; i<size ; i++)
			blocklights[i] = 0;
		return;
	}

// clear to ambient
	for (i=0 ; i<size ; i++)
		blocklights[i] = r_refdef.ambientlight<<8;


// add all the lightmaps
	if (lightmap)
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			scale = r_drawsurf.lightadj[maps];	// 8.8 fraction		
			for (i=0 ; i<size ; i++)
				blocklights[i] += lightmap[i] * scale;
			lightmap += size;	// skip to next lightmap
		}

// add all the dynamic lights
			 if (surf->dlightframe == r_framecount && dynlightenabled){
			 if (!crude)
			R_AddDynamicLightsCrude ();
				else
			R_AddDynamicLights ();
			 }

// and shadows
			 if (surf->shadowframe == r_framecount){
				R_AddShadows ();
			 }			 


// bound, invert, and shift
	for (i=0 ; i<size ; i++)
	{
		t = (255*256 - (int)blocklights[i]) >> (8 - VID_CBITS);

		if (t < (1 << 6))
			t = (1 << 6);

		blocklights[i] = t;
	}
}



// Integer only experiment......
void R_BuildLightMapReallyCrap (void)
{
	int			smax, tmax;
	int			t;
	int			i, size;
	byte		*lightmap;
	unsigned	scale;
	int			maps;
	int			crude;
	msurface_t	*surf;
	crude = r_lightquality->value;
	surf = r_drawsurf.surf;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;
	lightmap = surf->samples;

	if (r_fullbright->value || !cl.worldmodel->lightdata)
	{
		for (i=0 ; i<size ; i++)
			blocklights[i] = 0;
		return;
	}

// clear to ambient
	for (i=0 ; i<size ; i++)
		blocklights[i] = r_refdef.ambientlight<<8;


// add all the lightmaps
	if (lightmap)
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			scale = r_drawsurf.lightadj[maps];	// 8.8 fraction		
			for (i=0 ; i<size ; i++)
				blocklights[i] += lightmap[i] * scale;
			lightmap += size;	// skip to next lightmap
		}

// add all the dynamic lights
//			 if (surf->dlightframe == r_framecount && dynlightenabled){
//			 if (!crude)
//			R_AddDynamicLightsCrude ();
//				else
//			R_AddDynamicLights ();
//			 }

// and shadows
//			 if (surf->shadowframe == r_framecount){
//				R_AddShadows ();
//			 }			 


// bound, invert, and shift
	for (i=0 ; i<size ; i++)
	{
		t = (int)blocklights[i];

		if (t < (1 << 6))
			t = (1 << 6);

		blocklights[i] = t;
	}
}




// fteqw version
// optimized and fixed by leilei for less rendering bugs
extern cvar_t *temp2;
extern cvar_t *r_coloredlightmethod;
extern cvar_t *r_shiftlights;
void R_BuildLightMapRGB (void)
{
	int			smax, tmax;
	int			i, size;
	qbyte		*lightmap;
	unsigned	scale;
	int			maps;
	int			crude;
	msurface_t	*surf;
	int r;
	int sample;
	int overrights = overbrights; // localing
	int	shifted;
	crude = r_lightquality->value;
	if (r_shiftlights->value)
		shifted = 1;
	else 
		shifted = 0;
	if (overrights > 1)
		sample = 131032;
	if (overrights == 1)
		sample = 65536;
	else
		sample = 65536;

	if (coloredmethod)
		sample = 32768;	// overbright over this and our fast lightmaps complain.
	surf = r_drawsurf.surf;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax*3;
	lightmap = surf->samples;

	if (/* r_fullbright.value || */ !cl.worldmodel->lightdata)
	{
		for (i=0 ; i<size ; i++)
			blocklights[i] = 0;
		return;
	}

// clear to ambient
	for (i=0 ; i<size ; i++)
		blocklights[i] = r_refdef.ambientlight<<8;


// add all the lightmaps
	if (lightmap)
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			scale = r_drawsurf.lightadj[maps];	// 8.8 fraction		
			for (i=0 ; i<size ; i+=3)
			{
				blocklights[i]		+= lightmap[i] * scale;
				blocklights[i+1]	+= lightmap[i+1] * scale;
				blocklights[i+2]	+= lightmap[i+2] * scale;
			}
			lightmap += size;	// skip to next lightmap
		}

// add all the dynamic lights
			 if (surf->dlightframe == r_framecount && dynlightenabled)
				 if (!crude)
					R_AddDynamicLightsRGBCrude ();
				 else
					R_AddDynamicLightsRGB ();

// and shadows
			 if (surf->shadowframe == r_framecount)
				R_AddShadowsRGB ();
			 			 

	
// bound, invert, and shift
if (coloredmethod)
	{
	int t, re;
	for (i=0 ; i<size ; i++)
	{
		
		t = blocklights[i] / 2;
		if (t < 1024)
			t = 1024;
		if (t > sample)
			t = sample;
		t = t >> (8 - VID_CBITS);

		if (t < (1 << 6))
			t = (1 << 6);
		

		
		r = t;
	//	blocklights[i] = (r < 256) ? 256 : (r > sample) ? sample : r >> 1;
		blocklights[i] = r;
	
	}

	}
	else
	{
		for (i=0 ; i<size ; i++)
	{
		r = blocklights[i] >> shifted;
		blocklights[i] = (r < 256) ? 256 : (r > sample) ? sample : r;	// leilei - made min 256 to rid visual artifacts and gain speed
	}
	}
}

void R_BuildLightMapRGBHack (void)
{
	int			smax, tmax;
	int			t;
	int			i, size;
	qbyte		*lightmap;
	unsigned	scale;
	int			maps;
	msurface_t	*surf;
	int r;
	surf = r_drawsurf.surf;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax*3;
	lightmap = surf->samples;

	if (/* r_fullbright.value || */ !cl.worldmodel->lightdata)
	{
		for (i=0 ; i<size ; i++)
			blocklights[i] = 0;
		return;
	}

// clear to ambient
	for (i=0 ; i<size ; i++)
		blocklights[i] = r_refdef.ambientlight<<8;


// add all the lightmaps
	if (lightmap)
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			scale = r_drawsurf.lightadj[maps];	// 8.8 fraction		
			for (i=0 ; i<size ; i+=3)
			{
				blocklights[i]		+= lightmap[i] * scale;
				blocklights[i+1]	+= lightmap[i+1] * scale;
				blocklights[i+2]	+= lightmap[i+2] * scale;
			}
			lightmap += size;	// skip to next lightmap
		}

// add all the dynamic lights
			 if (surf->dlightframe == r_framecount)
		R_AddDynamicLightsRGB ();

// and shadows
			 if (surf->shadowframe == r_framecount)
				R_AddShadowsRGB ();
			 			 

	
// bound, invert, and shift
			 /*
for (i=0 ; i<size ; i++)

	{


	
		r = blocklights[i];
		    blocklights[i] = (r < 0) ? 0 : (r > 65535) ? 65535 : r;
			blocklights[i] = blocklights [i]  >> (8 - VID_CBITS);;
	}
	*/
	
 // bound, invert, and shift
	for (i=0 ; i<size ; i++)
	{
		t = (255*256 - (int)blocklights[i]) >> (8 - VID_CBITS);

		if (t < (1 << 6))
			t = (1 << 6);

		blocklights[i] = t;
	}
	
}



/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
texture_t *R_TextureAnimation (texture_t *base)
{
	int		reletive;
	int		count;

	if (currententity->frame)
	{
		if (base->alternate_anims)
			base = base->alternate_anims;
	}

	if (!base->anim_total)
		return base;

	reletive = (int)(cl.time*10) % base->anim_total;

	count = 0;
	while (base->anim_min > reletive || base->anim_max <= reletive)
	{
		base = base->anim_next;
		if (!base)
			base = base;					// leilei - TFN bb2 workaround
//			Sys_Error ("R_TextureAnimation: broken cycle");	
		if (++count > 100)
			Sys_Error ("R_TextureAnimation: infinite cycle");
	}

	return base;
}

int	hqlite;
int	ditheredrend;		// dithering


extern	cvar_t *r_lightquality;
extern	cvar_t *r_dither;

extern byte	palmap2[64][64][64];		// Higher quality for lighting
/*
===============
R_DrawSurface
===============
*/
#ifdef EXPREND
extern int shadowpass;
#endif
extern cvar_t *r_coloredlightmethod;
int	reallycrap = 0;
void R_DrawSurface (void)
{
	unsigned char	*basetptr;
	
	int				smax, tmax, twidth;
	int				u;

	int				soffset, basetoffset, texwidth;
	int				horzblockstep;
	unsigned char	*pcolumndest;
#ifdef EXPREND
	unsigned char	*pcolumnshadowdest;
#endif
	void			(*pblockdrawer)(void);
	texture_t		*mt;
	int				merp = 1;
#ifdef twentyfourbithack
	if (coloredlights == 2)
		merp = 1;
	
#endif
	if (!palmap2){
		hqlite = 0;	// don't do 18-bit lighting if we don't have this table
		coloredlights = 0; // disable colored lights anyway. :(
	}
	else
		hqlite = 1; // always use hqlite
	// calculate the lightings
	if (reallycrap)
	R_BuildLightMapReallyCrap ();
	else
	{
	if (coloredlights)
	R_BuildLightMapRGB ();
	else
	R_BuildLightMap ();
	}

	
// calculate the lightings
	
	surfrowbytes = r_drawsurf.rowbytes;

	mt = r_drawsurf.texture;


	r_source = (byte *)mt + mt->offsets[r_drawsurf.surfmip];
	


// the fractional light values should range from 0 to (VID_GRADES - 1) << 16
// from a source range of 0 - 255

//	texwidth = (mt->width) >> r_drawsurf.surfmip;
	texwidth = mt->width >> r_drawsurf.surfmip;

	blocksize = 16 >> r_drawsurf.surfmip;
	blockdivshift = 4 - r_drawsurf.surfmip;
	blockdivmask = (1 << blockdivshift) - 1;

	r_lightwidth = (r_drawsurf.surf->extents[0]>>4)+1;
//	r_lightwidth = ((r_drawsurf.surf->extents[0]>>4)+1);
	//r_lightwidther = (r_drawsurf.surf->extents[0]>>4)+1;
	

	r_numhblocks = r_drawsurf.surfwidth  >> blockdivshift;
	r_numvblocks = r_drawsurf.surfheight >> blockdivshift;
		
//==============================
	{
#ifndef EXPREND
		if (lowworld){
				if (coloredlights == 1)
					if (coloredmethod) pblockdrawer = surfmiptable888RGB[r_drawsurf.surfmip]; // 18-bit lookups
					else pblockdrawer = surfmiptable8RGBX[r_drawsurf.surfmip]; // 18-bit lookups
				else if (coloredlights == 2)	
					pblockdrawer = surfmiptable8RGBXD[r_drawsurf.surfmip]; // 18-bit lookups
				else
					pblockdrawer = surfmiptablefast[r_drawsurf.surfmip];	// use old fashioned grays
			
		
		}
		else
		{
			if (coloredlights == 1){
					if (coloredmethod) pblockdrawer = surfmiptable888RGB[r_drawsurf.surfmip]; // 18-bit lookups
					else pblockdrawer = surfmiptable8RGBX[r_drawsurf.surfmip]; // 18-bit lookups
					//	pblockdrawer = surfmiptable8RGBX[r_drawsurf.surfmip]; 
			}
			else if (coloredlights == 2){
				pblockdrawer = surfmiptable8RGBXD[r_drawsurf.surfmip];
			}
				else
			pblockdrawer = surfmiptable[r_drawsurf.surfmip];	// use old fashioned grays
		}
	// TODO: only needs to be set when there is a display settings change
		horzblockstep = blocksize;
	}
#else

	if (shadowpass)
		pblockdrawer = surfmiptableshadow[r_drawsurf.surfmip];	// use old fashioned grays
	else
		pblockdrawer = surfmiptablediffuse[r_drawsurf.surfmip];	// use old fashioned grays
			horzblockstep = blocksize;
}
		//	pblockdrawer = surfmiptable[r_drawsurf.surfmip];	// use old fashioned grays
#endif
	smax = mt->width >> r_drawsurf.surfmip;
	twidth = texwidth;
	tmax = mt->height >> r_drawsurf.surfmip;

	sourcetstep = texwidth;
	r_stepback = tmax * twidth;
	;
	r_sourcemax = r_source + (tmax * smax);

	soffset = r_drawsurf.surf->texturemins[0];
	basetoffset = r_drawsurf.surf->texturemins[1];

// << 16 components are to guarantee positive values for %
	soffset = ((soffset >> r_drawsurf.surfmip) + (smax << 16)) % smax;
	basetptr = &r_source[((((basetoffset >> r_drawsurf.surfmip)
		+ (tmax << 16)) % tmax) * twidth)];
#ifdef EXPREND
	if (shadowpass)
	pcolumndest = r_drawsurf.shadowdat;
	else
	pcolumndest = r_drawsurf.surfdat;
#else
	pcolumndest = r_drawsurf.surfdat;
#endif
	if (coloredlights == 2)
	horzblockstep *= 2;



	for (u=0 ; u<r_numhblocks; u++)
	{

		if (coloredlights)	{
		r_lightptr = blocklights + u * 3;

		}
	else
	{
		r_lightptr = blocklights + u;
	}
	{

#ifdef EXPREND
		if(shadowpass)
		prowdestbase = pcolumndest;
		else
		prowdestbase = pcolumndest;

		pbasesource = basetptr + soffset;
		
		(*pblockdrawer)();

		soffset = soffset + blocksize;
		if (soffset >= smax)
			soffset = 0;

		pcolumndest += horzblockstep;
#else
		prowdestbase = pcolumndest;

		pbasesource = basetptr + soffset;
		
		(*pblockdrawer)();

		soffset = soffset + blocksize;
		if (soffset >= smax)
			soffset = 0;

		pcolumndest += horzblockstep;

#endif
		

	}

	}
	
}





// scary 32-bit version

void R_DrawSurface32 (void)
{
#ifdef twentyfourbithack
	unsigned char	*basetptr;
	unsigned long	*basetptr24;
	int				smax, tmax, twidth;
	int				u;

	int				soffset24, basetoffset, texwidth;
	int				horzblockstep;
	unsigned char	*pcolumndest;
	void			(*pblockdrawer)(void);
	texture_t		*mt;
	int				merp = 4;
	int				surmip;

	
	R_BuildLightMapRGB ();


	
// calculate the lightings
	
	surfrowbytes = r_drawsurf.rowbytes;

	mt = r_drawsurf.texture;

	surmip = r_drawsurf.surfmip;

	r_source24 = (unsigned char *)mt + mt->offsets[surmip];

// the fractional light values should range from 0 to (VID_GRADES - 1) << 16
// from a source range of 0 - 255
	
	texwidth = (mt->width) >> r_drawsurf.surfmip;

	blocksize = 16 >> r_drawsurf.surfmip;
	blockdivshift = 4 - r_drawsurf.surfmip;
	blockdivmask = (1 << blockdivshift) - 1;

	r_lightwidth = ((r_drawsurf.surf->extents[0]>>4)+1);
	

	r_numhblocks = r_drawsurf.surfwidth  >> blockdivshift;
	r_numvblocks = r_drawsurf.surfheight >> blockdivshift;
	horzblockstep = blocksize;
//==============================
	{
		
		{
			if (coloredlights == 2){
				pblockdrawer = surfmiptable824RGBXD[r_drawsurf.surfmip];
			}
				else
			pblockdrawer = surfmiptable824RGBX[r_drawsurf.surfmip];	// we always use rgb lighting in 32-bit surfaces
		}
	// TODO: only needs to be set when there is a display settings change

	}
	smax = mt->width >> r_drawsurf.surfmip;
	twidth = texwidth;
	tmax = mt->height >> r_drawsurf.surfmip;

	sourcetstep = texwidth;
	r_stepback = tmax * twidth;
	
	r_sourcemax24 = r_source24 + (tmax * smax);

	soffset24 = r_drawsurf.surf->texturemins[0];
	basetoffset = r_drawsurf.surf->texturemins[1];



// << 16 components are to guarantee positive values for %
	basetptr24 = &r_source24[((((basetoffset >> r_drawsurf.surfmip)   + (tmax << 16)) % tmax) * twidth)];
	soffset24 = ((soffset24 >> r_drawsurf.surfmip) + (smax << 16)) % (smax);

	pcolumndest = r_drawsurf.surfdat;

	if (coloredlights == 2)
	horzblockstep *= 2;



	for (u=0 ; u<r_numhblocks; u++)
	{

		r_lightptr = blocklights + u * 3;


	

		prowdestbase = pcolumndest;

		pbasesource24 = basetptr24 + soffset24;
		(*pblockdrawer)();

		soffset24 = soffset24 + blocksize;
		if (soffset24 >= smax)
			soffset24 = 0;

		pcolumndest += horzblockstep;


	}



#endif	
}








//=============================================================================

#if	!id386



/*
================
R_DrawSurfaceBlock8_mip0
================
*/
void R_DrawSurfaceBlock8_mip0 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		
		lightleftstep = (r_lightptr[0] - lightleft) >> 4;
		
		lightrightstep = (r_lightptr[1] - lightright) >> 4;

		for (i=0 ; i<16 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 4;

			light = lightright;

			for (b=15; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
					[(light & 0xFF00) + pix];

			
				light += lightstep;
			}

			psource += sourcetstep;
				
			lightright += lightrightstep;
				
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

/*
================
R_DrawSurfaceBlock8_mip1
================
*/
void R_DrawSurfaceBlock8_mip1 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		
		lightleftstep = (r_lightptr[0] - lightleft) >> 3;

		lightrightstep = (r_lightptr[1] - lightright) >> 3;

		for (i=0 ; i<8 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 3;

			light = lightright;

			for (b=7; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				
				light += lightstep;
			}

			psource += sourcetstep;
			
			lightright += lightrightstep;
			
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip2
================
*/
void R_DrawSurfaceBlock8_mip2 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		
		lightleftstep = (r_lightptr[0] - lightleft) >> 2;
		
		lightrightstep = (r_lightptr[1] - lightright) >> 2;

		for (i=0 ; i<4 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 2;

			light = lightright;

			for (b=3; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				
				light += lightstep;
			}

			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip3
================
*/
void R_DrawSurfaceBlock8_mip3 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 1;
		lightrightstep = (r_lightptr[1] - lightright) >> 1;

		for (i=0 ; i<2 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 1;

			light = lightright;

			for (b=1; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
			}

			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock16

FIXME: make this work
================
*/
void R_DrawSurfaceBlock16 (void)
{
	int				k;
	unsigned char	*psource;
	int				lighttemp, lightstep, light;
	unsigned short	*prowdest;

	prowdest = (unsigned short *)prowdestbase;

	for (k=0 ; k<blocksize ; k++)
	{
		unsigned short	*pdest;
		unsigned char	pix;
		int				b;

		psource = pbasesource;
		lighttemp = lightright - lightleft;
		lightstep = lighttemp >> blockdivshift;

		light = lightleft;
		pdest = prowdest;

		for (b=0; b<blocksize; b++)
		{
			pix = *psource;
			*pdest = vid.colormap16[(light & 0xFF00) + pix];
			psource += sourcesstep;
			pdest++;
			light += lightstep;
		}

		pbasesource += sourcetstep;
		lightright += lightrightstep;
		lightleft += lightleftstep;
		prowdest = (unsigned short *)((long)prowdest + surfrowbytes);
	}

	prowdestbase = prowdest;
}








#endif
#ifdef EXPREND

/*
================
R_DrawSurfaceBlock8_mip0
================
*/
void R_DrawSurfaceBlock8D_mip0 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{

		for (i=0 ; i<16 ; i++)
		{

			for (b=15; b>=0; b--)
			{
				prowdest[b] = psource[b];

				
				

			}

			psource += sourcetstep;
				
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

/*
================
R_DrawSurfaceBlock8_mip1
================
*/
void R_DrawSurfaceBlock8D_mip1 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?

		for (i=0 ; i<8 ; i++)
		{

			for (b=7; b>=0; b--)
			{
				prowdest[b] = psource[b];
			}

			psource += sourcetstep;
			
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip2
================
*/
void R_DrawSurfaceBlock8D_mip2 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?

		for (i=0 ; i<4 ; i++)
		{

			for (b=3; b>=0; b--)
			{
				prowdest[b] = psource[b];
			}

			psource += sourcetstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip3
================
*/
void R_DrawSurfaceBlock8D_mip3 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		for (i=0 ; i<2 ; i++)
		{
			
			for (b=1; b>=0; b--)
			{
				prowdest[b] = psource[b];
				
			}

			psource += sourcetstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}




/*
================
R_DrawSurfaceBlock8_mip0
================
*/
void R_DrawSurfaceBlock8S_mip0 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		
		lightleftstep = (r_lightptr[0] - lightleft) >> 4;
		
		lightrightstep = (r_lightptr[1] - lightright) >> 4;

		for (i=0 ; i<16 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 4;

			light = lightright;

			for (b=15; b>=0; b--)
			{
				
				//prowdest[b] = light;
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) +15];
			
				light += lightstep;
			}

			psource += sourcetstep;
				
			lightright += lightrightstep;
				
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

/*
================
R_DrawSurfaceBlock8_mip1
================
*/
void R_DrawSurfaceBlock8S_mip1 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		
		lightleftstep = (r_lightptr[0] - lightleft) >> 3;

		lightrightstep = (r_lightptr[1] - lightright) >> 3;

		for (i=0 ; i<8 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 3;

			light = lightright;

			for (b=7; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00)];
				
				light += lightstep;
			}

			psource += sourcetstep;
			
			lightright += lightrightstep;
			
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip2
================
*/
void R_DrawSurfaceBlock8S_mip2 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		
		lightleftstep = (r_lightptr[0] - lightleft) >> 2;
		
		lightrightstep = (r_lightptr[1] - lightright) >> 2;

		for (i=0 ; i<4 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 2;

			light = lightright;

			for (b=3; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				
				light += lightstep;
			}

			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip3
================
*/
void R_DrawSurfaceBlock8S_mip3 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 1;
		lightrightstep = (r_lightptr[1] - lightright) >> 1;

		for (i=0 ; i<2 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 1;

			light = lightright;

			for (b=1; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
			}

			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


#endif
#if id386
// FAST VERSIONS BELOW LACK LIGHTSTEPPING


void R_DrawSurfaceBlock8_mip0fast (void)
{
	int				v, i, b, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		light = r_lightptr[0];
		for (i=0 ; i<16 ; i++)
		{
			for (b=15; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
					[(light & 0xFF00) + pix];
			}
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

void R_DrawSurfaceBlock8_mip1fast (void)
{
	int				v, i, b, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		light = r_lightptr[0];
		for (i=0 ; i<8 ; i++)
		{
			for (b=7; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
					[(light & 0xFF00) + pix];
			}
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

void R_DrawSurfaceBlock8_mip2fast (void)
{
	int				v, i, b, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		light = r_lightptr[0];
		for (i=0 ; i<4 ; i++)
		{
			for (b=3; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
					[(light & 0xFF00) + pix];
			}
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

void R_DrawSurfaceBlock8_mip3fast (void)
{
	int				v, i, b, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		light = r_lightptr[0];
		for (i=0 ; i<2 ; i++)
		{
			for (b=1; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
					[(light & 0xFF00) + pix];
			}
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}



#endif







extern unsigned	d_8to24table[256];
extern byte		*lmmap;

extern byte	palmap2[64][64][64];	





//	=!=!=!=!=!=!=!=
// ASMME: surf8rgb.s
//	=!=!=!=!=!=!=!=






// final!!! Crap cleaned out and slightly more optimized

extern byte transTable[256][256];



// leilei - i put them all here, cutting lots of bloat. I also deprecated hqlite. :(
// it's seemingly faster (!) doing it through one function instead of four











// 18-bit version
// Unrolled 

void R_DrawSurfaceBlock8RGBX_mip0()
{
	unsigned int				v, i; 
	unsigned int lightstep[3], light[3];
	unsigned int lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource, *prowdest;
	unsigned char *pix24;
	unsigned trans[3];
	psource = pbasesource;
	prowdest = prowdestbase;


	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 4; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 4;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 4; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 4;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 4;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 4;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 4;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 4;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 4;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 4;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 4;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 4;


		for (i=0 ; i<16 ; i++)
		{
			Mip0Stuff(MIP8RGBX);
		
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}




void R_DrawSurfaceBlock8RGBX_mip1()
{
	unsigned int				v, i; 
	unsigned int lightstep[3], light[3];
	unsigned int lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource, *prowdest;
	unsigned char *pix24;
	unsigned trans[3];
	psource = pbasesource;
	
	prowdest = prowdestbase;

	
	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 3; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 3;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 3; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 3;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 3;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 3;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 3;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 3;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 3;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 3;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 3;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 3;

		for (i=0 ; i<8 ; i++)
		{
			Mip1Stuff(MIP8RGBX);


		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}



void R_DrawSurfaceBlock8RGBX_mip2()
{
	unsigned int				v, i; 
	unsigned int lightstep[3], light[3];
	unsigned int lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource, *prowdest;
	unsigned char *pix24;
	unsigned trans[3];
	psource = pbasesource;
	
	prowdest = prowdestbase;

	
	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 2; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 2;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 2; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 2;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 2;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 2;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 2;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 2;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 2;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 2;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 2;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 2;

		for (i=0 ; i<4 ; i++)
		{
			Mip2Stuff(MIP8RGBX);


		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}


void R_DrawSurfaceBlock8RGBX_mip3()
{
	unsigned int				v, i; 
	unsigned int lightstep[3], light[3];
	unsigned int lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource, *prowdest;
	unsigned char *pix24;
	unsigned trans[3];
	psource = pbasesource;
	
	prowdest = prowdestbase;

	
	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 1; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 1;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 1; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 1;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 1;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 1;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 1;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 1;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 1;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 1;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 1;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 1;

		for (i=0 ; i<2 ; i++)
		{
			Mip3Stuff(MIP8RGBX);

	
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}



// 8RGBXD - dithered, 8-bit texture mip block 

void R_DrawSurfaceBlock8RGBXD_mip0 ()
{
	unsigned 				v, i; 
	unsigned  lightstep[3],light[3];
	unsigned  lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource;
	unsigned short *prowdest;
  	unsigned char *pix24;
	unsigned trans[3];
	psource = pbasesource;
	
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 4; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 4;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 4; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 4;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 4;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 4;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 4;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 4;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 4;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 4;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 4;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 4;
	
		for (i=0 ; i<16 ; i++)
		{

			Mip0Stuff(MIP8RGBXD);


		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}



void R_DrawSurfaceBlock8RGBXD_mip1 ()
{
	unsigned 				v, i; 
	unsigned  lightstep[3],light[3];
	unsigned  lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource;
	unsigned short *prowdest;
  	unsigned char *pix24;
	unsigned trans[3];
	psource = pbasesource;
	
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 3; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 3;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 3; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 3;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 3;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 3;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 3;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 3;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 3;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 3;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 3;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 3;

		for (i=0 ; i<8 ; i++)
		{
			Mip1Stuff(MIP8RGBXD);
			


		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}



void R_DrawSurfaceBlock8RGBXD_mip2 ()
{
	unsigned 				v, i; 
	unsigned  lightstep[3],light[3];
	unsigned  lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource;
	unsigned short *prowdest;
  	unsigned char *pix24;
	unsigned trans[3];
	psource = pbasesource;
	
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 2; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 2;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 2; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 2;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 2;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 2;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 2;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 2;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 2;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 2;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 2;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 2;

		for (i=0 ; i<4 ; i++)
		{
			Mip2Stuff(MIP8RGBXD);
			

	
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}




void R_DrawSurfaceBlock8RGBXD_mip3 ()
{
	unsigned 				v, i; 
	unsigned  lightstep[3],light[3];
	unsigned  lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource;
	unsigned short *prowdest;
  	unsigned char *pix24;
	unsigned trans[3];
	psource = pbasesource;
	
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 1; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 1;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 1; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 1;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 1;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 1;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 1;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 1;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 1;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 1;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 1;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 1;

		for (i=0 ; i<2 ; i++)
		{
			Mip3Stuff(MIP8RGBXD);

	
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}





// New (and highly experimental) triple lookup method


void R_DrawSurfaceBlock888RGBLoops (int mip, int mif)
{
	int				v, i, b; 
	int		mil;
	int lightstep[3], lighttemp[3], light[3];
	int lightdelta[3], lightdeltastep[3];
	int		r;
	unsigned char	pix, *psource, *prowdest;
 
	unsigned char *pix24;
	int trans[3];
		int ar, g, be;
	psource = pbasesource;
	
	prowdest = prowdestbase;

	
	mil = mip - 1;	


	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?

		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> mif; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> mif;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> mif; 


		r_lightptr += r_lightwidth * 3;



		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> mif;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> mif;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> mif;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> mif;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> mif;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> mif;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> mif;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> mif;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> mif;

		for (i=0 ; i<mip ; i++)
		{
			light[0] = lightrighta[0];
			light[1] = lightrighta[1];
			light[2] = lightrighta[2];
									
		for (b=mil; b>=0; b--)
			{
				ar = rgb_colormap_red		[(light[0] & 0xFF00) + psource[b]];
				g  = rgb_colormap_green		[(light[1] & 0xFF00) + psource[b]];
				be = rgb_colormap_blue		[(light[2] & 0xFF00) + psource[b]];
	 			ar>>=2;	g>>=2; be>>=2;	// leilei - shift to 15bpp
				prowdest[b] = d_15to8table[(ar<<0) + (g<<5) + (be<<10)];

				light[0] += lightdelta[0];
				light[1] += lightdelta[1];
				light[2] += lightdelta[2];

			}
		
			psource += sourcetstep;
				

					lightrighta[0] += lightrightstepa[0];
					lightlefta[0] += lightleftstepa[0];
					lightdelta[0] += lightdeltastep[0];
					lightrighta[1] += lightrightstepa[1];
					lightlefta[1] += lightleftstepa[1];
					lightdelta[1] += lightdeltastep[1];
					lightrighta[2] += lightrightstepa[2];
					lightlefta[2] += lightleftstepa[2];
					lightdelta[2] += lightdeltastep[2];
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}

#ifdef COMBINED
void R_DrawSurfaceBlock888RGB_mip0 (void)
{
	R_DrawSurfaceBlock888RGBLoops (16, 4);
}
void R_DrawSurfaceBlock888RGB_mip1 (void)
{
	R_DrawSurfaceBlock888RGBLoops (8, 3);
}
void R_DrawSurfaceBlock888RGB_mip2 (void)
{
	R_DrawSurfaceBlock888RGBLoops (4, 2);
}
void R_DrawSurfaceBlock888RGB_mip3 (void)
{
	R_DrawSurfaceBlock888RGBLoops (2, 1);
}
#else

void R_DrawSurfaceBlock888RGB_mip0()
{
	unsigned int				v, i; 
	unsigned int lightstep[3], light[3];
	unsigned int lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource, *prowdest;


	psource = pbasesource;
	prowdest = prowdestbase;


	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 4; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 4;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 4; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 4;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 4;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 4;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 4;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 4;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 4;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 4;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 4;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 4;


		for (i=0 ; i<16 ; i++)
		{
			Mip0Stuff(MIP888RGB);	// leilei - macrofied


		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}




void R_DrawSurfaceBlock888RGB_mip1()
{
	unsigned int				v, i; 
	unsigned int lightstep[3], light[3];
	unsigned int lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource, *prowdest;
	psource = pbasesource;
	
	prowdest = prowdestbase;

	
	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 3; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 3;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 3; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 3;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 3;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 3;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 3;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 3;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 3;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 3;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 3;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 3;

		for (i=0 ; i<8 ; i++)
		{
			Mip1Stuff(MIP888RGB);	// leilei - macrofied


		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}



void R_DrawSurfaceBlock888RGB_mip2()
{
	unsigned int				v, i; 
	unsigned int lightstep[3], light[3];
	unsigned int lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource, *prowdest;
	psource = pbasesource;
	
	prowdest = prowdestbase;

	
	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 2; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 2;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 2; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 2;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 2;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 2;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 2;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 2;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 2;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 2;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 2;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 2;

		for (i=0 ; i<4 ; i++)
		{
			Mip2Stuff(MIP888RGB);	// leilei - macrofied


		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}


void R_DrawSurfaceBlock888RGB_mip3()
{
	unsigned int				v, i; 
	unsigned int lightstep[3], light[3];
	unsigned int lightdelta[3], lightdeltastep[3];
	unsigned char	pix, *psource, *prowdest;
	psource = pbasesource;
	
	prowdest = prowdestbase;

	
	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 1; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 1;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 1; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 1;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 1;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 1;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 1;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 1;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 1;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 1;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 1;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 1;

		for (i=0 ; i<2 ; i++)
		{
			Mip3Stuff(MIP888RGB);	// leilei - macrofied

	
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
		
	}
}


#endif

//extern cvar_t temp3;

// 824RGBXD - dithered, 24-bit texture mip block 

// Looped versions for development and trial and erroring
// I will unroll the hell out of this when stuff's confirmed to function as designed.


void R_DrawSurfaceBlock824RGBXDLoops (int mip, int mif)
{
	int				v, i, b, r; 
	int lightstep[3], light[3];
	unsigned long *psource;
	unsigned short *prowdest;
	int mil;
	
	unsigned char *pix24;
	int trans[3];
	mil = mip - 1;
	psource = pbasesource24;
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
	
		lightlefta[0]  = r_lightptr[0];
		lightlefta[1]  = r_lightptr[0+1];
		lightlefta[2]  = r_lightptr[0+2];

		lightrighta[0] = r_lightptr[3];
		lightrighta[1] = r_lightptr[3+1];
		lightrighta[2] = r_lightptr[3+2];
		
		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0]  = ((int)r_lightptr[0]   - lightlefta[0]) >> mif;
		lightleftstepa[1]  = ((int)r_lightptr[0+1] - lightlefta[1]) >> mif;
		lightleftstepa[2]  = ((int)r_lightptr[0+2] - lightlefta[2]) >> mif;

		lightrightstepa[0] = ((int)r_lightptr[3]   - lightrighta[0]) >> mif;
		lightrightstepa[1] = ((int)r_lightptr[3+1] - lightrighta[1]) >> mif;
		lightrightstepa[2] = ((int)r_lightptr[3+2] - lightrighta[2]) >> mif;


		for (i=0 ; i<mip ; i++)
		{
			lightstep[0] = (lightlefta[0] - lightrighta[0]) >> mif;
			lightstep[1] = (lightlefta[1] - lightrighta[1]) >> mif;
			lightstep[2] = (lightlefta[2] - lightrighta[2]) >> mif;

			light[0] = lightrighta[0];
			light[1] = lightrighta[1];
			light[2] = lightrighta[2];

		for (b=mil; b>=0; b--)
		{
	
		  {

			
			pix24 = (unsigned char *)&psource[b];
			for (r=0; r<4; r++){
			trans[r] = ((int)pix24[r] * light[r]) >> 18;
			if (trans[r] & ~31) trans[r] = 31;
			}
			prowdest[b] = (trans[0] << 10) | (trans[1] << 5) | trans[2];

			}


			light[0] += lightstep[0];
			light[1] += lightstep[1];
			light[2] += lightstep[2];
			}
		
			psource += sourcetstep;
			


			lightrighta[0] += lightrightstepa[0];
			lightrighta[1] += lightrightstepa[1];
			lightrighta[2] += lightrightstepa[2];

			lightlefta[0] += lightleftstepa[0];
			lightlefta[1] += lightleftstepa[1];
			lightlefta[2] += lightleftstepa[2];
			
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax24)
			psource -= r_stepback;

	}
}


void R_DrawSurfaceBlock824RGBXD_mip0 ()
{
	int				v, i, b, r; 
	int lightstep[3], light[3], lightdelta[3], lightdeltastep[3];
	unsigned long *psource;
	unsigned short *prowdest;
	unsigned char *pix24;
	int trans[3];
	psource = pbasesource24;
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 4; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 4;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 4; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 4;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 4;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 4;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 4;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 4;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 4;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 4;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 4;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 4;
	
		for (i=0 ; i<16 ; i++)
		{
			
			Mip0Stuff(MIP824RGBXD);

	
		}

		if (psource >= r_sourcemax24)
			psource -= r_stepback;
		
	}
}



void R_DrawSurfaceBlock824RGBXD_mip1 ()
{
	int				v, i, b, r; 
	int lightstep[3], light[3], lightdelta[3], lightdeltastep[3];
	unsigned long *psource;
	unsigned short *prowdest;
	unsigned char *pix24;
	int trans[3];
	psource = pbasesource24;
	prowdest = (unsigned short *)prowdestbase;
	
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 3; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 3;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 3; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 3;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 3;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 3;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 3;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 3;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 3;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 3;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 3;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 3;

		for (i=0 ; i<8 ; i++)
		{
			Mip1Stuff(MIP824RGBXD);
			


		}

		if (psource >= r_sourcemax24)
			psource -= r_stepback;
		
	}
}



void R_DrawSurfaceBlock824RGBXD_mip2 ()
{
	int				v, i, b, r; 
	int lightstep[3], light[3], lightdelta[3], lightdeltastep[3];
	unsigned long *psource;
	unsigned short *prowdest;
	unsigned char *pix24;
	int trans[3];
	psource = pbasesource24;
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 2; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 2;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 2; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 2;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 2;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 2;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 2;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 2;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 2;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 2;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 2;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 2;

		for (i=0 ; i<4 ; i++)
		{
			Mip2Stuff(MIP824RGBXD);
						
			

		
		}

		if (psource >= r_sourcemax24)
			psource -= r_stepback;
		
	}
}




void R_DrawSurfaceBlock824RGBXD_mip3 ()
{
	int				v, i, b, r; 
	int lightstep[3], light[3], lightdelta[3], lightdeltastep[3];
	unsigned long *psource;
	unsigned short *prowdest;
	unsigned char *pix24;
	int trans[3];
	psource = pbasesource24;
	prowdest = (unsigned short *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
		lightlefta[0] = r_lightptr[0];
		lightrighta[0] = r_lightptr[3];
		lightlefta[1] = r_lightptr[0+1];
		lightrighta[1] = r_lightptr[3+1];
		lightlefta[2] = r_lightptr[0+2];
		lightrighta[2] = r_lightptr[3+2];

		lightdelta[0] = (lightlefta[0] - lightrighta[0])  >> 1; 
		lightdelta[1] = (lightlefta[1] - lightrighta[1])  >> 1;  
		lightdelta[2] = (lightlefta[2] - lightrighta[2])  >> 1; 


		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0] = (r_lightptr[0] - lightlefta[0]) >> 1;
		lightrightstepa[0] = (r_lightptr[3] - lightrighta[0]) >> 1;

		lightleftstepa[1] = (r_lightptr[0+1] - lightlefta[1]) >> 1;
		lightrightstepa[1] = (r_lightptr[3+1] - lightrighta[1]) >> 1;

		lightleftstepa[2] = (r_lightptr[0+2] - lightlefta[2]) >> 1;
		lightrightstepa[2] = (r_lightptr[3+2] - lightrighta[2]) >> 1;

		lightdeltastep[0] = (lightleftstepa[0] - lightrightstepa[0]) >> 1;
		lightdeltastep[1] = (lightleftstepa[1] - lightrightstepa[1]) >> 1;
		lightdeltastep[2] = (lightleftstepa[2] - lightrightstepa[2]) >> 1;

		for (i=0 ; i<2 ; i++)
		{
			Mip3Stuff(MIP824RGBXD);
			


		}

		if (psource >= r_sourcemax24)
			psource -= r_stepback;
		
	}
}






#ifdef twentyfourbithack


void R_DrawSurfaceBlock824RGBXDf_mip0 (void)
{
	R_DrawSurfaceBlock824RGBXDLoops (16, 4);
}
void R_DrawSurfaceBlock824RGBXDf_mip1 (void)
{
	R_DrawSurfaceBlock824RGBXDLoops (8, 3);
}
void R_DrawSurfaceBlock824RGBXDf_mip2 (void)
{
	R_DrawSurfaceBlock824RGBXDLoops (4, 2);
}
void R_DrawSurfaceBlock824RGBXDf_mip3 (void)
{
	R_DrawSurfaceBlock824RGBXDLoops (2, 1);
}











// No dithering in this one, goes straight to 8-bit spans (losing the point aren't we? Well colored lighting
// on 24-bit textures would probably choose better colors than colored lighting on translated textures
// so there's still *some* point to this.

void R_DrawSurfaceBlock824RGBXLoops (int mip, int mif)
{
	int				v, i, b, r; 
	int lightstep[3], light[3];
	unsigned long *psource;
	unsigned char *prowdest;
	int mil;
	
	unsigned char *pix24;
	int trans[3];
	mil = mip - 1;
	psource = (unsigned long *) pbasesource24;
	prowdest = (unsigned char *)prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
	
		lightlefta[0]  = r_lightptr[0];
		lightlefta[1]  = r_lightptr[0+1];
		lightlefta[2]  = r_lightptr[0+2];

		lightrighta[0] = r_lightptr[3];
		lightrighta[1] = r_lightptr[3+1];
		lightrighta[2] = r_lightptr[3+2];
		
		r_lightptr += r_lightwidth * 3;

		lightleftstepa[0]  = ((int)r_lightptr[0]   - lightlefta[0]) >> mif;
		lightleftstepa[1]  = ((int)r_lightptr[0+1] - lightlefta[1]) >> mif;
		lightleftstepa[2]  = ((int)r_lightptr[0+2] - lightlefta[2]) >> mif;

		lightrightstepa[0] = ((int)r_lightptr[3]   - lightrighta[0]) >> mif;
		lightrightstepa[1] = ((int)r_lightptr[3+1] - lightrighta[1]) >> mif;
		lightrightstepa[2] = ((int)r_lightptr[3+2] - lightrighta[2]) >> mif;


		for (i=0 ; i<mip ; i++)
		{
			lightstep[0] = (lightlefta[0] - lightrighta[0]) >> mif;
			lightstep[1] = (lightlefta[1] - lightrighta[1]) >> mif;
			lightstep[2] = (lightlefta[2] - lightrighta[2]) >> mif;

			light[0] = lightrighta[0];
			light[1] = lightrighta[1];
			light[2] = lightrighta[2];

		for (b=mil; b>=0; b--)
		{
	
		  {

			
			pix24 = (unsigned char *)&psource[b];
			for (r=0; r<4; r++){
			trans[r] = ((int)pix24[r] * light[r]) >> 17;
			if (trans[r] & ~63) trans[r] = 63;
			
			}

			prowdest[b] = palmap2[trans[0]][trans[1]][trans[2]]; 

			}


			light[0] += lightstep[0];
			light[1] += lightstep[1];
			light[2] += lightstep[2];
			}
		
			psource += sourcetstep;
			


			lightrighta[0] += lightrightstepa[0];
			lightrighta[1] += lightrightstepa[1];
			lightrighta[2] += lightrightstepa[2];

			lightlefta[0] += lightleftstepa[0];
			lightlefta[1] += lightleftstepa[1];
			lightlefta[2] += lightleftstepa[2];
			
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax24)
			psource -= r_stepback;

	}
}


void R_DrawSurfaceBlock824RGBX_mip0 (void)
{
	R_DrawSurfaceBlock824RGBXLoops (16, 4);
}
void R_DrawSurfaceBlock824RGBX_mip1 (void)
{
	R_DrawSurfaceBlock824RGBXLoops (8, 3);
}
void R_DrawSurfaceBlock824RGBX_mip2 (void)
{
	R_DrawSurfaceBlock824RGBXLoops (4, 2);
}
void R_DrawSurfaceBlock824RGBX_mip3 (void)
{
	R_DrawSurfaceBlock824RGBXLoops (2, 1);
}

#endif

//============================================================================

/*
================
R_GenTurbTile
================

 



*/	  
		
		  		  
						
						  
					
void R_GenTurbTile (pixel_t *pbasetex, void *pdest)
{
	int		*turb;
	int		i, j, s, t;
	byte	*pd;

	turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));
	pd = (byte *)pdest;

	for (i=0 ; i<TILE_SIZE ; i++)
	{
		for (j=0 ; j<TILE_SIZE ; j++)
		{
			s = (((j << 16) + turb[i & (CYCLE-1)]) >> 16) & 63;
			t = (((i << 16) + turb[j & (CYCLE-1)]) >> 16) & 63;
			*pd++ = *(pbasetex + (t<<6) + s);
		}
	}
}


/*
================
R_GenTurbTile16
================
*/
void R_GenTurbTile16 (pixel_t *pbasetex, void *pdest)
{
	int				*turb;
	int				i, j, s, t;
	unsigned short	*pd;

	turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));
	pd = (unsigned short *)pdest;

	for (i=0 ; i<TILE_SIZE ; i++)
	{
		for (j=0 ; j<TILE_SIZE ; j++)
		{
			s = (((j << 16) + turb[i & (CYCLE-1)]) >> 16) & 63;
			t = (((i << 16) + turb[j & (CYCLE-1)]) >> 16) & 63;
			*pd++ = d_8to16table[*(pbasetex + (t<<6) + s)];
		}
	}
}


/*
================
R_GenTile
================
*/
void R_GenTile (msurface_t *psurf, void *pdest)
{
	if (psurf->flags & SURF_DRAWTURB)
	{

		if (r_pixbytes == 1)
		{
			R_GenTurbTile ((pixel_t *)
				((byte *)psurf->texinfo->texture + psurf->texinfo->texture->offsets[0]), pdest);
		}
		else
		{
			R_GenTurbTile16 ((pixel_t *)
				((byte *)psurf->texinfo->texture + psurf->texinfo->texture->offsets[0]), pdest);
		}
	}
	else if (psurf->flags & SURF_DRAWSKY)
	{
		if (r_pixbytes == 1)
		{
			R_GenSkyTile (pdest);
		}
		else
		{
			R_GenSkyTile16 (pdest);
		}
	}
	else
	{
		Sys_Error ("Unknown tile type");
	}
}




















/*
================
R_BuildSurfaceDisplayList

	Gather up polygons. Because...... we can.
================
*/


mvertex_t	*r_pcurrentvertbase;
model_t		*currentmodel;

int	nColinElim;


void R_BuildSurfaceDisplayList (msurface_t *fa)
{
	int			i, lindex, lnumverts, s_axis, t_axis;
	float		dist, lastdist, lzi, scale, u, v, frac;
	unsigned	mask;
	vec3_t		local, transformed;
	medge_t		*pedges, *r_pedge;
	mplane_t	*pplane;
	int			newverts, newpage, lastvert;	//vertpage,  // 2001-12-10 Reduced compiler warnings by Jeff Ford
	qboolean	visible;
	float		*vec;
	float		s, t;
	glpoly_t	*poly;

// reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = fa->numedges;
//	vertpage = 0;	// 2001-12-10 Reduced compiler warnings by Jeff Ford

	//
	// draw texture
	//
	poly = Hunk_Alloc (sizeof(glpoly_t) + (lnumverts-4) * VERTEXSIZE*sizeof(float));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = currentmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = r_pcurrentvertbase[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = r_pcurrentvertbase[r_pedge->v[1]].position;
		}
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->texture->height;

		VectorCopy (vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		//
		// lightmap texture coordinates
		//

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;
	}

	//
	// remove co-linear points - Ed
	//

	poly->numverts = lnumverts;

}